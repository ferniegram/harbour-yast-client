#include "forumtopicsmodel.h"

#define DEBUG_MODULE ChatManagerAndModel
#include "debuglog.h"

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
    const QString NAME("name");
}

ForumTopicsModel::ForumTopic::ForumTopic(const QVariantMap &forumTopic) :
    data(forumTopic),
    messageThreadId(data.value(INFO).toMap().value(MESSAGE_THREAD_ID).toLongLong())
{}

const QVector<int> ForumTopicsModel::ForumTopic::updateIsPinned(bool value) {
    if (data.value(IS_PINNED).toBool() != value) {
        data.insert(IS_PINNED, value);
        return QVector<int>{RoleIsPinned};
    }
    return QVector<int>();
}

const QVector<int> ForumTopicsModel::ForumTopic::updateLastReadInboxMessageId(qlonglong value) {
    if (data.value(LAST_READ_INBOX_MESSAGE_ID).toLongLong() != value) {
        data.insert(LAST_READ_INBOX_MESSAGE_ID, value);
        return QVector<int>{}; // TODO...
    }
    return QVector<int>();
}

const QVector<int> ForumTopicsModel::ForumTopic::updateLastReadOutboxMessageId(qlonglong value) {
    if (data.value(LAST_READ_OUTBOX_MESSAGE_ID).toLongLong() != value) {
        data.insert(LAST_READ_OUTBOX_MESSAGE_ID, value);
        return QVector<int>{}; // TODO...
    }
    return QVector<int>();
}

const QVector<int> ForumTopicsModel::ForumTopic::updateNotificationSettings(const QVariantMap &value) {
    if (data.value(NOTIFICATION_SETTINGS).toMap() != value) {
        data.insert(NOTIFICATION_SETTINGS, value);
        return QVector<int>{RoleNotificationSettings};
    }
    return QVector<int>();
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
    LOG("Initializing" << chatId);
    this->chatId = chatId;
    emit chatIdChanged();

    this->tdLibWrapper->getForumTopics(chatId);
}

QHash<int,QByteArray> ForumTopicsModel::roleNames() const {
    return QHash<int,QByteArray>{
        {RoleName, "name"},
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
        const ForumTopic *topic = topics.at(row);
        switch (role) {
        case RoleName:
            return topic->data.value(INFO).toMap().value(NAME);
        case RoleInfo:
            return topic->data.value(INFO).toMap();
        default:
            return QVariant();
        }
    }
    return QVariant();
}

void ForumTopicsModel::reset() {
    LOG("Resetting");
    chatId = 0;
    nextOffsetDate = 0;
    nextOffsetMessageId = 0;
    nextOffsetMessageThreadId = 0;
    emit chatIdChanged();
}

void ForumTopicsModel::loadMore() {
    if (chatId != 0 && nextOffsetDate != 0 && nextOffsetMessageId != 0 && nextOffsetMessageThreadId != 0) {
        this->tdLibWrapper->getForumTopics(chatId, nextOffsetDate, nextOffsetMessageId, nextOffsetMessageThreadId);
    }
}

void ForumTopicsModel::handleForumTopicsReceived(qlonglong chatId, int totalCount, QVariantList newTopics, qint32 nextOffsetDate, qlonglong nextOffsetMessageId, qlonglong nextOffsetMessageThreadId) {
    if (this->chatId == chatId) {
        LOG("Forum topics received" << totalCount);
        for (const QVariant &topicVariant : newTopics) {
            ForumTopic *topic = new ForumTopic(topicVariant.toMap());
            this->topics.append(topic);
            this->topicIndexMap.insert(topic->messageThreadId, this->topics.size() - 1);
        }

        this->nextOffsetDate = nextOffsetDate;
        this->nextOffsetMessageId = nextOffsetMessageId;
        this->nextOffsetMessageThreadId = nextOffsetMessageThreadId;
    }
}

void ForumTopicsModel::handleForumTopicUpdated(qlonglong chatId, qlonglong messageThreadId, bool isPinned, qlonglong lastReadInboxMessageId, qlonglong lastReadOutboxMessageId, const QVariantMap &notificationSettings) {
    if (this->chatId == chatId && topicIndexMap.contains(messageThreadId)) {
        LOG("Forum topic updated" << chatId << messageThreadId);

        const int topicIndex = topicIndexMap.value(messageThreadId);
        ForumTopic *topic = this->topics.value(topicIndex);
        QVector<int> changedRoles;

        changedRoles
                << topic->updateIsPinned(isPinned)
                << topic->updateLastReadInboxMessageId(lastReadInboxMessageId)
                << topic->updateLastReadOutboxMessageId(lastReadOutboxMessageId)
                << topic->updateNotificationSettings(notificationSettings);

        if (!changedRoles.isEmpty()) {
            const QModelIndex modelIndex = index(topicIndex);
            emit dataChanged(modelIndex, modelIndex, changedRoles);
        }
    }
}

void ForumTopicsModel::handleForumTopicInfoUpdated(qlonglong chatId, qlonglong messageThreadId, const QVariantMap &info) {
    if (this->chatId == chatId && topicIndexMap.contains(messageThreadId)) {
        LOG("Forum topic info updated" << chatId << messageThreadId);

        const int topicIndex = topicIndexMap.value(messageThreadId);
        ForumTopic *topic = this->topics.value(topicIndex);
        topic->data.insert(INFO, info);
        const QModelIndex modelIndex = index(topicIndex);
        emit dataChanged(modelIndex, modelIndex, QVector<int>{RoleInfo});
    }
}
