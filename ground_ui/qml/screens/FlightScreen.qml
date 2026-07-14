import QtQuick
import OpenHDGroundUI

Item {
    id: root

    readonly property color osdText: "#f5fbfc"
    readonly property color healthy: "#7cf3b4"
    readonly property color warning: "#ffdf71"
    readonly property color critical: "#ff7676"

    function linkColor() {
        if (!linkModel.linkConnected)
            return critical
        if (linkModel.linkQualityPercent >= 0 && linkModel.linkQualityPercent < 55)
            return warning
        return healthy
    }

    function batteryColor() {
        if (!vehicleModel.batteryValid)
            return osdText
        if (vehicleModel.batteryPercent < 25)
            return critical
        if (vehicleModel.batteryPercent < 45)
            return warning
        return healthy
    }

    function numberOrDash(value, decimals) {
        return value < 0 ? "--" : Number(value).toFixed(decimals)
    }

    VideoCanvas {
        anchors.fill: parent
        healthy: videoModel.videoAvailable
        recording: videoModel.recording
        decoderState: videoModel.decoderState
        streamWidth: videoModel.streamWidth
        streamHeight: videoModel.streamHeight
        streamFps: videoModel.streamFps
        codec: videoModel.codec
    }

    Rectangle {
        anchors.fill: parent
        gradient: Gradient {
            GradientStop { position: 0.0; color: "#12000000" }
            GradientStop { position: 0.62; color: "#00000000" }
            GradientStop { position: 1.0; color: "#36000000" }
        }
    }

    CompassRibbon {
        anchors.top: parent.top
        anchors.topMargin: 12
        anchors.horizontalCenter: parent.horizontalCenter
        heading: vehicleModel.headingDegrees
    }

    HorizonOverlay {
        anchors.centerIn: parent
        anchors.verticalCenterOffset: 18
        roll: vehicleModel.rollDegrees
        pitch: vehicleModel.pitchDegrees
    }

    FlightTape {
        anchors.left: parent.left
        anchors.leftMargin: 34
        anchors.verticalCenter: parent.verticalCenter
        anchors.verticalCenterOffset: 22
        value: vehicleModel.groundSpeed
        unit: ""
        label: "SPD m/s"
        leftSide: true
        step: 5
    }

    FlightTape {
        anchors.right: parent.right
        anchors.rightMargin: 34
        anchors.verticalCenter: parent.verticalCenter
        anchors.verticalCenterOffset: 22
        value: vehicleModel.relativeAltitude
        unit: "m"
        label: "ALT"
        leftSide: false
        step: 10
    }

    Column {
        anchors.left: parent.left
        anchors.leftMargin: 22
        anchors.top: parent.top
        anchors.topMargin: 18
        spacing: 13

        OsdReadout {
            label: "DOWNLINK"
            value: linkModel.rssiDbm > -127
                   ? linkModel.rssiDbm + " dBm  " + numberOrDash(linkModel.linkQualityPercent, 0) + "%"
                   : "WAITING"
            valueColor: root.linkColor()
        }

        OsdReadout {
            label: "VIDEO / LOSS"
            value: (linkModel.videoBitrateKbps / 1000).toFixed(1) + " Mbps  " + numberOrDash(linkModel.packetLossPercent, 1) + "%"
            valueColor: linkModel.videoHealthy ? root.osdText : root.warning
        }

        OsdReadout {
            label: "TELEMETRY"
            value: linkModel.telemetryHealthy ? "LINK ACTIVE" : "WAITING"
            valueColor: linkModel.telemetryHealthy ? root.healthy : root.warning
        }
    }

    Column {
        anchors.right: parent.right
        anchors.rightMargin: 22
        anchors.top: parent.top
        anchors.topMargin: 18
        spacing: 13

        OsdReadout {
            label: "FLIGHT BATTERY"
            value: vehicleModel.batteryValid
                   ? vehicleModel.batteryVoltage.toFixed(2) + " V  " + vehicleModel.batteryPercent + "%"
                   : "WAITING"
            valueColor: root.batteryColor()
            alignRight: true
        }

        OsdReadout {
            label: "GPS"
            value: vehicleModel.positionValid ? vehicleModel.satellites + " SAT  3D FIX" : "NO FIX"
            valueColor: vehicleModel.positionValid ? root.healthy : root.warning
            alignRight: true
        }

        OsdReadout {
            label: "UPLINK / RC"
            value: linkModel.rcConnected ? "RC CONNECTED" : "RC STATUS N/A"
            valueColor: linkModel.rcConnected ? root.healthy : root.osdText
            alignRight: true
        }
    }

    Rectangle {
        width: modeText.implicitWidth + 30
        height: 33
        radius: 3
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: parent.top
        anchors.topMargin: 74
        color: "#bb070b0e"
        border.width: 1
        border.color: vehicleModel.armed ? root.critical : root.healthy

        Text {
            id: modeText

            anchors.centerIn: parent
            text: (vehicleModel.armed ? "ARMED  " : "SAFE  ") + vehicleModel.flightMode
            color: vehicleModel.armed ? root.critical : root.healthy
            font.family: "JetBrains Mono"
            font.pixelSize: 14
            font.weight: Font.Bold
            font.letterSpacing: 1.0
            style: Text.Outline
            styleColor: "#e8000000"
        }
    }

    WarningBanner {
        alertData: alertModel.count > 0 ? alertModel.get(0) : null
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: flightData.top
        anchors.bottomMargin: 14
    }

    Row {
        id: flightData

        anchors.left: parent.left
        anchors.leftMargin: 22
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 18
        spacing: 26

        OsdReadout {
            label: "POSITION"
            value: vehicleModel.positionValid
                   ? vehicleModel.latitude.toFixed(5) + ", " + vehicleModel.longitude.toFixed(5)
                   : "NO POSITION"
        }

        OsdReadout {
            label: "VERTICAL SPEED"
            value: vehicleModel.verticalSpeed.toFixed(1) + " m/s"
        }
    }

    Row {
        anchors.right: parent.right
        anchors.rightMargin: 22
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 18
        spacing: 26

        OsdReadout {
            label: "HOME"
            value: vehicleModel.homeDistanceMeters >= 0
                   ? vehicleModel.homeDistanceMeters.toFixed(0) + " m"
                   : "NOT SET"
            alignRight: true
        }

        OsdReadout {
            visible: videoModel.recording
            label: "VIDEO"
            value: "REC"
            valueColor: root.critical
            alignRight: true
        }
    }
}
