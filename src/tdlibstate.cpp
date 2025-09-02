#include "tdlibstate.h"

#define DEBUG_MODULE TDLibState
#include "debuglog.h"

TDLibState::TDLibState(TDLibReceiver *tdLibReceiver, QObject *parent) :
    QObject(parent),
    authorizationState(AuthorizationState::Closed),
    diceEmojis()
{
    connect(tdLibReceiver, &TDLibReceiver::authorizationStateChanged, this, &TDLibState::handleAuthorizationStateChanged);
    connect(tdLibReceiver, &TDLibReceiver::activeEmojiReactionsUpdated, this, &TDLibState::handleActiveEmojiReactionsUpdated);
    connect(tdLibReceiver, &TDLibReceiver::diceEmojisUpdated, this, &TDLibState::handleDiceEmojisUpdated);
}

void TDLibState::handleAuthorizationStateChanged(const QString &authorizationState, const QVariantMap authorizationStateData) {
    if (authorizationState == "authorizationStateReady")
        this->authorizationState = AuthorizationState::AuthorizationReady;
    else if (authorizationState == "authorizationStateWaitTdlibParameters") {
        this->initialParametersRequested();
        this->authorizationState = AuthorizationState::WaitTdlibParameters;
    } else if (authorizationState == "authorizationStateClosed")
        this->authorizationState = AuthorizationState::Closed;
    else if (authorizationState == "authorizationStateClosing")
        this->authorizationState = AuthorizationState::Closing;
    else if (authorizationState == "authorizationStateLoggingOut")
        this->authorizationState = AuthorizationState::LoggingOut;
    else if (authorizationState == "authorizationStateWaitCode")
        this->authorizationState = AuthorizationState::WaitCode;
    else if (authorizationState == "authorizationStateWaitEmailAddress")
        this->authorizationState = AuthorizationState::WaitEmailAddress;
    else if (authorizationState == "authorizationStateWaitEmailCode")
        this->authorizationState = AuthorizationState::WaitEmailCode;
    else if (authorizationState == "authorizationStateWaitOtherDeviceConfirmation")
        this->authorizationState = AuthorizationState::WaitOtherDeviceConfirmation;
    else if (authorizationState == "authorizationStateWaitPassword")
        this->authorizationState = AuthorizationState::WaitPassword;
    else if (authorizationState == "authorizationStateWaitPhoneNumber")
        this->authorizationState = AuthorizationState::WaitPhoneNumber;
    else if (authorizationState == "authorizationStateWaitPremiumPurchase")
        this->authorizationState = AuthorizationState::WaitPremiumPurchase;
    else if (authorizationState == "authorizationStateWaitRegistration")
        this->authorizationState = AuthorizationState::WaitRegistration;

    this->authorizationStateData = authorizationStateData;
    emit authorizationStateChanged();
}

void TDLibState::handleActiveEmojiReactionsUpdated(const QStringList& emojis) {
    if (activeEmojiReactions != emojis) {
        activeEmojiReactions = emojis;
        LOG(emojis.count() << "reaction(s) available");
        emit reactionsUpdated();
    }
}

void TDLibState::handleDiceEmojisUpdated(const QStringList &emojis) {
    if (diceEmojis != emojis) {
        LOG("Dice emojis updated" << emojis);
        diceEmojis = emojis;
    }
}

bool TDLibState::isDiceEmoji(const QString &text) {
    LOG("Checking if text is a dice emoji" << text);
    return diceEmojis.contains(QString(text).trimmed());
}
