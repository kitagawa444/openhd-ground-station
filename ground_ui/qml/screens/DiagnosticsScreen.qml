import QtQuick
import QtQuick.Controls
import OpenHDGroundUI

Item {
    id: root

    property string title: "DIAGNOSTICS"
    property string subtitle: ""
    property var metrics: []

    Colors {
        id: palette
    }

    Typography {
        id: type
    }

    Rectangle {
        anchors.fill: parent
        gradient: Gradient {
            GradientStop { position: 0.0; color: "#071219" }
            GradientStop { position: 0.55; color: "#0a1d28" }
            GradientStop { position: 1.0; color: "#060d12" }
        }
    }

    Rectangle {
        width: parent.width * 0.65
        height: parent.height * 0.44
        x: parent.width * 0.32
        y: -parent.height * 0.24
        radius: width / 2
        color: "#143b6b55"
    }

    Column {
        id: heading

        anchors.left: parent.left
        anchors.leftMargin: 30
        anchors.right: parent.right
        anchors.rightMargin: 30
        anchors.top: parent.top
        anchors.topMargin: 24
        spacing: 4

        Text {
            text: root.title
            color: palette.textPrimary
            font.family: type.displayFamily
            font.pixelSize: 30
            font.weight: Font.Bold
            font.letterSpacing: 1.4
        }

        Text {
            text: root.subtitle
            color: palette.textMuted
            font.family: type.bodyFamily
            font.pixelSize: 14
        }
    }

    Flickable {
        anchors.top: heading.bottom
        anchors.topMargin: 22
        anchors.left: parent.left
        anchors.leftMargin: 30
        anchors.right: parent.right
        anchors.rightMargin: 20
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 14
        clip: true
        contentWidth: width
        contentHeight: metricGrid.height

        ScrollBar.vertical: ScrollBar {}

        Grid {
            id: metricGrid

            width: parent.width - 12
            height: implicitHeight
            columns: width >= 1500 ? 5 : width >= 1160 ? 4 : 3
            spacing: 12

            Repeater {
                model: root.metrics

                delegate: DiagnosticCard {
                    required property var modelData

                    width: (metricGrid.width - (metricGrid.columns - 1) * metricGrid.spacing) / metricGrid.columns
                    section: modelData.section
                    label: modelData.label
                    value: modelData.value
                    detail: modelData.detail
                }
            }
        }
    }

    Text {
        anchors.centerIn: parent
        visible: root.metrics.length === 0
        text: "WAITING FOR OPENHD DATA"
        color: palette.textDim
        font.family: type.monoFamily
        font.pixelSize: 16
        font.letterSpacing: 1.4
    }
}
