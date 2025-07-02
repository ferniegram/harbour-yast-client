import QtQuick 2.0
import Sailfish.Silica 1.0

AlbumMessageContentBase {
    id: messageContent
    width: parent.width
    height: column.height
    Column {
        id: column
        width: parent.width
        Repeater {
            model: albumMessages
            MessageAudio {
                width: parent.width
                messageListItem: messageContent.messageListItem
                overlayFlickable: messageContent.overlayFlickable
                rawMessage: albumMessages[index]
                highlighted: messageContent.highlighted
            }
        }
    }
}
