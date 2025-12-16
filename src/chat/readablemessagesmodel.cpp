#include "readablemessagesmodel.h"

#define DEBUG_MODULE ReadableMessagesModel
#include "debuglog.h"

namespace {
    const QString ID("id");
    const QString CHAT_ID("chat_id");
    const QString LAST_READ_INBOX_MESSAGE_ID("last_read_inbox_message_id");
    const QString LAST_READ_OUTBOX_MESSAGE_ID("last_read_outbox_message_id");
    const QString MESSAGE_ID("message_id");
}

ReadableMessagesModel::ReadableMessagesModel(TDLibWrapper *tdLibWrapper, QObject *parent) :
    JumpableMessagesModel(tdLibWrapper, parent),
    loadingFullEnd(false),
    containsSponsoredMessages(false),
    sponsoredMessagesMessagesBetween(0)
{
    connect(this->tdLibWrapper, SIGNAL(messagesReceived(qlonglong, int, const QVariantList &, int)), this, SLOT(handleMessagesReceived(qlonglong, int, const QVariantList &, int)));
    connect(this->tdLibWrapper, &TDLibWrapper::foundChatMessagesReceived, this, &ReadableMessagesModel::handleFoundChatMessagesReceived);
    connect(this->tdLibWrapper, &TDLibWrapper::sponsoredMessagesReceived, this, &ReadableMessagesModel::handleSponsoredMessagesReceived);
    connect(this->tdLibWrapper, &TDLibWrapper::newMessageReceived, this, &ReadableMessagesModel::handleNewMessageReceived);

    connect(this, &ReadableMessagesModel::messageSendSucceeded, this, &ReadableMessagesModel::lastReadSentMessageUpdated);
    connect(this, &ReadableMessagesModel::messagesReceived, this, &ReadableMessagesModel::lastReadSentMessageUpdated);

    // FIXME: can this be implemented better?
    connect(this, &ReadableMessagesModel::messagesReceived, this, &ReadableMessagesModel::lastReadMessageIndexChanged);
    connect(this, &ReadableMessagesModel::newMessageReceived, this, &ReadableMessagesModel::lastReadMessageIndexChanged);
    connect(this, &ReadableMessagesModel::unreadCountUpdated, this, &ReadableMessagesModel::lastReadMessageIndexChanged);

}

bool ReadableMessagesModel::clear() {
    LOG("Clearing readable messages model");
    loadingFullEnd = false;
    if (JumpableMessagesModel::clear()) {
        emit lastReadSentMessageUpdated();
        return true;
    }
    return false;
}

int ReadableMessagesModel::getLastReadMessageIndex() {
    int listInboxPosition = messageIndexMap.value(lastReadInboxMessageId(), -1);
    if (listInboxPosition > messages.size() - 1) listInboxPosition = -1;
    return listInboxPosition;
}

int ReadableMessagesModel::calculateLastReadSentMessageIndex() {
    LOG("calculateLastReadSentMessageIndex");
    qlonglong id = lastReadOutboxMessageId();
    LOG("lastReadSentMessageId" << id);
    LOG("size messageIndexMap" << messageIndexMap.size());
    LOG("contains ID?" << messageIndexMap.contains(id));
    int listOutboxPosition;
    if (messageIndexMap.contains(id))
        listOutboxPosition = messageIndexMap.value(id, -1);
    else {
        LOG("Last read sent message is not loaded, falling back to last loaded sent message");
        listOutboxPosition = findLastSentMessageIndex();
    }
    LOG("Last read sent message" << id << "is at position" << listOutboxPosition);
    return listOutboxPosition;
}

int ReadableMessagesModel::calculateScrollPosition() {
    if (loadingFullEnd) return this->messages.size() - 1;

    int scrollPosition = this->messageIndexMap.value(this->highlightedMessageId, -1);
    if (scrollPosition == -1) {
        LOG("calculateLastScrollMessageIndex");

        int listInboxPosition = this->messageIndexMap.value(lastReadInboxMessageId(), -1);
        int listOwnPosition = findLastSentMessageIndex();

        if (listInboxPosition > messages.size() - 1) listInboxPosition = -1;
        if (listOwnPosition > messages.size() - 1) listOwnPosition = -1;

        LOG("Last read received message is at position" << listInboxPosition << "; last read sent message is at position" << listOwnPosition);

        scrollPosition = qMax(listInboxPosition, listOwnPosition);
    }

    LOG("Calculating new scroll position, current:" << scrollPosition << ", list size:" << this->messages.size());
    return qMin(scrollPosition + 1, this->messages.size() - 1);
}

