#ifndef CHATMODEL_H
#define CHATMODEL_H

#include "messagesmodel.h"

class ChatModel : public MessagesModel {
    Q_OBJECT
public:
    ChatModel(TDLibWrapper *tdLibWrapper);
};

#endif // CHATMODEL_H
