import QtQuick
import OpenHDGroundUI

Item {
    id: root

    implicitWidth: metrics.mapSize
    implicitHeight: metrics.mapSize

    Colors {
        id: palette
    }

    Metrics {
        id: metrics
    }

    Typography {
        id: type
    }

    readonly property double northMeters: vehicleModel.positionValid && vehicleModel.homePositionValid
                                         ? (vehicleModel.latitude - vehicleModel.homeLatitude) * 111320
                                         : 0
    readonly property double eastMeters: vehicleModel.positionValid && vehicleModel.homePositionValid
                                        ? (vehicleModel.longitude - vehicleModel.homeLongitude)
                                          * 111320 * Math.cos(vehicleModel.homeLatitude * Math.PI / 180)
                                        : 0

    Rectangle {
        anchors.fill: parent
        radius: metrics.radiusLarge
        color: "#bf0d171e"
        border.width: 1
        border.color: palette.panelStroke
    }

    Repeater {
        model: 6

        Rectangle {
            width: parent ? parent.width - 34 : 0
            height: 1
            x: 17
            y: 30 + index * 34
            color: "#123f5664"
        }
    }

    Repeater {
        model: 6

        Rectangle {
            width: 1
            height: parent ? parent.height - 34 : 0
            x: 30 + index * 34
            y: 17
            color: "#123f5664"
        }
    }

    Text {
        anchors.left: parent.left
        anchors.leftMargin: 18
        anchors.top: parent.top
        anchors.topMargin: 16
        text: "MINI MAP"
        color: palette.textDim
        font.family: type.bodyFamily
        font.pixelSize: type.captionSize
        font.weight: Font.DemiBold
        font.letterSpacing: 1.2
    }

    Rectangle {
        id: homeMarker

        width: 12
        height: 12
        radius: 6
        anchors.centerIn: parent
        color: palette.caution
        border.width: 2
        border.color: "#fff0c6"
        visible: vehicleModel.homePositionValid
    }

    Rectangle {
        id: vehicleMarker

        width: 16
        height: 16
        radius: 8
        x: Math.max(16, Math.min(root.width - width - 16, root.width * 0.5 + root.eastMeters * 0.7 - width / 2))
        y: Math.max(16, Math.min(root.height - height - 16, root.height * 0.5 - root.northMeters * 0.7 - height / 2))
        color: palette.accent
        border.width: 2
        border.color: "#d9f7ff"
        rotation: vehicleModel.headingDegrees
        visible: vehicleModel.positionValid

        Rectangle {
            width: 2
            height: 16
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.bottom: parent.top
            color: "#bfeeffff"
        }
    }

    Text {
        anchors.left: parent.left
        anchors.leftMargin: 18
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 18
        text: vehicleModel.homePositionValid ? "HOME " + vehicleModel.homeDistanceMeters.toFixed(0) + " m" : "HOME N/A"
        color: palette.textMuted
        font.family: type.monoFamily
        font.pixelSize: 12
    }
}
