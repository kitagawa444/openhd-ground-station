import QtQuick
import QtQuick.Controls
import OpenHDGroundUI

Item {
    id: root

    property string section: ""
    property string label: ""
    property string value: ""
    property string detail: ""

    implicitHeight: 112

    Colors {
        id: diagnosticColors
    }

    Typography {
        id: type
    }

    Rectangle {
        anchors.fill: parent
        radius: 14
        color: diagnosticColors.panelGlass
        border.width: 1
        border.color: diagnosticColors.panelStroke
    }

    Column {
        anchors.fill: parent
        anchors.margins: 13
        spacing: 3

        Text {
            width: parent.width
            text: root.section
            color: diagnosticColors.accent
            font.family: type.monoFamily
            font.pixelSize: 10
            font.weight: Font.Bold
            font.letterSpacing: 0.9
            elide: Text.ElideRight
        }

        Text {
            width: parent.width
            text: root.label
            color: diagnosticColors.textDim
            font.family: type.bodyFamily
            font.pixelSize: 12
            font.weight: Font.DemiBold
            elide: Text.ElideRight
        }

        Text {
            width: parent.width
            text: root.value
            color: diagnosticColors.textPrimary
            font.family: type.displayFamily
            font.pixelSize: 17
            font.weight: Font.Bold
            elide: Text.ElideRight
        }

        Text {
            width: parent.width
            visible: root.detail.length > 0
            text: root.detail
            color: diagnosticColors.textMuted
            font.family: type.monoFamily
            font.pixelSize: 10
            elide: Text.ElideRight
        }
    }

    MouseArea {
        anchors.fill: parent
        cursorShape: Qt.PointingHandCursor
        onClicked: detailPopup.open()
    }

    Popup {
        id: detailPopup

        parent: Overlay.overlay
        anchors.centerIn: parent
        width: Math.min(parent.width * 0.9, 980)
        height: Math.min(parent.height * 0.72, detailText.contentHeight + 100)
        padding: 0
        modal: true

        background: Rectangle {
            radius: 16
            color: "#f20b151e"
            border.width: 1
            border.color: diagnosticColors.accent
        }

        contentItem: Column {
            anchors.fill: parent
            anchors.margins: 22
            spacing: 10

            Text {
                text: root.section + " / " + root.label
                color: diagnosticColors.accent
                font.family: type.monoFamily
                font.pixelSize: 13
                font.weight: Font.Bold
            }

            Text {
                text: root.value
                color: diagnosticColors.textPrimary
                font.family: type.displayFamily
                font.pixelSize: 22
                font.weight: Font.Bold
                wrapMode: Text.Wrap
                width: parent.width
            }

            ScrollView {
                width: parent.width
                height: parent.height - 82
                clip: true

                Text {
                    id: detailText

                    width: parent.width
                    text: root.detail.length > 0 ? root.detail : "No additional detail"
                    color: diagnosticColors.textMuted
                    font.family: type.monoFamily
                    font.pixelSize: 12
                    wrapMode: Text.WrapAnywhere
                }
            }
        }
    }
}
