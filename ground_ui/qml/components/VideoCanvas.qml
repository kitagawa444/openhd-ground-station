import QtQuick
import OpenHDGroundUI

Item {
    id: root

    property bool healthy: true
    property bool recording: false
    property string decoderState: ""
    property int streamWidth: 0
    property int streamHeight: 0
    property int streamFps: 0
    property string codec: "Unknown"

    clip: true

    Colors {
        id: palette
    }

    Typography {
        id: type
    }

    Rectangle {
        anchors.fill: parent
        color: "#05080b"
    }

    Image {
        anchors.fill: parent
        visible: !demoMode && videoModel.frameAvailable
        source: "image://openhd/frame?sequence=" + videoModel.frameSequence
        fillMode: Image.PreserveAspectCrop
        cache: false
        asynchronous: false
        smooth: true
    }

    Rectangle {
        anchors.fill: parent
        visible: !videoModel.frameAvailable
        gradient: Gradient {
            GradientStop { position: 0.0; color: "#0b151d" }
            GradientStop { position: 0.55; color: root.healthy ? "#102b32" : "#30191b" }
            GradientStop { position: 1.0; color: "#05080b" }
        }
    }

    Column {
        anchors.centerIn: parent
        visible: !videoModel.frameAvailable
        spacing: 8

        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            text: demoMode ? "SIMULATED VIDEO FEED" : "WAITING FOR VIDEO"
            color: palette.textPrimary
            font.family: type.monoFamily
            font.pixelSize: 18
            font.weight: Font.Bold
            font.letterSpacing: 1.8
        }

        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            text: demoMode ? "OpenHD OSD layout preview" : root.decoderState
            color: palette.textMuted
            font.family: type.bodyFamily
            font.pixelSize: 13
        }
    }
}
