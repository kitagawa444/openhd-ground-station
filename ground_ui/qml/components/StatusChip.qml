import QtQuick
import OpenHDGroundUI

Item {
    id: root

    property string label: ""
    property string value: ""
    property color tone: palette.accent

    implicitWidth: 152
    implicitHeight: 52

    Colors {
        id: palette
    }

    Metrics {
        id: metrics
    }

    Typography {
        id: type
    }

    Rectangle {
        anchors.fill: parent
        radius: metrics.radiusMedium
        color: palette.panelSoft
        border.width: 1
        border.color: Qt.rgba(root.tone.r, root.tone.g, root.tone.b, 0.28)
    }

    Rectangle {
        width: 4
        height: parent.height - 18
        radius: 2
        anchors.left: parent.left
        anchors.leftMargin: 10
        anchors.verticalCenter: parent.verticalCenter
        color: root.tone
    }

    Column {
        anchors.fill: parent
        anchors.leftMargin: 24
        anchors.rightMargin: 14
        anchors.topMargin: 9
        anchors.bottomMargin: 9
        spacing: 2

        Text {
            text: root.label
            color: palette.textDim
            font.family: type.bodyFamily
            font.pixelSize: type.captionSize
            font.weight: Font.DemiBold
            font.letterSpacing: 1.1
        }

        Text {
            text: root.value
            color: palette.textPrimary
            font.family: type.displayFamily
            font.pixelSize: type.bodySize + 2
            font.weight: Font.DemiBold
            elide: Text.ElideRight
        }
    }
}
