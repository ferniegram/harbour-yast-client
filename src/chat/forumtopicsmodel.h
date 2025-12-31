#ifndef FORUMTOPICSMODEL_H
#define FORUMTOPICSMODEL_H

#include <QAbstractListModel>

#include "tdlib/tdlibwrapper.h"
#include "forumtopic.h"

class ForumTopicsModel : public QAbstractListModel {
    Q_OBJECT
    Q_PROPERTY(qlonglong chatId MEMBER chatId NOTIFY chatIdChanged)

    friend class ForumTopicMessagesModel;

public:
    explicit ForumTopicsModel(TDLibWrapper *tdLibWrapper, Utilities *utilities, qlonglong chatId, QObject *parent = nullptr);

    virtual QHash<int,QByteArray> roleNames() const override;
    virtual int rowCount(const QModelIndex &) const override;
    virtual QVariant data(const QModelIndex &index, int role) const override;

    Q_INVOKABLE void reset();
    Q_INVOKABLE void loadMore();

signals:
    void chatIdChanged();
    void forumTopicUpdated(int forumTopicId);
    void forumTopicsReceived();

private slots:
    void handleForumTopicsReceived(qlonglong chatId, int totalCount, QVariantList newTopics, qint32 nextOffsetDate, qlonglong nextOffsetMessageId, int nextOffsetForumTopicId);
    void handleForumTopicUpdated(qlonglong chatId, int forumTopicId, const QVariantMap &update);
    void handleForumTopicInfoUpdated(qlonglong chatId, int forumTopicId, const QVariantMap &info);

private:
    ForumTopic *getTopic(int id);

private:
    TDLibWrapper *tdLibWrapper;
    Utilities *utilities;

    QList<ForumTopic*> topics;
    QHash<int, int> topicIndexMap;

    qlonglong chatId;
    qint32 nextOffsetDate;
    qlonglong nextOffsetMessageId;
    int nextOffsetForumTopicId;
    bool endReached;
};

#endif // FORUMTOPICSMODEL_H
