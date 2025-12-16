#ifndef READABLEMESSAGESMODEL_H
#define READABLEMESSAGESMODEL_H

#include "jumpablemessagesmodel.h"

class ReadableMessagesModel : public JumpableMessagesModel {
    Q_OBJECT
    Q_PROPERTY(int lastReadMessageIndexInBounds READ calculateLastReadMessageIndexInBounds NOTIFY lastReadMessageIndexChanged)
    Q_PROPERTY(int lastReadIncomingMessageIndex READ getLastReadMessageIndex NOTIFY lastReadMessageIndexChanged)

    Q_PROPERTY(int lastReadSentMessageIndex READ calculateLastReadSentMessageIndex NOTIFY lastReadSentMessageUpdated)

    Q_PROPERTY(bool containsSponsoredMessages MEMBER containsSponsoredMessages NOTIFY containsSponsoredMessagesChanged)

public:
    ReadableMessagesModel(TDLibWrapper *tdLibWrapper, QObject *parent = nullptr);

    Q_INVOKABLE virtual bool clear() override;
    Q_INVOKABLE void loadEnd(bool markAllAsRead = false);
    Q_INVOKABLE virtual int calculateScrollPosition() override;

signals:
    void newMessageReceived(const QVariantMap &message);
    void unreadCountUpdated(int unreadCount, const QString &lastReadInboxMessageId);

    void lastReadSentMessageUpdated();
    void lastReadMessageIndexChanged();

    void containsSponsoredMessagesChanged();

private slots:
    void handleFoundChatMessagesReceived(qlonglong chatId, TDLibWrapper::SearchMessagesFilter filter, int extra, const QVariantList &messages, int totalCount, qlonglong /*nextFromMessageId*/);
    void handleSponsoredMessagesReceived(qlonglong chatId, const QVariantList &sponsoredMessages, int messagesBetween);
    void handleNewMessageReceived(qlonglong chatId, const QVariantMap &message);

protected:
    int calculateLastReadMessageIndexInBounds();

    int getLastReadMessageIndex();
    int calculateLastReadSentMessageIndex();

    virtual void appendMessages(const QList<MessageData*> newMessages, bool updateIsLastInSequence = true) override;

    virtual void loadMoreHistoryImpl() override;
    virtual void loadMoreFutureImpl() override;
    virtual void loadHistoryForMessageImpl(qlonglong messageId) override;

    virtual qlonglong lastReadInboxMessageId() const = 0;
    virtual qlonglong lastReadOutboxMessageId() const = 0;
    virtual qlonglong lastMessageId() const = 0;

    void insertSponsoredMessage(int insertIndex, const QVariantMap &message, qlonglong messageId);

protected slots:
    virtual void handlePrepareMessagesReceived(int totalCount, UpdateType fromUpdate) override;

protected:
    bool loadingFullEnd;
    bool containsSponsoredMessages;
    QVariantList pendingSponsoredMessages;
    int sponsoredMessagesMessagesBetween;
};

#endif // READABLEMESSAGESMODEL_H
