import QtQuick
import OpenHDGroundUI

Item {
    id: root

    property string label: ""
    property string value: ""
    property color valueColor: "#f4f7f9"
    property bool alignRight: false

    implicitWidth: Math.max(labelText.implicitWidth, valueText.implicitWidth)
    implicitHeight: labelText.implicitHeight + valueText.implicitHeight + 2

    Typography {
        id: type
    }

    Column {
        anchors.left: root.alignRight ? undefined : parent.left
        anchors.right: root.alignRight ? parent.right : undefined
        spacing: 0

        Text {
            id: labelText

            text: root.label
            color: "#d9f2f6"
            font.family: type.monoFamily
            font.pixelSize: 10
            font.weight: Font.Bold
            font.letterSpacing: 1.1
            horizontalAlignment: root.alignRight ? Text.AlignRight : Text.AlignLeft
            style: Text.Outline
            styleColor: "#d9000000"
        }

        Text {
            id: valueText

            text: root.value
            color: root.valueColor
            font.family: type.monoFamily
            font.pixelSize: 19
            font.weight: Font.Bold
            horizontalAlignment: root.alignRight ? Text.AlignRight : Text.AlignLeft
            style: Text.Outline
            styleColor: "#e8000000"
        }
    }
}
