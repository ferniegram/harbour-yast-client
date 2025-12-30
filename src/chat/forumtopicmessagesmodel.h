#ifndef FORUMTOPICMESSAGESMODEL_H
#define FORUMTOPICMESSAGESMODEL_H

#include "readablemessagesmodel.h"
#include "forumtopicsmodel.h"

class ForumTopicMessagesModel : public ReadableMessagesModel {
    Q_OBJECT
    Q_PROPERTY(QObject* tdlib MEMBER tdLibWrapper WRITE setTDLibWrapper NOTIFY tdlibChanged)
    Q_PROPERTY(QObject* forumTopicsModel MEMBER forumTopicsModel WRITE setForumTopicsModel NOTIFY forumTopicsModelChanged)
    Q_PROPERTY(qlonglong chatId MEMBER chatId WRITE setChatId NOTIFY chatIdChanged)
    Q_PROPERTY(int forumTopicId MEMBER forumTopicId WRITE setForumTopicId NOTIFY forumTopicIdChanged)

public:
    ForumTopicMessagesModel(QObject *parent = nullptr);
    ForumTopicMessagesModel(TDLibWrapper *tdLibWrapper, qlonglong chatId, int forumTopicId, QObject *parent = nullptr);

    void setTDLibWrapper(QObject* obj);
    void setForumTopicsModel(QObject* obj);
    void setChatId(qlonglong chatId);
    void setForumTopicId(int forumTopicId);

    Q_INVOKABLE virtual bool clear() override;
    Q_INVOKABLE void setSearchQuery(const QString newSearchQuery);

signals:
    void tdlibChanged();
    void forumTopicsModelChanged();
    void chatIdChanged();
    void forumTopicIdChanged();

protected:
    virtual void setupTDLibWrapper() override;

    virtual void loadMessages(int extra, qlonglong fromMessageId, int offset = -1) override;
    virtual inline bool canLoadMoreMessages() const override { return searchQuery.isEmpty(); }

    virtual qlonglong lastReadInboxMessageId() const override;
    virtual qlonglong lastReadOutboxMessageId() const override;
    virtual qlonglong lastMessageId() const override;

private:
    ForumTopicsModel::ForumTopic *getTopic() const;
    void initialize();

private slots:
    void handleForumTopicUpdated(int forumTopicId);
    void handleForumTopicMessagesReceived(qlonglong chatId, int forumTopicId, int extra, const QVariantList &messages, int totalCount);
    void handleNewMessageReceived(qlonglong chatId, const QVariantMap &message);

private:
    ForumTopicsModel *forumTopicsModel;

    using ForumTopic = ForumTopicsModel::ForumTopic;

    bool initialized;
    int forumTopicId;
    QString searchQuery;
};

#endif // FORUMTOPICMESSAGESMODEL_H
