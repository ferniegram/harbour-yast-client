#include "chatmanager.h"

#define DEBUG_MODULE ChatManagerAndModel
#include "debuglog.h"

#include "chatdata.h"

namespace {
    const QString _TYPE("@type");
    const QString ID("id");
    const QString SMALL("small");
    const QString USER_ID("user_id");
    const QString CHAT_ID("chat_id");
    const QString PHOTO("photo");
    const QString LAST_READ_INBOX_MESSAGE_ID("last_read_inbox_message_id");
    const QString LAST_READ_OUTBOX_MESSAGE_ID("last_read_outbox_message_id");
    const QString LAST_MESSAGE("last_message");
    const QString TYPE("type");
    const QString IS_CHANNEL("is_channel");
    const QString BASIC_GROUP_ID("basic_group_id");
    const QString SUPERGROUP_ID("supergroup_id");
    const char* PROPERTY_CHAT_INFORMATION = "chatInformation";

    const QString CONTENT_MESSAGE_PHOTO("messagePhoto");
    const QString CONTENT_MESSAGE_VIDEO("messageVideo");
    const QString CONTENT_MESSAGE_ANIMATION("messageAnimation");
    const QString CONTENT_MESSAGE_VIDEO_NOTE("messageVideoNote");
}

ChatMessagesModel::ChatMessagesModel(TDLibWrapper *tdLibWrapper, qlonglong chatId, QObject *parent) : ReadableMessagesModel(tdLibWrapper, parent), searchQuery() {
    this->chatId = chatId;
}

bool ChatMessagesModel::clear() {
    LOG("Clearing chat model");
    this->searchQuery.clear();
    return ReadableMessagesModel::clear();
}

void ChatMessagesModel::loadMessages(qlonglong fromMessageId, int offset) {
    if (searchQuery.isEmpty())
        this->tdLibWrapper->getChatHistory(chatId, fromMessageId, offset);
    else
        // ignore offset for now
        this->tdLibWrapper->searchChatMessages(chatId, searchQuery, fromMessageId);
}

void ChatMessagesModel::setSearchQuery(const QString newSearchQuery) {
    if (this->searchQuery != newSearchQuery) {
        this->clear();
        this->searchQuery = newSearchQuery;
        this->loadMessages(searchQuery.isEmpty() ? this->parent()->property(PROPERTY_CHAT_INFORMATION).toMap().value(LAST_READ_INBOX_MESSAGE_ID).toLongLong() : 0); // fixme
    }
}

qlonglong ChatMessagesModel::lastReadInboxMessageId() const {
    return this->parent()->property(PROPERTY_CHAT_INFORMATION).toMap().value(LAST_READ_INBOX_MESSAGE_ID).toLongLong();
}
qlonglong ChatMessagesModel::lastReadOutboxMessageId() const {
    return this->parent()->property(PROPERTY_CHAT_INFORMATION).toMap().value(LAST_READ_OUTBOX_MESSAGE_ID).toLongLong();
}
qlonglong ChatMessagesModel::lastMessageId() const { // FIXME: this is wrong and shouldn't be used ideally
    return this->parent()->property(PROPERTY_CHAT_INFORMATION).toMap().value(LAST_MESSAGE).toMap().value(ID).toLongLong();
}



ChatManager::ChatManager(QObject *parent)
    : QObject(parent),
      tdLibWrapper(nullptr),
      chatId(0),
      pinnedMessageId(0),
      mainModelsInitializationScheduled(false),
      mainModelsInitializationScheduledFromMessageId(0),

      chatMessagesModel(nullptr),
      photoAndVideoMessagesModel(nullptr),
      animationMessagesModel(nullptr),
      videoNoteMessagesModel(nullptr),
      topicsModel(nullptr)
{
    connect(this, &ChatManager::chatIdChanged, this, &ChatManager::smallPhotoChanged);
    connect(this, &ChatManager::chatIdChanged, this, &ChatManager::chatInformationChanged);
    connect(this, &ChatManager::chatIdChanged, this, &ChatManager::viewAsTopicsChanged);
    connect(this, &ChatManager::chatIdChanged, this, &ChatManager::userInfoChanged);
    connect(this, &ChatManager::chatIdChanged, this, &ChatManager::groupInfoChanged);
}

ChatManager::~ChatManager() {
    LOG("Destroying myself...");
}