void ReadableMessagesModel::handlePrepareMessagesReceived(int totalCount, UpdateType fromUpdate) {
    LOG("Updating start/end reached values");
    LOG(lastMessageId() << this->messageIndexMap);

    if (this->messageIndexMap.contains(lastMessageId())) {
        endReached = true;
        LOG("Last message is in the model, end was reached");
    }

    JumpableMessagesModel::handlePrepareMessagesReceived(totalCount, fromUpdate);


    // Insert pending sponsored messages
    if (containsSponsoredMessages && messages.size() > 0 && !pendingSponsoredMessages.isEmpty()) {
        if (fromUpdate == UpdatePreviousSlice) {
            int firstSponsored = 0;
            for (; firstSponsored < messages.size(); firstSponsored++)
                if (messages.at(firstSponsored)->isSponsored)
                    break;

            int i = 0;
            for (int insertIndex = firstSponsored - sponsoredMessagesMessagesBetween;
                 i < pendingSponsoredMessages.size() && insertIndex >= 0;
                 i++, insertIndex -= sponsoredMessagesMessagesBetween) {
                const QVariantMap message = pendingSponsoredMessages.at(i).toMap();
                insertSponsoredMessage(insertIndex, message, message.value(ID).toLongLong());
            }

            pendingSponsoredMessages.erase(pendingSponsoredMessages.begin(), pendingSponsoredMessages.begin() + i);
        } else if (fromUpdate == UpdateNextSlice) {
            int lastSponsored = messages.size() - 1;
            for (; lastSponsored >= 0; lastSponsored--)
                if (messages.at(lastSponsored)->isSponsored)
                    break;

            int i = 0;
            for (int insertIndex = lastSponsored + 1 + sponsoredMessagesMessagesBetween;
                 i < pendingSponsoredMessages.size() && insertIndex <= messages.size();
                 i++, insertIndex += 1 + sponsoredMessagesMessagesBetween) {
                const QVariantMap message = pendingSponsoredMessages.at(i).toMap();
                insertSponsoredMessage(insertIndex, message, message.value(ID).toLongLong());
            }

            pendingSponsoredMessages.erase(pendingSponsoredMessages.begin(), pendingSponsoredMessages.begin() + i);
        }
    }
}

int ReadableMessagesModel::calculateLastReadMessageIndexInBounds() {
    LOG("calculateLastReadMessageIndexInBounds");
    const qlonglong lastReadMessageId = lastReadInboxMessageId(); // last read incoming message id

    LOG("lastReadMessageId" << lastReadMessageId);
    LOG("size messageIndexMap" << messageIndexMap.size()
        << "; contains last read ID?" << messageIndexMap.contains(lastReadMessageId)
        );

    int listInboxPosition = messageIndexMap.value(lastReadMessageId, messages.size() - 1);
    int listOwnPosition = findLastSentMessageIndex();

    if (listInboxPosition > messages.size() - 1)
        listInboxPosition = messages.size() - 1;
    if (listOwnPosition > messages.size() - 1)
        listOwnPosition = -1;

    LOG("Last known message is at position" << listInboxPosition << "; last own message is at position" << listOwnPosition);

    return qMax(listInboxPosition, listOwnPosition);
}


void ReadableMessagesModel::loadMoreHistoryImpl() {
    this->loadMessages(UpdatePreviousSlice, messages.first()->messageId);
}
void ReadableMessagesModel::loadMoreFutureImpl() {
    this->loadMessages(UpdateNextSlice, messages.last()->messageId, -49);
}
void ReadableMessagesModel::loadHistoryForMessageImpl(qlonglong messageId) {
    this->loadMessages(UpdateInitial, messageId);
}

void ReadableMessagesModel::handleFoundChatMessagesReceived(qlonglong chatId, TDLibWrapper::SearchMessagesFilter filter, int extra, const QVariantList &messages, int totalCount, qlonglong /*nextFromMessageId*/) {
    if (this->chatId == chatId && filter == TDLibWrapper::SearchMessagesFilterEmpty) {
        LOG("Found chat messages received");
        handleMessagesReceived(extra, messages, totalCount);
    }
}

