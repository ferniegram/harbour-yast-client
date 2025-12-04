#ifndef CHATMANAGER_H
#define CHATMANAGER_H

#include "readablemessagesmodel.h"
#include "mediamessagesmodel.h"
#include "forumtopicsmodel.h"

class ChatMessagesModel : public ReadableMessagesModel {
    Q_OBJECT
public:
    ChatMessagesModel(TDLibWrapper *tdLibWrapper, qlonglong chatId, QObject *parent = nullptr);

    Q_INVOKABLE virtual bool clear() override;
    Q_INVOKABLE void setSearchQuery(const QString newSearchQuery);

    friend class ChatManager;

protected:
    virtual void loadMessages(qlonglong fromMessageId, int offset = -1) override;
    virtual inline bool canLoadMoreMessages() const override { return searchQuery.isEmpty(); }

    virtual qlonglong lastReadInboxMessageId() const override;
    virtual qlonglong lastReadOutboxMessageId() const override;
    virtual qlonglong lastMessageId() const override; // FIXME: this is (might be) wrong and shouldn't be used ideally

private:
    QString searchQuery;
};

class ChatManager : public QObject {
    Q_OBJECT
    Q_PROPERTY(QObject* tdlib MEMBER tdLibWrapper WRITE setTDLibWrapper NOTIFY tdlibChanged)
    Q_PROPERTY(qlonglong chatId MEMBER chatId WRITE setChatId NOTIFY chatIdChanged)
    Q_PROPERTY(bool infoInitialized READ infoInitialized NOTIFY chatIdChanged)
    Q_PROPERTY(QVariantMap chatInformation READ chatInformation NOTIFY chatInformationChanged)
    Q_PROPERTY(bool viewAsTopics READ viewAsTopics NOTIFY viewAsTopicsChanged)
    Q_PROPERTY(TDLibWrapper::ChatType chatType READ chatType NOTIFY chatIdChanged)
    Q_PROPERTY(bool isChannel READ isChannel NOTIFY chatIdChanged)
    Q_PROPERTY(QVariant userInfo READ userInfo NOTIFY userInfoChanged)
    Q_PROPERTY(QVariant groupInfo READ groupInfo NOTIFY groupInfoChanged)

    Q_PROPERTY(QVariantMap smallPhoto READ smallPhoto NOTIFY smallPhotoChanged)
    Q_PROPERTY(QVariantMap pendingJoinRequests READ pendingJoinRequests NOTIFY pendingJoinRequestsChanged)

    Q_PROPERTY(ChatMessagesModel* model MEMBER chatMessagesModel NOTIFY messageModelsChanged)

    Q_PROPERTY(MediaMessagesModel* photoAndVideoMessagesModel MEMBER photoAndVideoMessagesModel NOTIFY messageModelsChanged)
    Q_PROPERTY(MediaMessagesModel* animationMessagesModel MEMBER animationMessagesModel NOTIFY messageModelsChanged)
    Q_PROPERTY(MediaMessagesModel* videoNoteMessagesModel MEMBER videoNoteMessagesModel NOTIFY messageModelsChanged)

    Q_PROPERTY(ForumTopicsModel* topicsModel MEMBER topicsModel NOTIFY topicsModelChanged)

    Q_PROPERTY(qlonglong pinnedMessageId MEMBER pinnedMessageId NOTIFY pinnedMessageChanged)
    Q_PROPERTY(QVariantMap chatActionsByUsers MEMBER chatActionsByUsers NOTIFY chatActionsChanged)
    Q_PROPERTY(QVariantMap chatActionsByChats MEMBER chatActionsByChats NOTIFY chatActionsChanged)

public:
    ChatManager(QObject *parent = nullptr);
    ~ChatManager();

    void setTDLibWrapper(QObject* obj);

    Q_INVOKABLE void reset(bool resetChatId = true);
    void setChatId(qlonglong chatId);
    Q_INVOKABLE void initializeMainModels(qlonglong fromMessageId = 0);
    Q_INVOKABLE void initializeMediaMessagesModel(MediaMessagesModel* model, qlonglong fromMessageId = 0);
    Q_INVOKABLE void initializeMediaMessagesModels();
    bool viewAsTopics();
    inline qlonglong getChatId() { return chatId; }
    inline bool infoInitialized() { return chatId != 0; }
    inline QVariantMap chatInformation() const {
        if (tdLibWrapper)
            return tdLibWrapper->getChat(chatId);
        return QVariantMap();
    }

    TDLibWrapper::ChatType chatType() const;
    bool isChannel() const;
    QVariant userInfo() const;
    QVariant groupInfo() const;

    QVariantMap smallPhoto() const;
    QVariantMap pendingJoinRequests() const;

signals:
    void tdlibChanged();
    void messageModelsChanged();
    void topicsModelChanged();
    void chatIdChanged();
    void pinnedMessageChanged();
    void chatActionsChanged();
    void chatInformationChanged();
    void viewAsTopicsChanged();
    void userInfoChanged();
    void groupInfoChanged();

    void smallPhotoChanged();
    void pendingJoinRequestsChanged();

private slots:
    void handleChatReadInboxUpdated(const QString &id);
    void handleChatReadOutboxUpdated(const QString &id);
    void handleChatRolesUpdated(qlonglong chatId, const QVector<int> changedRoles = QVector<int>());
    void handleChatPendingJoinRequestsUpdated(qlonglong chatId);
    void handleChatPinnedMessageUpdated(qlonglong chatId, qlonglong pinnedMessageId);
    void handleChatActionUpdated(qlonglong chatId, const QVariantMap &sender, const QVariantMap &chatAction, qlonglong messageThreadId);
    void handleUserUpdated(const QString &userId);
    void handleBasicGroupUpdated(qlonglong groupId);
    void handleSupergroupUpdated(qlonglong groupId);

private:
    qlonglong userId() const;
    qlonglong groupId() const;

    void finishInitialization();
    void initializeMessageModels();

private:
    TDLibWrapper *tdLibWrapper;

    qlonglong chatId;
    qlonglong pinnedMessageId;
    bool mainModelsInitializationScheduled;
    qlonglong mainModelsInitializationScheduledFromMessageId;

    ChatMessagesModel *chatMessagesModel;
    MediaMessagesModel* photoAndVideoMessagesModel;
    MediaMessagesModel* animationMessagesModel;
    MediaMessagesModel* videoNoteMessagesModel;
    ForumTopicsModel *topicsModel;

    QVariantMap chatActionsByUsers; // QMap<qlonglong, QString>
    QVariantMap chatActionsByChats; //QMap<qlonglong, QString>
};

#endif // CHATMANAGER_H
