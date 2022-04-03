import QtQuick 2.0
import QtQuick.Controls 1.0
import org.freedesktop.gstreamer.GLVideoItem 1.0

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
}
