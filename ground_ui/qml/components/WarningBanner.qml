import QtQuick
import OpenHDGroundUI

Item {
    id: root

    property var alertData: null

    visible: alertData !== null && alertData.title !== undefined
    implicitWidth: 540
    implicitHeight: visible ? 82 : 0
    opacity: visible ? 1.0 : 0.0

    Colors {
        id: palette
    }

    Metrics {
        id: metrics
    }

    Typography {
        id: type
    }

    function severityColor() {
        if (!alertData || alertData.severity === undefined)
            return palette.accent
        if (alertData.severity === "critical")
            return palette.critical
        if (alertData.severity === "caution")
            return palette.caution
        return palette.accent
    }

    Behavior on opacity {
        NumberAnimation {
            duration: 180
        }
    }

    Rectangle {
        anchors.fill: parent
        radius: metrics.radiusLarge
        color: Qt.rgba(root.severityColor().r, root.severityColor().g, root.severityColor().b, 0.14)
        border.width: 1
        border.color: Qt.rgba(root.severityColor().r, root.severityColor().g, root.severityColor().b, 0.45)
    }

    Rectangle {
        width: 6
        radius: 3
        anchors.left: parent.left
        anchors.leftMargin: 14
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.topMargin: 12
        anchors.bottomMargin: 12
        color: root.severityColor()
    }

    Column {
        anchors.fill: parent
        anchors.leftMargin: 32
        anchors.rightMargin: 18
        anchors.topMargin: 12
        anchors.bottomMargin: 12
        spacing: 4

        Row {
            spacing: 10

            Text {
                text: alertData ? alertData.title : ""
                color: palette.textPrimary
                font.family: type.displayFamily
                font.pixelSize: type.sectionSize
                font.weight: Font.Bold
            }

            Text {
                text: alertData ? alertData.timestamp : ""
                color: palette.textDim
                font.family: type.monoFamily
                font.pixelSize: type.captionSize + 1
            }
        }

        Text {
            text: alertData ? alertData.message : ""
            color: palette.textMuted
            font.family: type.bodyFamily
            font.pixelSize: type.bodySize
            wrapMode: Text.Wrap
            maximumLineCount: 2
        }
    }
}
