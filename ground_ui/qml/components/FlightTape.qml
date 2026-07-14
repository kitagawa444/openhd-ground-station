import QtQuick
import OpenHDGroundUI

Item {
    id: root

    property real value: 0
    property string unit: "m"
    property string label: "ALT"
    property bool leftSide: true
    property int step: 10

    implicitWidth: 118
    implicitHeight: 330
    clip: true

    Typography {
        id: type
    }

    readonly property int roundedValue: Math.round(value)

    Rectangle {
        width: 1
        height: parent.height
        color: "#d9ffffff"
        anchors.left: root.leftSide ? parent.left : undefined
        anchors.right: root.leftSide ? undefined : parent.right
    }

    Repeater {
        model: 9

        delegate: Item {
            required property int index

            width: root.width
            height: 26
            y: root.height * index / 8 - height / 2
            readonly property int tickValue: root.roundedValue + (4 - index) * root.step

            Rectangle {
                width: index % 2 === 0 ? 20 : 12
                height: 1
                color: "#d9ffffff"
                anchors.left: root.leftSide ? parent.left : undefined
                anchors.right: root.leftSide ? undefined : parent.right
                anchors.verticalCenter: parent.verticalCenter
            }

            Text {
                anchors.left: root.leftSide ? parent.left : undefined
                anchors.right: root.leftSide ? undefined : parent.right
                anchors.leftMargin: root.leftSide ? 28 : 0
                anchors.rightMargin: root.leftSide ? 0 : 28
                anchors.verticalCenter: parent.verticalCenter
                text: parent.tickValue
                color: "#f5fbfc"
                font.family: type.monoFamily
                font.pixelSize: index % 2 === 0 ? 13 : 11
                font.weight: Font.DemiBold
                horizontalAlignment: root.leftSide ? Text.AlignLeft : Text.AlignRight
                style: Text.Outline
                styleColor: "#e8000000"
            }
        }
    }

    Rectangle {
        width: 84
        height: 38
        anchors.left: root.leftSide ? parent.left : undefined
        anchors.right: root.leftSide ? undefined : parent.right
        anchors.verticalCenter: parent.verticalCenter
        color: "#c9081015"
        border.width: 1
        border.color: "#f4f7f9"

        Text {
            anchors.centerIn: parent
            text: root.roundedValue + root.unit
            color: "#ffffff"
            font.family: type.monoFamily
            font.pixelSize: 17
            font.weight: Font.Bold
        }
    }

    Text {
        anchors.left: root.leftSide ? parent.left : undefined
        anchors.right: root.leftSide ? undefined : parent.right
        anchors.top: parent.top
        anchors.topMargin: -2
        text: root.label
        color: "#d9f2f6"
        font.family: type.monoFamily
        font.pixelSize: 10
        font.weight: Font.Bold
        font.letterSpacing: 1.1
        style: Text.Outline
        styleColor: "#e8000000"
    }
}
