import QtQuick
import OpenHDGroundUI

Item {
    id: root

    property real heading: 0

    implicitWidth: 460
    implicitHeight: 62
    clip: true

    Typography {
        id: type
    }

    function normalize(degrees) {
        var normalized = Math.round(degrees) % 360
        return normalized < 0 ? normalized + 360 : normalized
    }

    function headingLabel(degrees) {
        var normalized = normalize(degrees)
        if (normalized === 0) return "N"
        if (normalized === 90) return "E"
        if (normalized === 180) return "S"
        if (normalized === 270) return "W"
        return normalized % 30 === 0 ? normalized : ""
    }

    Repeater {
        model: 15

        delegate: Item {
            required property int index

            readonly property real degreesFromCenter: (index - 7) * 15 - (root.heading % 15)
            readonly property real currentHeading: root.heading + degreesFromCenter

            x: root.width / 2 + degreesFromCenter * 3.0 - width / 2
            width: 42
            height: root.height

            Rectangle {
                width: index % 2 === 1 ? 1 : 2
                height: index % 2 === 1 ? 8 : 14
                anchors.top: parent.top
                anchors.horizontalCenter: parent.horizontalCenter
                color: "#e8ffffff"
            }

            Text {
                anchors.top: parent.top
                anchors.topMargin: 17
                anchors.horizontalCenter: parent.horizontalCenter
                text: root.headingLabel(parent.currentHeading)
                color: root.normalize(parent.currentHeading) === 0 ? "#ffdf71" : "#f5fbfc"
                font.family: type.monoFamily
                font.pixelSize: 13
                font.weight: Font.Bold
                style: Text.Outline
                styleColor: "#e8000000"
            }
        }
    }

    Item {
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: parent.top
        width: 18
        height: 30

        Rectangle {
            width: 2
            height: 22
            anchors.horizontalCenter: parent.horizontalCenter
            color: "#ffdf71"
        }

        Text {
            anchors.top: parent.top
            anchors.topMargin: 26
            anchors.horizontalCenter: parent.horizontalCenter
            text: root.normalize(root.heading).toString().padStart(3, "0")
            color: "#ffdf71"
            font.family: type.monoFamily
            font.pixelSize: 12
            font.weight: Font.Bold
            style: Text.Outline
            styleColor: "#e8000000"
        }
    }
}
