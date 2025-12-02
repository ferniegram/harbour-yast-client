import QtQuick 2.0

QtObject {
    id: user

    property var userId
    property var userInformation: tdLibWrapper.getUserInformation(userId)

    onUserIdChanged:
        userInformation = tdLibWrapper.getUserInformation(userId)
    property Connections __tdLibWrapperConnections: Connections {
        target: tdLibWrapper
        onUserUpdated:
            if (user.userId == userId) // explicitly allow type correction here!
                user.userInformation = userInformation
    }
}
