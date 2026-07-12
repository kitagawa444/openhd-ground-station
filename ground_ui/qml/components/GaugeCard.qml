import QtQuick
import OpenHDGroundUI

Item {
    id: root

    property string label: ""
    property string value: ""
    property string unit: ""
    property color tone: palette.accent

    implicitWidth: 226
    implicitHeight: 110

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
        radius: metrics.radiusLarge
        color: palette.panelGlass
        border.width: 1
        border.color: Qt.rgba(root.tone.r, root.tone.g, root.tone.b, 0.26)
    }

    Rectangle {
        width: 58
        height: 58
        radius: 29
        anchors.left: parent.left
        anchors.leftMargin: 16
        anchors.verticalCenter: parent.verticalCenter
        color: Qt.rgba(root.tone.r, root.tone.g, root.tone.b, 0.16)
        border.width: 1
        border.color: Qt.rgba(root.tone.r, root.tone.g, root.tone.b, 0.32)
    }

    Text {
        anchors.left: parent.left
        anchors.leftMargin: 30
        anchors.verticalCenter: parent.verticalCenter
        text: root.label.length > 0 ? root.label.charAt(0) : ""
        color: root.tone
        font.family: type.displayFamily
        font.pixelSize: 24
        font.weight: Font.Bold
    }

    Column {
        anchors.left: parent.left
        anchors.leftMargin: 92
        anchors.right: parent.right
        anchors.rightMargin: 18
        anchors.verticalCenter: parent.verticalCenter
        spacing: 2

        Text {
            text: root.label
            color: palette.textDim
            font.family: type.bodyFamily
            font.pixelSize: type.captionSize
            font.weight: Font.DemiBold
            font.letterSpacing: 1.2
        }

        Row {
            id: valueRow

            spacing: 6

            Text {
                id: valueText

                text: root.value
                color: palette.textPrimary
                font.family: type.displayFamily
                font.pixelSize: type.dataSize
                font.weight: Font.Bold
            }

            Text {
                anchors.baseline: valueText.baseline
                text: root.unit
                color: palette.textMuted
                font.family: type.monoFamily
                font.pixelSize: type.bodySize
            }
        }
    }
}