void ChatManager::setTDLibWrapper(QObject *obj) {
    TDLibWrapper *wrapper = qobject_cast<TDLibWrapper*>(obj);
    if (tdLibWrapper != wrapper) {
        tdLibWrapper = wrapper;
        LOG("TDLibWrapper set" << wrapper);
        emit tdlibChanged();

        if (tdLibWrapper) {
            connect(this->tdLibWrapper, &TDLibWrapper::chatReadInboxUpdated, this, &ChatManager::handleChatReadInboxUpdated);
            connect(this->tdLibWrapper, &TDLibWrapper::chatReadOutboxUpdated, this, &ChatManager::handleChatReadOutboxUpdated);
            connect(this->tdLibWrapper, &TDLibWrapper::chatRolesUpdated, this, &ChatManager::handleChatRolesUpdated);
            connect(this->tdLibWrapper, &TDLibWrapper::chatPinnedMessageUpdated, this, &ChatManager::handleChatPinnedMessageUpdated);
            connect(this->tdLibWrapper, &TDLibWrapper::chatActionUpdated, this, &ChatManager::handleChatActionUpdated);
            connect(this->tdLibWrapper, &TDLibWrapper::userUpdated, this, &ChatManager::handleUserUpdated);
            connect(this->tdLibWrapper, &TDLibWrapper::basicGroupUpdated, this, &ChatManager::handleBasicGroupUpdated);
            connect(this->tdLibWrapper, &TDLibWrapper::superGroupUpdated, this, &ChatManager::handleSupergroupUpdated);

            if (chatId) {
                LOG("tdLibWrapper set when chatId already is set, finishing initialization");
                emit this->chatIdChanged(); // emit signals for chat, user, group info and so on
                finishInitialization();
            }

            if (mainModelsInitializationScheduled) {
                LOG("tdLibWrapper set, running scheduled main models initialization");
                this->initializeMainModels(mainModelsInitializationScheduledFromMessageId);
                mainModelsInitializationScheduled = false;
                mainModelsInitializationScheduledFromMessageId = 0;
            }
        }
    }
}

void ChatManager::handleChatReadInboxUpdated(const QString &id) {
    if (this->chatId == id.toLongLong() && this->chatMessagesModel)
        emit this->chatMessagesModel->lastReadMessageIndexChanged();
}

void ChatManager::handleChatReadOutboxUpdated(const QString &id) {
    if (this->chatId == id.toLongLong() && this->chatMessagesModel)
        emit this->chatMessagesModel->lastReadSentMessageUpdated();
}

QVariantMap ChatManager::smallPhoto() const {
    return chatInformation().value(PHOTO).toMap().value(SMALL).toMap();
}

QVariantMap ChatManager::pendingJoinRequests() const {
    return chatInformation().value("pending_join_requests").toMap();
}

void ChatManager::handleChatPendingJoinRequestsUpdated(qlonglong chatId) {
    if (this->chatId == chatId)
        emit pendingJoinRequestsChanged();
}

TDLibWrapper::ChatType ChatManager::chatType() const {
    if (tdLibWrapper) {
        ChatData* chatData = tdLibWrapper->getChatData(chatId);
        if (chatData)
            return chatData->chatType;
    }
    return TDLibWrapper::ChatTypeUnknown;
}

bool ChatManager::isChannel() const {
    return chatType() == TDLibWrapper::ChatTypeSupergroup && chatInformation().value(TYPE).toMap().value(IS_CHANNEL).toBool();
}

qlonglong ChatManager::userId() const {
    return chatInformation().value(TYPE).toMap().value(USER_ID).toLongLong();
}

qlonglong ChatManager::groupId() const {
    return chatInformation().value(TYPE).toMap().value(chatType() == TDLibWrapper::ChatTypeSupergroup ? SUPERGROUP_ID : BASIC_GROUP_ID).toLongLong();
}

QVariant ChatManager::userInfo() const {
    const TDLibWrapper::ChatType type = chatType();
    if (type == TDLibWrapper::ChatTypePrivate || type == TDLibWrapper::ChatTypeSecret)
        return tdLibWrapper->getUserInformation(QString::number(this->userId()));
    return QVariant();
}

QVariant ChatManager::groupInfo() const {
    const TDLibWrapper::ChatType type = chatType();
    if (type == TDLibWrapper::ChatTypeBasicGroup)
        return tdLibWrapper->getBasicGroup(groupId());
    if (type == TDLibWrapper::ChatTypeSupergroup)
        return tdLibWrapper->getSuperGroup(groupId());
    return QVariant();
}

void ChatManager::handleUserUpdated(const QString &userId) {
    if (this->userId() == userId.toLongLong())
        emit userInfoChanged();
}

void ChatManager::handleBasicGroupUpdated(qlonglong groupId) {
    if (chatType() == TDLibWrapper::ChatTypeBasicGroup && this->groupId() == groupId)
        emit groupInfoChanged();
}

void ChatManager::handleSupergroupUpdated(qlonglong groupId) {
    if (chatType() == TDLibWrapper::ChatTypeSupergroup && this->groupId() == groupId)
        emit groupInfoChanged();
}

void ChatManager::handleChatRolesUpdated(qlonglong chatId, const QVector<int> changedRoles) {
    if (this->chatId == chatId) {
        if (changedRoles.contains(ChatData::RolePhotoSmall)) {
            LOG("Chat photo updated" << chatId);
            emit smallPhotoChanged();
        }
        LOG("Chat roles updated" << chatId << changedRoles);
        emit chatInformationChanged();
    }
}

void ChatManager::handleChatPinnedMessageUpdated(qlonglong id, qlonglong pinnedMessageId) {
    if (id == chatId) {
        LOG("Pinned message updated" << chatId);
        this->pinnedMessageId = pinnedMessageId;
        emit pinnedMessageChanged();
    }
}

