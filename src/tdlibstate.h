#ifndef TDLIBSTATE_H
#define TDLIBSTATE_H

#include <QObject>
#include "tdlibreceiver.h"

class TDLibState : public QObject {
    Q_OBJECT
public:
    enum AuthorizationState {
        Closed,
        Closing,
        LoggingOut,
        AuthorizationReady,
        WaitCode,
        WaitEmailAddress,
        WaitEmailCode,
        WaitOtherDeviceConfirmation,
        WaitPassword,
        WaitPhoneNumber,
        WaitPremiumPurchase,
        WaitRegistration,
        WaitTdlibParameters
    };
    Q_ENUM(AuthorizationState)


    explicit TDLibState(TDLibReceiver *tdLibReceiver, QObject *parent = nullptr);

    bool isDiceEmoji(const QString &text);

signals:
    void reactionsUpdated();
    void authorizationStateChanged();

    void initialParametersRequested();

private slots:
    void handleAuthorizationStateChanged(const QString &authorizationState, const QVariantMap authorizationStateData);
    void handleActiveEmojiReactionsUpdated(const QStringList& emojis);
    void handleDiceEmojisUpdated(const QStringList &emojis);

public: // FIXME: should be private ideally
    TDLibState::AuthorizationState authorizationState;
    QVariantMap authorizationStateData;
    QStringList activeEmojiReactions;
    QStringList diceEmojis;
};

#endif // TDLIBSTATE_H
