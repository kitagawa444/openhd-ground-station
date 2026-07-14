import QtQuick
import OpenHDGroundUI

Item {
    id: root

    property real roll: 0
    property real pitch: 0

    implicitWidth: 430
    implicitHeight: 300
    clip: true

    Typography {
        id: type
    }

    Item {
        id: ladder

        width: root.width * 1.5
        height: root.height * 1.5
        anchors.horizontalCenter: parent.horizontalCenter
        y: (root.height - height) / 2 + root.pitch * 3.2
        rotation: -root.roll
        transformOrigin: Item.Center

        Rectangle {
            width: parent.width
            height: 2
            anchors.verticalCenter: parent.verticalCenter
            color: "#f0ffffff"
        }

        Repeater {
            model: 9

            delegate: Item {
                required property int index

                readonly property int pitchMark: (4 - index) * 5
                width: parent.width
                height: 28
                anchors.horizontalCenter: parent.horizontalCenter
                y: parent.height / 2 + (index - 4) * 30 - height / 2
                visible: pitchMark !== 0

                Rectangle {
                    width: Math.abs(parent.pitchMark) % 10 === 0 ? 116 : 66
                    height: 1
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.verticalCenter: parent.verticalCenter
                    color: "#e8ffffff"
                }

                Text {
                    anchors.right: parent.horizontalCenter
                    anchors.rightMargin: 68
                    anchors.verticalCenter: parent.verticalCenter
                    text: Math.abs(parent.pitchMark)
                    color: "#e8ffffff"
                    font.family: type.monoFamily
                    font.pixelSize: 11
                    style: Text.Outline
                    styleColor: "#e8000000"
                }

                Text {
                    anchors.left: parent.horizontalCenter
                    anchors.leftMargin: 68
                    anchors.verticalCenter: parent.verticalCenter
                    text: Math.abs(parent.pitchMark)
                    color: "#e8ffffff"
                    font.family: type.monoFamily
                    font.pixelSize: 11
                    style: Text.Outline
                    styleColor: "#e8000000"
                }
            }
        }
    }

    Item {
        anchors.centerIn: parent
        width: 114
        height: 34

        Rectangle {
            width: 42
            height: 2
            anchors.left: parent.left
            anchors.verticalCenter: parent.verticalCenter
            color: "#ffdf71"
        }

        Rectangle {
            width: 42
            height: 2
            anchors.right: parent.right
            anchors.verticalCenter: parent.verticalCenter
            color: "#ffdf71"
        }

        Rectangle {
            width: 2
            height: 20
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.verticalCenter: parent.verticalCenter
            color: "#ffdf71"
        }

        Rectangle {
            width: 20
            height: 2
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.bottom: parent.bottom
            color: "#ffdf71"
        }
    }

    Text {
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom
        text: "R " + root.roll.toFixed(0) + "  P " + root.pitch.toFixed(0)
        color: "#d9f2f6"
        font.family: type.monoFamily
        font.pixelSize: 11
        font.weight: Font.Bold
        style: Text.Outline
        styleColor: "#e8000000"
    }
}