void ChatManager::handleChatActionUpdated(qlonglong chatId, const QVariantMap &sender, const QVariantMap &action, qlonglong messageThreadId) {
    const QString actionType = action.value(_TYPE).toString();
    if (messageThreadId == 0 && chatId == this->chatId) {
        LOG("Chat action updated");
        if (sender.value(_TYPE).toString() == "messageSenderChat") {
            const QString senderChatId = sender.value(CHAT_ID).toString();
            if (actionType == "chatActionCancel")
                chatActionsByChats.remove(senderChatId);
            else chatActionsByChats.insert(senderChatId, actionType);
        } else {
            const QString senderUserId = sender.value(USER_ID).toString();
            if (actionType == "chatActionCancel")
                chatActionsByUsers.remove(senderUserId);
            else chatActionsByUsers.insert(senderUserId, actionType);
        }
        LOG(chatActionsByChats << chatActionsByUsers << chatId << sender << action);
        emit chatActionsChanged();
    }
}


void ChatManager::reset(bool resetChatId) {
    LOG("Resetting chat manager resetChatId:" << resetChatId);
    if (this->chatMessagesModel)
        this->chatMessagesModel->reset();
    if (this->photoAndVideoMessagesModel)
        this->photoAndVideoMessagesModel->reset();
    if (this->animationMessagesModel)
        this->animationMessagesModel->reset();
    if (this->videoNoteMessagesModel)
        this->videoNoteMessagesModel->reset();
    if (this->topicsModel)
        this->topicsModel->reset();

    if (resetChatId) {
        chatId = 0;
        emit chatIdChanged();
        emit pendingJoinRequestsChanged();
    }

    mainModelsInitializationScheduled = false;
    mainModelsInitializationScheduledFromMessageId = 0;

    if (!chatActionsByUsers.isEmpty()) {
        chatActionsByUsers.clear();
        emit chatActionsChanged();
    }
    if (!chatActionsByChats.isEmpty()) {
        chatActionsByChats.clear();
        emit chatActionsChanged();
    }
    LOG("Finished resetting chat manager resetChatId:" << resetChatId);
}

void ChatManager::initializeMessageModels() {
    LOG("Initializing message models");
    chatMessagesModel = new ChatMessagesModel(tdLibWrapper, this->chatId, this);
    photoAndVideoMessagesModel = new MediaMessagesModel(tdLibWrapper, TDLibWrapper::SearchMessagesFilterPhotoAndVideo, this);
    animationMessagesModel = new MediaMessagesModel(tdLibWrapper, TDLibWrapper::SearchMessagesFilterAnimation, this);
    videoNoteMessagesModel = new MediaMessagesModel(tdLibWrapper, TDLibWrapper::SearchMessagesFilterVideoNote, this);
    emit messageModelsChanged();
}

void ChatManager::setChatId(qlonglong chatId) {
    if (this->chatId == chatId) {
        LOG("Chat ID" << chatId << "already set");
        return;
    }

    LOG("Setting chat ID to" << chatId);

    this->chatId = chatId;
    emit chatIdChanged();

    if (tdLibWrapper)
        finishInitialization();
    else {
        LOG("tdLibWrapper not yet set, not finishing initialization (will be done after it is set)");
    }
}

void ChatManager::finishInitialization() {
    tdLibWrapper->openChat(chatId);
    if (!pendingJoinRequests().isEmpty())
        emit pendingJoinRequestsChanged();
}

void ChatManager::initializeMainModels(qlonglong fromMessageId) {
    //doBasicInitialization(chatInformation);
    if (!tdLibWrapper) {
        LOG("tdLibWrapper not yet set, not initializing main models and scheduling instead");
        this->mainModelsInitializationScheduled = true;
        this->mainModelsInitializationScheduledFromMessageId = fromMessageId;
        return;
    }

    LOG("Initializing main models" << chatId << "from message id" << fromMessageId);

    reset(false);
    LOG("Reset for initializing main models done" << chatId);

    if (viewAsTopics()) {
        LOG("Initializing a forum chat");
        this->topicsModel = new ForumTopicsModel(tdLibWrapper, this);
        this->topicsModel->init(chatId);
    } else {
        LOG("Initializing a regular chat");
        initializeMessageModels();

        tdLibWrapper->getChatHistory(chatId, fromMessageId != 0 ? fromMessageId : this->chatInformation().value(LAST_READ_INBOX_MESSAGE_ID).toLongLong());
    }
}



void ChatManager::initializeMediaMessagesModel(MediaMessagesModel* model, qlonglong fromMessageId) {
    model->init(this->chatId, fromMessageId);
}

void ChatManager::initializeMediaMessagesModels() {
    initializeMediaMessagesModel(photoAndVideoMessagesModel);
    initializeMediaMessagesModel(animationMessagesModel);
    initializeMediaMessagesModel(videoNoteMessagesModel);
}

bool ChatManager::viewAsTopics() {
    // TODO
    return false;
}
