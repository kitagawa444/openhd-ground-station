import QtQuick
import QtQuick.Controls
import OpenHDGroundUI

Button {
    id: root

    property string headline: text
    property string detail: ""
    property color tone: palette.accent
    property bool active: false

    implicitWidth: 188
    implicitHeight: 78
    text: ""
    hoverEnabled: true

    Colors {
        id: palette
    }

    Metrics {
        id: metrics
    }

    Typography {
        id: type
    }

    background: Rectangle {
        radius: metrics.radiusMedium
        color: root.down
               ? Qt.rgba(root.tone.r, root.tone.g, root.tone.b, 0.26)
               : root.hovered
                 ? Qt.rgba(root.tone.r, root.tone.g, root.tone.b, 0.18)
                 : root.active
                   ? Qt.rgba(root.tone.r, root.tone.g, root.tone.b, 0.16)
                   : palette.panelGlass
        border.width: 1
        border.color: Qt.rgba(root.tone.r, root.tone.g, root.tone.b, root.active ? 0.65 : 0.32)
    }

    contentItem: Column {
        anchors.fill: parent
        anchors.leftMargin: 16
        anchors.rightMargin: 16
        anchors.topMargin: 12
        anchors.bottomMargin: 12
        spacing: 4

        Text {
            text: root.headline
            color: palette.textPrimary
            font.family: type.displayFamily
            font.pixelSize: type.sectionSize
            font.weight: Font.DemiBold
        }

        Text {
            text: root.detail
            color: palette.textMuted
            font.family: type.bodyFamily
            font.pixelSize: type.captionSize + 1
            wrapMode: Text.Wrap
            maximumLineCount: 2
        }
    }
}
