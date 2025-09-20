#include "forumtopicsmodel.h"

namespace {
    const QString INFO("info");
    const QString LAST_MESSAGE("last_message");
    const QString ORDER("order");
    const QString IS_PINNED("is_pinned");
    const QString UNREAD_COUNT("unread_count");
    const QString LAST_READ_INBOX_MESSAGE_ID("last_read_inbox_message_id");
    const QString LAST_READ_OUTBOX_MESSAGE_ID("last_read_outbox_message_id");
    const QString UNREAD_MENTION_COUNT("unread_mention_count");
    const QString UNREAD_REACTION_COUNT("unread_reaction_count");
    const QString NOTIFICATION_SETTINGS("notification_settings");
    const QString DRAFT_MESSAGE("draft_message");
    const QString MESSAGE_THREAD_ID("message_thread_id");
}

ForumTopicsModel::ForumTopic::ForumTopic(const QVariantMap &forumTopic) : data(forumTopic) {

}

qlonglong ForumTopicsModel::ForumTopic::messageThreadId() {
    return data.value(INFO).toMap().value(MESSAGE_THREAD_ID).toLongLong();
}

ForumTopicsModel::ForumTopicsModel(TDLibWrapper *tdLibWrapper, QObject *parent) :
    QAbstractListModel(parent),
    tdLibWrapper(tdLibWrapper),
    chatId(0),
    nextOffsetDate(0),
    nextOffsetMessageId(0),
    nextOffsetMessageThreadId(0)
{
    connect(tdLibWrapper, &TDLibWrapper::forumTopicsReceived, this, &ForumTopicsModel::handleForumTopicsReceived);
}

void ForumTopicsModel::init(qlonglong chatId) {
    this->chatId = chatId;
    emit chatIdChanged();

    this->tdLibWrapper->getForumTopics(chatId);
}

QHash<int,QByteArray> ForumTopicsModel::roleNames() const {
    return QHash<int,QByteArray>{
        {RoleInfo, "info"},
        //{RoleLastMessage, "last_message"}, // TODO
        {RoleOrder, "order"},
        {RoleIsPinned, "is_pinned"},
        {RoleUnreadCount, "unread_count"},
        {RoleUnreadMentionCount, "unread_mention_count"},
        {RoleUnreadReactionCount, "unread_reaction_count"},
        {RoleNotificationSettings, "notification_settings"},
        //{RoleDraftMessage, ""}
    };
}

int ForumTopicsModel::rowCount(const QModelIndex &) const {
    return topics.size();
}

QVariant ForumTopicsModel::data(const QModelIndex &index, int role) const {
    const int row = index.row();
    if (row >= 0 && row < topics.size()) {
        switch (role) {
        default:
            return QVariant();
        }
    }
    return QVariant();
}

void ForumTopicsModel::reset() {
    chatId = 0;
    nextOffsetDate = 0;
    nextOffsetMessageId = 0;
    nextOffsetMessageThreadId = 0;
    emit chatIdChanged();
}

void ForumTopicsModel::loadMore() {
    if (chatId != 0) {
        this->tdLibWrapper->getForumTopics(chatId, nextOffsetDate, nextOffsetMessageId, nextOffsetMessageThreadId);
    }
}

void ForumTopicsModel::handleForumTopicsReceived(qlonglong chatId, int totalCount, QVariantList newTopics, qint32 nextOffsetDate, qlonglong nextOffsetMessageId, qlonglong nextOffsetMessageThreadId) {
    if (this->chatId == chatId) {
        for (const QVariant &topicVariant : newTopics) {
            ForumTopic *topic = new ForumTopic(topicVariant.toMap());
            this->topics.append(topic);
            this->topicIndexMap.insert(topic->messageThreadId(), this->topics.size() - 1);
        }

        this->nextOffsetDate = nextOffsetDate;
        this->nextOffsetMessageId = nextOffsetMessageId;
        this->nextOffsetMessageThreadId = nextOffsetMessageThreadId;
    }
}
