import QtQuick
import OpenHDGroundUI

Item {
    id: root

    implicitWidth: metrics.actionWidth

    Colors {
        id: palette
    }

    Metrics {
        id: metrics
    }

    Column {
        anchors.fill: parent
        spacing: 14
        visible: demoMode

        ActionButton {
            headline: vehicleModel.armed ? "Disarm" : "Arm"
            detail: vehicleModel.armed ? "Return to safe idle state" : "Enable flight state in demo"
            tone: vehicleModel.armed ? palette.critical : palette.caution
            active: vehicleModel.armed
            onClicked: vehicleModel.armed ? flightCommands.disarm() : flightCommands.arm()
        }

        ActionButton {
            headline: "Mode"
            detail: vehicleModel.flightMode
            tone: palette.accent
            onClicked: flightCommands.cycleFlightMode()
        }

        ActionButton {
            headline: videoModel.recording ? "Stop Record" : "Record"
            detail: videoModel.recording ? "Finish current clip" : "Start demo capture flag"
            tone: videoModel.recording ? palette.critical : palette.healthy
            active: videoModel.recording
            onClicked: cameraCommands.toggleRecording()
        }

        ActionButton {
            headline: "Reconnect"
            detail: "Simulate link and decoder recovery"
            tone: palette.caution
            onClicked: systemCommands.reconnectLink()
        }

        ActionButton {
            headline: "Restart Video"
            detail: "Exercise decoder restart state"
            tone: palette.accent
            onClicked: cameraCommands.restartVideo()
        }
    }

    Rectangle {
        anchors.fill: parent
        visible: !demoMode
        radius: metrics.radiusLarge
        color: palette.panelGlass
        border.width: 1
        border.color: palette.panelStroke
    }

    Column {
        anchors.fill: parent
        anchors.margins: 18
        spacing: 10
        visible: !demoMode

        Text {
            text: "LIVE MODE"
            color: palette.accent
            font.family: "JetBrains Mono"
            font.pixelSize: 14
            font.weight: Font.Bold
            font.letterSpacing: 1.2
        }

        Text {
            text: "OpenHD telemetry and RTP state are read-only in this build."
            color: palette.textPrimary
            font.pixelSize: 16
            font.weight: Font.DemiBold
            wrapMode: Text.Wrap
        }

        Text {
            text: "Flight, RC, recording, and backend restart commands will be added only after their real MAVLink/OpenHD paths are verified."
            color: palette.textMuted
            font.pixelSize: 13
            wrapMode: Text.Wrap
        }
    }
}
