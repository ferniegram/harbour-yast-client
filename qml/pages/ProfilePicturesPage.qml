import QtQuick 2.0
import Sailfish.Silica 1.0
import App.Logic 1.0
import '../components'
import '../components/messageContent/mediaAlbumPage'

MediaAlbumPage {
    property alias userId: profilePicturesModel.userId
    property bool isMyself: userId === tdLibWrapper.myUserId

    model: UserProfilePicturesModel {
        id: profilePicturesModel
        tdlib: tdLibWrapper
    }
    modelIsMedia: false

    pagedView.direction: PagedView.LeftToRight
    delegate: PhotoComponent {
        width: PagedView.contentWidth
        height: PagedView.contentHeight

        property string photoId: photo_id

        photoData: null
        photoSize: big_photo
    }

    onIndexChanged:
        if (index >= count - 1 - 10)
            profilePicturesModel.loadMore()

    // TODO: fix downloading pictures
    // also deleting own picture seems to be broken (deletes, but the model isn't affected)
    // and ideally the viewer should open the current photo if it's not the first one when possible

    // TODO: make photos preview a ListView so only currently needed items would be loaded (see also ChatPhotosPage)
    overlay.previewModel: profilePicturesModel
    overlay.previewComponent: Component {
        TDLibPhoto {
            readonly property bool current: pagedView.currentItem.photoId == photo_id

            height: parent.height
            width: current ? height : (height / 2)
            Behavior on width { NumberAnimation { duration: 150 } }

            fileInformation: small_photo.photo
            minithumbnail: photo_minithumbnail
            highlighted: singlePreviewMouseArea.containsPress

            MouseArea {
                id: singlePreviewMouseArea
                anchors.fill: parent
                onClicked: overlay.jumpedToIndex(index)
            }
        }
    }

    overlay.forwardButtonVisible: false
    overlay.deleteButtonVisible: isMyself
    overlay.applyButtonVisible: isMyself

    overlay.onDeleted:
        tdLibWrapper.deleteProfilePhoto(pagedView.currentItem.photoId)
    overlay.onApplied:
        tdLibWrapper.setPreviousProfilePhoto(pagedView.currentItem.photoId)
}
