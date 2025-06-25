/*
    Copyright (C) 2020 Sebastian J. Wolf and other contributors

    This file is part of Fernschreiber.

    Fernschreiber is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Fernschreiber is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Fernschreiber. If not, see <http://www.gnu.org/licenses/>.
*/
import QtQuick 2.6
import Sailfish.Silica 1.0
import "../components"
import "../js/twemoji.js" as Emoji
import "../js/functions.js" as Functions
import "../js/debug.js" as Debug

Page {
    id: newChatPage
    allowedOrientations: Orientation.All

    Component.onDestruction: contactsProxyModel.setFilterWildcard('*')

    Connections {
        target: tdLibWrapper
        onContactsImported: {
            busyLabel.running = false
            appNotification.show(qsTr("Contacts successfully synchronized with Telegram."))
        }
    }

    ContactSync {
        id: contactSync
        onSyncError: busyLabel.running = false
    }

    SilicaFlickable {
        id: newChatContainer
        contentHeight: newChatPage.height
        anchors.fill: parent

        PullDownMenu {
            MenuItem {
                onClicked: {
                    busyLabel.running = true
                    contactSync.synchronize()
                    // Success message is not fired before TDLib returned "Contacts imported" (see above)
                }
                text: qsTr("Synchronize Contacts with Telegram")
            }
        }

        BusyLabel {
            id: busyLabel
            anchors.verticalCenter: parent.verticalCenter
            text: qsTr("Loading contacts...")
        }

        PageHeader { id: header; title: qsTr("Your Contacts") }

        SearchField {
            id: search
            width: parent.width
            anchors.top: header.bottom
            placeholderText: qsTr("Search a contact...")
            visible: !busyLabel.running
            active: visible
            opacity: visible ? 1 : 0
            Behavior on opacity { FadeAnimator {} }

            onTextChanged: contactsProxyModel.setFilterWildcard("*" + text + "*")

            EnterKey.iconSource: "image://theme/icon-m-enter-close"
            EnterKey.onClicked: {
                search.focus = false
                newChatPage.focus = true
            }

        }

        SilicaListView {
            id: listView
            model: contactsProxyModel
            clip: true
            width: parent.width
            anchors {
                top: search.bottom
                bottom: parent.bottom
            }
            visible: !busyLabel.running
            opacity: visible ? 1 : 0
            Behavior on opacity { FadeAnimator {} }

            signal newChatInitiated (int currentIndex)

            ViewPlaceholder {
                y: Theme.paddingLarge
                enabled: listView.count === 0
                text: (search.text.length > 0)
                    ? qsTr("No contacts found.")
                    : qsTr("You don't have any contacts.")
            }

            delegate: Item {
                id: newChatListItem
                width: parent.width
                height: contactListItem.height

                PhotoTextsListItem {
                    id: contactListItem

                    opacity: visible ? 1 : 0
                    Behavior on opacity { FadeAnimation {} }

                    pictureThumbnail.photoData: typeof photo_small !== "undefined" ? photo_small : {}
                    width: parent.width

                    primaryText.text: Emoji.emojify(title, primaryText.font.pixelSize, "../js/emoji/")
                    prologSecondaryText.text: "@" + ( username !== "" ? username : user_id )
                    tertiaryText {
                        maximumLineCount: 1
                        text: Functions.getChatPartnerStatusText(user_status, user_last_online);
                    }

                    onClicked: listView.newChatInitiated(index)

                    Connections {
                        target: listView

                        onNewChatInitiated: {
                            if (index === currentIndex) {
                                contactListItem.visible = false;
                            } else {
                                contactListItem.visible = true;
                            }
                        }
                    }

                    Connections {
                        target: search
                        onFocusChanged: {
                            if (search.focus) {
                                contactListItem.visible = true;
                            }
                        }
                    }
                }

                Column {
                    id: selectChatTypeColumn
                    visible: !contactListItem.visible
                    opacity: visible ? 1 : 0
                    Behavior on opacity { FadeAnimation {} }
                    width: parent.width
                    height: contactListItem.height

                    Item {
                        width: parent.width
                        height: parent.height - chatTypeSeparator.height

                        Rectangle {
                            anchors.fill: parent
                            opacity: Theme.opacityLow
                            color: Theme.overlayBackgroundColor
                        }

                        BackgroundItem {
                            id: privateChatItem
                            height: parent.height
                            width: parent.width / 2

                            Row {
                                x: Theme.horizontalPageMargin
                                width: parent.width - x
                                y: Theme.paddingSmall
                                height: parent.height - 2*y
                                anchors.verticalCenter: parent.verticalCenter

                                Item {
                                    id: privateChatButton
                                    width: Theme.itemSizeLarge
                                    height: Theme.itemSizeLarge
                                    anchors.verticalCenter: parent.verticalCenter
                                    Icon {
                                        anchors.centerIn: parent
                                        source: "image://theme/icon-m-chat"
                                    }
                                }

                                Column {
                                    height: parent.height
                                    width: parent.width - privateChatButton.width - Theme.horizontalPageMargin
                                    anchors.verticalCenter: parent.verticalCenter
                                    spacing: Theme.paddingSmall
                                    Text {
                                        id: privateChatHeader
                                        width: parent.width
                                        font.pixelSize: Theme.fontSizeMedium
                                        font.weight: Font.ExtraBold
                                        color: Theme.primaryColor
                                        maximumLineCount: 1
                                        elide: Text.ElideRight
                                        textFormat: Text.StyledText
                                        text: qsTr("Private Chat")
                                    }
                                    Text {
                                        width: parent.width
                                        height: parent.height - privateChatHeader.height - Theme.paddingSmall
                                        font.pixelSize: Theme.fontSizeTiny
                                        color: Theme.secondaryColor
                                        wrapMode: Text.Wrap
                                        elide: Text.ElideRight
                                        textFormat: Text.StyledText
                                        text: qsTr("Transport-encrypted, uses Telegram Cloud, sharable across devices")
                                    }
                                }

                            }

                            onClicked: tdLibWrapper.createPrivateChat(display.id, "openDirectly")
                        }

                        BackgroundItem {
                            height: parent.height
                            width: parent.width / 2
                            anchors.left: privateChatItem.right
                            anchors.top: parent.top


                            Row {
                                x: Theme.horizontalPageMargin
                                width: parent.width - x
                                y: Theme.paddingSmall
                                height: parent.height - 2*y
                                anchors.verticalCenter: parent.verticalCenter

                                Item {
                                    id: secretChatButton
                                    width: Theme.itemSizeLarge
                                    height: Theme.itemSizeLarge
                                    anchors.verticalCenter: parent.verticalCenter
                                    Icon {
                                        anchors.centerIn: parent
                                        source: "image://theme/icon-m-chat"
                                    }
                                }

                                Column {
                                    height: parent.height
                                    width: parent.width - secretChatButton.width - Theme.horizontalPageMargin
                                    anchors.verticalCenter: parent.verticalCenter
                                    spacing: Theme.paddingSmall
                                    Text {
                                        width: parent.width
                                        font.pixelSize: Theme.fontSizeMedium
                                        font.weight: Font.ExtraBold
                                        color: Theme.primaryColor
                                        maximumLineCount: 1
                                        elide: Text.ElideRight
                                        textFormat: Text.StyledText
                                        text: qsTr("Secret Chat")
                                    }
                                    Text {
                                        width: parent.width
                                        height: parent.height - privateChatHeader.height - Theme.paddingSmall
                                        font.pixelSize: Theme.fontSizeTiny
                                        color: Theme.secondaryColor
                                        wrapMode: Text.Wrap
                                        elide: Text.ElideRight
                                        textFormat: Text.StyledText
                                        text: qsTr("End-to-end-encrypted, accessible on this device only")
                                    }
                                }

                            }

                            onClicked: tdLibWrapper.createNewSecretChat(display.id, "openDirectly")
                        }

                    }

                    Separator {
                        id: chatTypeSeparator
                        width: parent.width
                        color: Theme.primaryColor
                        horizontalAlignment: Qt.AlignHCenter
                    }

                }

            }

            VerticalScrollDecorator {}
        }
    }
}
