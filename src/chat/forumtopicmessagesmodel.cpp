#include "forumtopicmessagesmodel.h"

#define DEBUG_MODULE ForumTopicMessagesModel
#include "debuglog.h"

namespace {
    const QString ID("id");
    const QString TOPIC_ID("topic_id");
    const QString FORUM_TOPIC_ID("forum_topic_id");
}

ForumTopicMessagesModel::ForumTopicMessagesModel(QObject *parent) : ReadableMessagesModel(), forumTopicsModel(nullptr), initialized(false), forumTopicId(0) {
}

ForumTopicMessagesModel::ForumTopicMessagesModel(TDLibWrapper *tdLibWrapper, qlonglong chatId, int forumTopicId, QObject *parent)
    : ReadableMessagesModel(tdLibWrapper, parent),
      initialized(false),
      forumTopicId(forumTopicId)
{
    this->chatId = chatId;
}

void ForumTopicMessagesModel::setTDLibWrapper(QObject *obj) {
    TDLibWrapper *wrapper = qobject_cast<TDLibWrapper*>(obj);
    if (tdLibWrapper != wrapper) {
        tdLibWrapper = wrapper;
        LOG("TDLibWrapper set" << wrapper);
        emit tdlibChanged();

        if (tdLibWrapper) {
            setupTDLibWrapper();

            initialize();
        }
    }
}

void ForumTopicMessagesModel::setupTDLibWrapper() {
    ReadableMessagesModel::setupTDLibWrapper();

    connect(tdLibWrapper, &TDLibWrapper::forumTopicMessagesReceived, this, &ForumTopicMessagesModel::handleForumTopicMessagesReceived);
    connect(this->tdLibWrapper, &TDLibWrapper::newMessageReceived, this, &ForumTopicMessagesModel::handleNewMessageReceived);
}

void ForumTopicMessagesModel::setForumTopicsModel(QObject *obj) {
    ForumTopicsModel *model = qobject_cast<ForumTopicsModel*>(obj);
    if (forumTopicsModel != model) {
        LOG("ForumTopicsModel set" << model);
        forumTopicsModel = model;
        emit forumTopicsModelChanged();

        if (forumTopicsModel) {
            connect(forumTopicsModel, &ForumTopicsModel::forumTopicUpdated, this, &ForumTopicMessagesModel::handleForumTopicUpdated);

            initialize();
        }
    }
}

void ForumTopicMessagesModel::handleForumTopicUpdated(int forumTopicId) {
    if (this->forumTopicId == forumTopicId) {
        LOG("Forum topic info updated" << forumTopicId);
        // TODO
    }
}

void ForumTopicMessagesModel::setChatId(qlonglong chatId) {
    if (this->chatId != chatId) {
        LOG("Chat ID set" << chatId);
        this->chatId = chatId;
        emit chatIdChanged();

        initialize();
    }
}

void ForumTopicMessagesModel::setForumTopicId(int forumTopicId) {
    if (this->forumTopicId != forumTopicId) {
        LOG("Forum topic ID set" << forumTopicId);
        this->forumTopicId = forumTopicId;
        emit forumTopicIdChanged();

        initialize();
    }
}

void ForumTopicMessagesModel::initialize() {
    if (!initialized && tdLibWrapper && forumTopicsModel && chatId && forumTopicId) {
        LOG("Initializing");
        initialized = true;

        // todo...
        this->loadMessages(UpdateInitial, lastReadInboxMessageId());
    }
}

bool ForumTopicMessagesModel::clear() {
    LOG("Clearing forum topic messages model");
    this->searchQuery.clear();
    return ReadableMessagesModel::clear();
}

void ForumTopicMessagesModel::loadMessages(int extra, qlonglong fromMessageId, int offset) {
    if (searchQuery.isEmpty())
        this->tdLibWrapper->getForumTopicHistory(chatId, forumTopicId, extra, fromMessageId, offset);
    // TODO: support search
    //else
        // ignore offset for now
        //this->tdLibWrapper->searchChatMessages(chatId, searchQuery, extra, fromMessageId);
}

void ForumTopicMessagesModel::setSearchQuery(const QString newSearchQuery) {
    if (this->searchQuery != newSearchQuery) {
        this->clear();
        this->searchQuery = newSearchQuery;
        this->loadMessages(UpdateInitial, searchQuery.isEmpty() ? lastReadInboxMessageId() : 0);
    }
}

inline ForumTopicsModel::ForumTopic *ForumTopicMessagesModel::getTopic() const {
    return forumTopicsModel ? forumTopicsModel->getTopic(forumTopicId) : nullptr;
}

qlonglong ForumTopicMessagesModel::lastReadInboxMessageId() const {
    ForumTopic *topic = getTopic();
    return topic ? topic->lastReadInboxMessageId() : 0;
}
qlonglong ForumTopicMessagesModel::lastReadOutboxMessageId() const {
    ForumTopic *topic = getTopic();
    return topic ? topic->lastReadOutboxMessageId() : 0;
}
qlonglong ForumTopicMessagesModel::lastMessageId() const {
    ForumTopic *topic = getTopic();
    return topic ? topic->lastMessage().value(ID).toLongLong() : 0;
}

void ForumTopicMessagesModel::handleForumTopicMessagesReceived(qlonglong chatId, int forumTopicId, int extra, const QVariantList &messages, int totalCount) {
    if (this->chatId == chatId && this->forumTopicId == forumTopicId) {
        LOG("Messages received");
        handleMessagesReceived(extra, messages, totalCount);
    }
}

void ForumTopicMessagesModel::handleNewMessageReceived(qlonglong chatId, const QVariantMap &message) {
    if (this->chatId == chatId && this->forumTopicId == message.value(TOPIC_ID).toMap().value(FORUM_TOPIC_ID).toInt())
        ReadableMessagesModel::handleNewMessageReceived(message);
}
