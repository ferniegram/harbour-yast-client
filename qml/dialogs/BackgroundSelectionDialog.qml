import QtQuick 2.0
import Sailfish.Silica 1.0
import Sailfish.Pickers 1.0
import '../components'

Dialog {
    id: dialog
    property var backgrounds: []
    property var selected

    Connections {
        target: tdLibWrapper
        onBackgroundsReceived: dialog.backgrounds = backgrounds
    }

    Component.onCompleted: tdLibWrapper.getInstalledBackgrounds()

    DialogHeader { id: header }

    SilicaFlickable {
        width: parent.width
        anchors {
            top: header.bottom
            bottom: parent.bottom
        }
        contentHeight: flow.height
        clip: !atYBeginning
        Flow {
            id: flow
            width: parent.width
            property int columnCount: Math.floor(width / (Theme.pixelRatio * 240))
            property real cellHeight: width / columnCount
            property real cellWidth: cellHeight

            Repeater {
                model: dialog.backgrounds
                delegate: BackgroundItem {
                    id: gridItem
                    width: flow.cellWidth
                    height: flow.cellHeight

                    TDLibPhoto {
                        anchors.fill: parent
                        highlighted: gridItem.highlighted
                        minithumbnail: modelData.document && modelData.document.minithumbnail ? modelData.document.minithumbnail : null
                        image.fileInformation: modelData.document && modelData.document.document ? modelData.document.document : {}
                    }
                }
            }
        }
    }
}
