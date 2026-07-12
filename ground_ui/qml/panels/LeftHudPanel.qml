import QtQuick
import OpenHDGroundUI

Item {
    id: root

    implicitWidth: metrics.hudWidth

    Colors {
        id: palette
    }

    Metrics {
        id: metrics
    }

    Column {
        anchors.fill: parent
        spacing: 14

        GaugeCard {
            label: "Altitude"
            value: vehicleModel.positionValid ? vehicleModel.relativeAltitude.toFixed(1) : "--"
            unit: "m"
            tone: palette.accent
        }

        GaugeCard {
            label: "Ground Speed"
            value: vehicleModel.positionValid ? vehicleModel.groundSpeed.toFixed(1) : "--"
            unit: "m/s"
            tone: palette.healthy
        }

        GaugeCard {
            label: "Vertical"
            value: vehicleModel.positionValid ? vehicleModel.verticalSpeed.toFixed(1) : "--"
            unit: "m/s"
            tone: vehicleModel.verticalSpeed >= 0 ? palette.caution : palette.accent
        }

        GaugeCard {
            label: "Heading"
            value: vehicleModel.positionValid ? Math.round(vehicleModel.headingDegrees).toString() : "--"
            unit: "deg"
            tone: palette.caution
        }
    }
}
