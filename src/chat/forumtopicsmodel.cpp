#include "forumtopicsmodel.h"

#define DEBUG_MODULE ForumTopicsModel
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
    const QString FORUM_TOPIC_ID("forum_topic_id");
    const QString ID("id");
}

ForumTopicsModel::ForumTopic::ForumTopic(TDLibWrapper *tdLibWrapper, Utilities *utilities, const QVariantMap &forumTopic) :
    BaseMessagableData(tdLibWrapper, utilities),
    data(forumTopic),
    id(data.value(INFO).toMap().value(FORUM_TOPIC_ID).toLongLong())
{}

qlonglong ForumTopicsModel::ForumTopic::messageThreadId() const {
    return data.value(INFO).toMap().value(MESSAGE_THREAD_ID).toLongLong();
}

bool ForumTopicsModel::ForumTopic::isPinned() const {
    return data.value(IS_PINNED).toBool();
}

qlonglong ForumTopicsModel::ForumTopic::lastReadInboxMessageId() const {
    return data.value(LAST_READ_INBOX_MESSAGE_ID).toLongLong();
}

qlonglong ForumTopicsModel::ForumTopic::lastReadOutboxMessageId() const {
    return data.value(LAST_READ_OUTBOX_MESSAGE_ID).toLongLong();
}

const QVariantMap ForumTopicsModel::ForumTopic::lastMessage() const {
    return data.value(LAST_MESSAGE).toMap();
}

const QVariantMap ForumTopicsModel::ForumTopic::notificationSettings() const {
    return data.value(NOTIFICATION_SETTINGS).toMap();
}

const QVector<int> ForumTopicsModel::ForumTopic::updateIsPinned(bool value) {
    if (data.value(IS_PINNED).toBool() != value) {
        data.insert(IS_PINNED, value);
        return {RoleIsPinned};
    }
    return {};
}

const QVector<int> ForumTopicsModel::ForumTopic::updateLastReadInboxMessageId(qlonglong value) {
    if (data.value(LAST_READ_INBOX_MESSAGE_ID).toLongLong() != value) {
        data.insert(LAST_READ_INBOX_MESSAGE_ID, value);
        return {RoleLastReadInboxMessageId};
    }
    return {};
}

const QVector<int> ForumTopicsModel::ForumTopic::updateLastReadOutboxMessageId(qlonglong value) {
    if (data.value(LAST_READ_OUTBOX_MESSAGE_ID).toLongLong() != value) {
        data.insert(LAST_READ_OUTBOX_MESSAGE_ID, value);
        return {RoleLastReadOutboxMessageId};
    }
    return {};
}

const QVector<int> ForumTopicsModel::ForumTopic::updateNotificationSettings(const QVariantMap &value) {
    if (data.value(NOTIFICATION_SETTINGS).toMap() != value) {
        data.insert(NOTIFICATION_SETTINGS, value);
        return {RoleNotificationSettings};
    }
    return {};
}



ForumTopicsModel::ForumTopicsModel(TDLibWrapper *tdLibWrapper, Utilities *utilities, qlonglong chatId, QObject *parent) :
    QAbstractListModel(parent),
    tdLibWrapper(tdLibWrapper),
    utilities(utilities),
    chatId(0),
    nextOffsetDate(0),
    nextOffsetMessageId(0),
    nextOffsetMessageThreadId(0)
{
    LOG("Initializing" << chatId);

    connect(tdLibWrapper, &TDLibWrapper::forumTopicsReceived, this, &ForumTopicsModel::handleForumTopicsReceived);
    connect(tdLibWrapper, &TDLibWrapper::forumTopicUpdated, this, &ForumTopicsModel::handleForumTopicUpdated);
    connect(tdLibWrapper, &TDLibWrapper::forumTopicInfoUpdated, this, &ForumTopicsModel::handleForumTopicInfoUpdated);

    this->chatId = chatId;
    this->tdLibWrapper->getForumTopics(chatId);
}

QHash<int,QByteArray> ForumTopicsModel::roleNames() const {
    return QHash<int,QByteArray>{
        {RoleId, "forum_topic_id"},
        {RoleName, "name"},
        {RoleInfo, "info"},
        //{RoleLastMessage, "last_message"}, // TODO
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
        case RoleId:
            return topic->id;
        case RoleName:
            return topic->data.value(INFO).toMap().value(NAME).toString();
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
        beginInsertRows(QModelIndex(), topics.length(), topics.length() + newTopics.length() - 1);
        for (const QVariant &topicVariant : newTopics) {
            ForumTopic *topic = new ForumTopic(tdLibWrapper, utilities, topicVariant.toMap());
            this->topics.append(topic);
            this->topicIndexMap.insert(topic->id, this->topics.size() - 1);
        }
        endInsertRows();

        this->nextOffsetDate = nextOffsetDate;
        this->nextOffsetMessageId = nextOffsetMessageId;
        this->nextOffsetMessageThreadId = nextOffsetMessageThreadId;
    }
}

void ForumTopicsModel::handleForumTopicUpdated(qlonglong chatId, int forumTopicId, const QVariantMap &update) {
    if (this->chatId == chatId && topicIndexMap.contains(forumTopicId)) {
        LOG("Forum topic updated" << chatId << forumTopicId);

        const int topicIndex = topicIndexMap.value(forumTopicId);
        ForumTopic *topic = this->topics.value(topicIndex);
        QVector<int> changedRoles;

        changedRoles
                << topic->updateIsPinned(update.value(IS_PINNED).toBool())
                << topic->updateLastReadInboxMessageId(update.value(LAST_READ_INBOX_MESSAGE_ID).toLongLong())
                << topic->updateLastReadOutboxMessageId(update.value(LAST_READ_OUTBOX_MESSAGE_ID).toLongLong())
                << topic->updateNotificationSettings(update.value(NOTIFICATION_SETTINGS).toMap());

        if (!changedRoles.isEmpty()) {
            const QModelIndex modelIndex = index(topicIndex);
            emit dataChanged(modelIndex, modelIndex, changedRoles);
            emit forumTopicUpdated(forumTopicId);
        }
    }
}

void ForumTopicsModel::handleForumTopicInfoUpdated(qlonglong chatId, int forumTopicId, const QVariantMap &info) {
    if (this->chatId == chatId && topicIndexMap.contains(forumTopicId)) {
        LOG("Forum topic info updated" << chatId << forumTopicId);

        const int topicIndex = topicIndexMap.value(forumTopicId);
        ForumTopic *topic = this->topics.value(topicIndex);
        topic->data.insert(INFO, info);
        const QModelIndex modelIndex = index(topicIndex);
        emit dataChanged(modelIndex, modelIndex, QVector<int>{RoleInfo}); // TODO
        emit forumTopicUpdated(forumTopicId);
    }
}

ForumTopicsModel::ForumTopic *ForumTopicsModel::getTopic(int id) {
    return topicIndexMap.contains(id) ? topics.value(topicIndexMap.value(id)) : nullptr;
}
