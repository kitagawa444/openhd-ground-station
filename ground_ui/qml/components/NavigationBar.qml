import QtQuick
import QtQuick.Controls
import OpenHDGroundUI

Item {
    id: root

    property int currentIndex: 0

    implicitHeight: 58

    Colors {
        id: navigationColors
    }

    Typography {
        id: type
    }

    Rectangle {
        anchors.fill: parent
        color: "#e60a1218"
        border.width: 1
        border.color: navigationColors.panelStroke
    }

    Row {
        anchors.centerIn: parent
        spacing: 8

        Repeater {
            model: ["FPV", "FLIGHT", "LINK", "SYSTEM", "MESSAGES", "PROTOCOL"]

            delegate: Button {
                required property int index
                required property string modelData

                width: 118
                height: 36
                text: modelData
                onClicked: root.currentIndex = index

                background: Rectangle {
                    radius: 18
                    color: root.currentIndex === index ? "#2635bccc" : "transparent"
                    border.width: 1
                    border.color: root.currentIndex === index ? navigationColors.accent : "#29425860"
                }

                contentItem: Text {
                    text: modelData
                    color: root.currentIndex === index ? "#f0fbff" : navigationColors.textMuted
                    font.family: type.monoFamily
                    font.pixelSize: 12
                    font.weight: Font.Bold
                    font.letterSpacing: 1.1
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
            }
        }
    }
}