void ReadableMessagesModel::insertSponsoredMessage(int insertIndex, const QVariantMap &message, qlonglong messageId) {
    LOG("New sponsored message will be added:" << messageId << "at" << insertIndex);

    beginInsertRows(QModelIndex(), insertIndex, insertIndex);
    messages.insert(insertIndex, new MessageData(message, messageId));
    for (int j = insertIndex; j < messages.size(); j++)
        messageIndexMap.insert(messages.at(j)->messageId, j);
    endInsertRows();


    if (insertIndex > 0) {
        QModelIndex modelIndex = index(insertIndex - 1);
        emit dataChanged(modelIndex, modelIndex, QVector<int>{MessageData::RoleIsLastInSequence});
    }
    if (insertIndex + 1 < messages.size()) {
        QModelIndex modelIndex = index(insertIndex + 1);
        emit dataChanged(modelIndex, modelIndex, QVector<int>{MessageData::RoleIsFirstInSequence});
    }
}

void ReadableMessagesModel::appendMessages(const QList<MessageData *> newMessages, bool updateIsLastInSequence) {
    LOG("Appending" << newMessages.size() << "messages");
    if (containsSponsoredMessages && sponsoredMessagesMessagesBetween == 0) {
        LOG("Contains a single sponsored message, inserting before it");
        int lastIndex = messages.size() - 1;
        for (; lastIndex >= 0; lastIndex--) {
            if (!messages.at(lastIndex)->isSponsored)
                break;
        }
        insertMessagesAt(lastIndex, newMessages, updateIsLastInSequence);
    } else
        // Have multiple sponsored messages, don't move anything and instead add pending ones when needed
        MessagesModel::appendMessages(newMessages, updateIsLastInSequence);
}

void ReadableMessagesModel::handleSponsoredMessagesReceived(qlonglong chatId, const QVariantList &sponsoredMessages, int messagesBetween) {
    if (this->chatId != chatId)
        return;

    LOG("Handling sponsored messages" << chatId << sponsoredMessages.size() << messagesBetween);
    if (sponsoredMessages.length() == 0) {
        LOG("No sponsored messages");
        return;
    }

    if (messagesBetween == 0) {
        const QVariantMap message = sponsoredMessages.at(0).toMap();
        const qlonglong messageId = message.value(MESSAGE_ID).toLongLong();
        if (messageId && !messageIndexMap.contains(messageId)) {
            LOG("Single sponsored message will be added:" << messageId);
            appendMessages({new MessageData(message, messageId)});
            this->pendingSponsoredMessages.empty();
        }
    } else {
        int insertIndex = messages.size();
        for (int i=0; i < sponsoredMessages.size(); i++) {
            const QVariantMap message = sponsoredMessages.at(i).toMap();

            const qlonglong messageId = message.value(MESSAGE_ID).toLongLong();
            if (messageId && !messageIndexMap.contains(messageId)) {
                insertSponsoredMessage(insertIndex, message, messageId);

                insertIndex -= messagesBetween;
                if (insertIndex < 0) {
                    this->pendingSponsoredMessages = sponsoredMessages.mid(i + 1);
                    this->sponsoredMessagesMessagesBetween = messagesBetween;
                    LOG("Received" << this->pendingSponsoredMessages.size() << "extra sponsored messages, saving for later use");
                    // TODO: actually add pending sponsored messages
                    break;
                }
            }
        }
    }

    if (!containsSponsoredMessages) {
        containsSponsoredMessages = true;
        emit containsSponsoredMessagesChanged();
    }
    sponsoredMessagesMessagesBetween = messagesBetween;
}

void ReadableMessagesModel::handleNewMessageReceived(qlonglong chatId, const QVariantMap &message) {
    const qlonglong messageId = message.value(ID).toLongLong();
    if (chatId == this->chatId && !messageIndexMap.contains(messageId)) {
        if (canLoadMoreMessages() && this->endReached) {
            LOG("New message received for this chat");
            QList<MessageData*> messagesToBeAdded;
            messagesToBeAdded.append(new MessageData(message, messageId));
            insertMessages(messagesToBeAdded);
            setMessagesAlbum(messagesToBeAdded);
            emit newMessageReceived(message);
        } else {
            LOG("New message in this chat, but not relevant as less recent messages need to be loaded first!");
        }
    }
}


void ReadableMessagesModel::loadEnd(bool markAllAsRead) {
    if (!this->waitingForSlice() && !waitingFor.value(UpdateReload) && !messages.isEmpty()) {
        LOG("Loading end of the chat... markAllAsRead:" << markAllAsRead << (markAllAsRead ? 0 : lastReadOutboxMessageId()) << chatId);

        //if (markAllAsRead) // FIXME: is this really needed?
        //    this->tdLibWrapper->toggleChatIsMarkedAsUnread(this->chatId, false);
        this->loadingFullEnd = markAllAsRead; // doesn't seem to always work (also a similar issue with search)

        this->clear();
        this->loadMessages(UpdateInitial, markAllAsRead ? 0 : lastReadOutboxMessageId());
    }
}
