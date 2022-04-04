import QtQuick 2.0
import QtQuick.Controls 1.0
import QtQuick.Layouts 1.15
import org.freedesktop.gstreamer.GLVideoItem 1.0
import examples.webrtc 1.0 

ApplicationWindow {
    visible: true
    width: 640
    height: 480
    title: qsTr("Minimal QML")
    objectName: "test1"

    Rectangle {
        color: "white"
        anchors.fill: parent

        GstGLVideoItem {
            objectName: "videoCallItem"
            anchors.fill: parent
        }
    }
    RowLayout {
        anchors.bottom: parent.bottom
        Button {
            text: qsTr("Hangup")
            onClicked: WebRTCHandler.hangup()
        }
        Button {
            text: qsTr("Quit")
            onClicked: Qt.quit()
        }
    }
}
