import QtQuick
import OpenHDGroundUI

Item {
    id: root

    Colors {
        id: palette
    }

    Metrics {
        id: metrics
    }

    VideoCanvas {
        anchors.fill: parent
        healthy: videoModel.videoAvailable
        recording: videoModel.recording
        decoderState: videoModel.decoderState
        streamWidth: videoModel.streamWidth
        streamHeight: videoModel.streamHeight
        streamFps: videoModel.streamFps
        codec: videoModel.codec
    }

    TopStatusBar {
        id: topBar

        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.margins: metrics.gap
    }

    WarningBanner {
        id: warningBanner

        alertData: alertModel.count > 0 ? alertModel.get(0) : null
        anchors.top: topBar.bottom
        anchors.topMargin: 12
        anchors.horizontalCenter: parent.horizontalCenter
    }

    LeftHudPanel {
        id: leftHud

        anchors.left: parent.left
        anchors.leftMargin: metrics.gap
        anchors.top: warningBanner.bottom
        anchors.topMargin: 18
        anchors.bottom: bottomPanel.top
        anchors.bottomMargin: metrics.gap
        width: metrics.hudWidth
    }

    RightActionPanel {
        id: rightPanel

        anchors.top: warningBanner.bottom
        anchors.topMargin: 18
        anchors.right: parent.right
        anchors.rightMargin: metrics.gap
        anchors.bottom: bottomPanel.top
        anchors.bottomMargin: metrics.gap
        width: metrics.actionWidth
    }

    MiniMapPanel {
        id: miniMap

        anchors.right: rightPanel.left
        anchors.rightMargin: metrics.gap
        anchors.bottom: bottomPanel.top
        anchors.bottomMargin: metrics.gap
        width: metrics.mapSize
        height: metrics.mapSize
    }

    BottomInfoPanel {
        id: bottomPanel

        anchors.left: parent.left
        anchors.leftMargin: metrics.gap
        anchors.right: miniMap.left
        anchors.rightMargin: metrics.gap
        anchors.bottom: parent.bottom
        anchors.bottomMargin: metrics.gap
        height: metrics.bottomPanelHeight
    }

    Rectangle {
        anchors.left: leftHud.right
        anchors.leftMargin: metrics.gap
        anchors.right: miniMap.left
        anchors.rightMargin: metrics.gap
        anchors.top: warningBanner.bottom
        anchors.topMargin: 24
        anchors.bottom: bottomPanel.top
        anchors.bottomMargin: metrics.gap
        radius: metrics.radiusLarge
        color: "transparent"
        border.width: 1
        border.color: "#14ffffff"
    }

    Text {
        anchors.left: leftHud.right
        anchors.leftMargin: metrics.gap + 20
        anchors.bottom: bottomPanel.top
        anchors.bottomMargin: metrics.gap + 18
        text: vehicleModel.armed ? "AIRFRAME ARMED" : "SAFE IDLE"
        color: vehicleModel.armed ? palette.critical : palette.healthy
        font.family: "JetBrains Mono"
        font.pixelSize: 13
        font.weight: Font.Bold
        font.letterSpacing: 1.4
    }
}
