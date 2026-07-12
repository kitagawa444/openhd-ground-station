import QtQuick
import QtQuick.Layouts
import OpenHDGroundUI

Item {
    id: root

    implicitHeight: metrics.statusBarHeight

    Colors {
        id: palette
    }

    Metrics {
        id: metrics
    }

    Rectangle {
        anchors.fill: parent
        radius: metrics.radiusLarge
        color: palette.panelGlass
        border.width: 1
        border.color: palette.panelStroke
    }

    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: 16
        anchors.rightMargin: 16
        anchors.topMargin: 6
        anchors.bottomMargin: 6
        spacing: 12

        StatusChip {
            label: "LINK"
            value: linkModel.linkConnected
                   ? (linkModel.linkQualityPercent >= 0 ? linkModel.linkQualityPercent + "%" : "LIVE")
                   : (linkModel.backendConnected ? "WAITING" : "OFFLINE")
            tone: linkModel.linkConnected ? palette.healthy : linkModel.backendConnected ? palette.caution : palette.critical
        }

        StatusChip {
            label: "MODE"
            value: vehicleModel.flightMode
            tone: palette.accent
        }

        StatusChip {
            label: "BATTERY"
            value: vehicleModel.batteryValid
                   ? vehicleModel.batteryPercent + "%  |  " + vehicleModel.batteryVoltage.toFixed(1) + "V"
                   : "N/A"
            tone: vehicleModel.batteryValid && vehicleModel.batteryPercent < 35 ? palette.caution : palette.healthy
        }

        StatusChip {
            label: "GPS"
            value: vehicleModel.positionValid ? vehicleModel.satellites + " sats" : "WAITING"
            tone: vehicleModel.positionValid && vehicleModel.satellites < 12 ? palette.caution : palette.healthy
        }

        StatusChip {
            label: "LATENCY"
            value: linkModel.latencyMs >= 0 ? linkModel.latencyMs + " ms" : "N/A"
            tone: linkModel.latencyMs > 45 ? palette.caution : palette.accent
        }

        Item {
            Layout.fillWidth: true
        }

        StatusChip {
            label: "VIDEO"
            value: videoModel.videoAvailable ? videoModel.lastFrameAgeMs + " ms RTP age" : "NO FEED"
            tone: videoModel.videoAvailable ? palette.healthy : palette.caution
        }

        StatusChip {
            label: "REC"
            value: videoModel.recording ? "ACTIVE" : "IDLE"
            tone: videoModel.recording ? palette.critical : palette.textMuted
        }
    }
}
