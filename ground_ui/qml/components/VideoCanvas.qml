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
        gradient: Gradient {
            GradientStop {
                position: 0.0
                color: "#08161f"
            }
            GradientStop {
                position: 0.5
                color: root.healthy ? "#0c2f39" : "#2b1a1a"
            }
            GradientStop {
                position: 1.0
                color: "#050b10"
            }
        }
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

    Repeater {
        model: 16

        Rectangle {
            width: root.width
            height: 1
            y: index * root.height / 16
            color: "#10ffffff"
        }
    }

    Repeater {
        model: 10

        Rectangle {
            width: 1
            height: root.height
            x: index * root.width / 10
            color: "#08ffffff"
        }
    }

    Rectangle {
        id: sweep

        width: root.width * 0.32
        height: root.height
        rotation: 5
        y: 0
        color: "transparent"
        gradient: Gradient {
            GradientStop {
                position: 0.0
                color: "#00ffffff"
            }
            GradientStop {
                position: 0.5
                color: "#18d5f7ff"
            }
            GradientStop {
                position: 1.0
                color: "#00ffffff"
            }
        }

        SequentialAnimation on x {
            loops: Animation.Infinite

            NumberAnimation {
                from: -sweep.width
                to: root.width + sweep.width
                duration: 5200
                easing.type: Easing.InOutSine
            }
        }
    }

    Rectangle {
        width: 220
        height: 56
        radius: 18
        anchors.left: parent.left
        anchors.leftMargin: 26
        anchors.top: parent.top
        anchors.topMargin: 24
        color: "#9d091015"
        border.width: 1
        border.color: "#23577788"
    }

    Column {
        anchors.left: parent.left
        anchors.leftMargin: 42
        anchors.top: parent.top
        anchors.topMargin: 34
        spacing: 4

        Text {
            text: root.healthy ? "OPENHD FPV LIVE" : "VIDEO STANDBY"
            color: palette.textPrimary
            font.family: type.displayFamily
            font.pixelSize: 18
            font.weight: Font.Bold
            font.letterSpacing: 0.8
        }

        Text {
            text: root.healthy
                  ? (demoMode ? "Synthetic video state for UI development" : "Live RTP decoded from UDP 5800")
                  : (demoMode ? "Waiting for demo transport recovery" : "Waiting for live RTP packets from OpenHD Ground")
            color: palette.textMuted
            font.family: type.bodyFamily
            font.pixelSize: 12
        }
    }

    Row {
        anchors.right: parent.right
        anchors.rightMargin: 30
        anchors.top: parent.top
        anchors.topMargin: 26
        spacing: 10

        Rectangle {
            visible: root.recording
            width: 74
            height: 34
            radius: 17
            color: "#b8261a1a"
            border.width: 1
            border.color: "#ff716f"

            Text {
                anchors.centerIn: parent
                text: "REC"
                color: "#fff1f1"
                font.family: type.monoFamily
                font.pixelSize: 13
                font.weight: Font.Bold
                font.letterSpacing: 1.2
            }
        }

        Rectangle {
            width: 170
            height: 34
            radius: 17
            color: "#9d091015"
            border.width: 1
            border.color: "#23577788"

            Text {
                anchors.centerIn: parent
                text: root.streamWidth > 0
                      ? root.streamWidth + " x " + root.streamHeight + " @ " + root.streamFps + " fps"
                      : root.codec + "  |  " + root.decoderState
                color: palette.textMuted
                font.family: type.monoFamily
                font.pixelSize: 12
            }
        }
    }

    Rectangle {
        width: 112
        height: 112
        radius: 56
        anchors.centerIn: parent
        color: "transparent"
        border.width: 1
        border.color: "#2ae8f1ff"
        opacity: 0.28
    }

    Rectangle {
        width: 24
        height: 2
        anchors.centerIn: parent
        color: "#55ffffff"
    }

    Rectangle {
        width: 2
        height: 24
        anchors.centerIn: parent
        color: "#55ffffff"
    }
}
