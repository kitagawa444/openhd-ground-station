import QtQuick
import QtQuick.Controls
import OpenHDGroundUI

Item {
    id: root

    property var messages: []

    Colors {
        id: palette
    }

    Typography {
        id: type
    }

    Rectangle {
        anchors.fill: parent
        color: "#071219"
    }

    Column {
        id: heading

        anchors.left: parent.left
        anchors.leftMargin: 30
        anchors.top: parent.top
        anchors.topMargin: 24
        spacing: 4

        Text {
            text: "MAVLINK MESSAGES"
            color: palette.textPrimary
            font.family: type.displayFamily
            font.pixelSize: 30
            font.weight: Font.Bold
            font.letterSpacing: 1.4
        }

        Text {
            text: "STATUSTEXT messages from the flight controller, OpenHD Air, and OpenHD Ground"
            color: palette.textMuted
            font.family: type.bodyFamily
            font.pixelSize: 14
        }
    }

    ListView {
        anchors.top: heading.bottom
        anchors.topMargin: 22
        anchors.left: parent.left
        anchors.leftMargin: 30
        anchors.right: parent.right
        anchors.rightMargin: 30
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 16
        clip: true
        spacing: 8
        model: root.messages

        delegate: Rectangle {
            required property var modelData

            width: ListView.view.width
            height: 62
            radius: 12
            color: modelData.severity === "critical" ? "#2b191b" : "#b8122029"
            border.width: 1
            border.color: modelData.severity === "critical" ? palette.critical : palette.panelStroke

            Row {
                anchors.fill: parent
                anchors.leftMargin: 16
                anchors.rightMargin: 16
                spacing: 14

                Text {
                    width: 64
                    anchors.verticalCenter: parent.verticalCenter
                    text: modelData.timestamp
                    color: palette.textDim
                    font.family: type.monoFamily
                    font.pixelSize: 12
                }

                Text {
                    width: 90
                    anchors.verticalCenter: parent.verticalCenter
                    text: modelData.source
                    color: palette.accent
                    font.family: type.monoFamily
                    font.pixelSize: 12
                    font.weight: Font.Bold
                    elide: Text.ElideRight
                }

                Text {
                    anchors.verticalCenter: parent.verticalCenter
                    width: parent.width - 200
                    text: modelData.text
                    color: palette.textPrimary
                    font.family: type.bodyFamily
                    font.pixelSize: 15
                    elide: Text.ElideRight
                }
            }
        }
    }

    Text {
        anchors.centerIn: parent
        visible: root.messages.length === 0
        text: "NO MAVLINK STATUS TEXT RECEIVED"
        color: palette.textDim
        font.family: type.monoFamily
        font.pixelSize: 16
        font.letterSpacing: 1.2
    }
}
