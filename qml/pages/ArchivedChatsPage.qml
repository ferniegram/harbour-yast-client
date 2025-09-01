import QtQuick 2.0
import Sailfish.Silica 1.0
import '../components'

Page {
    allowedOrientations: Orientation.All

    property var overviewPage

    ChatsView {
        headerText: qsTr("Archive")
        model: archiveChatListModel
    }
}
