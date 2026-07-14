import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import OpenHDGroundUI

ApplicationWindow {
    id: window

    visible: true
    width: 1600
    height: 900
    minimumWidth: 1280
    minimumHeight: 720
    title: "OpenHD Ground UI Prototype"

    Colors {
        id: palette
    }

    color: palette.windowBackground

    background: Rectangle {
        anchors.fill: parent
        gradient: Gradient {
            GradientStop {
                position: 0.0
                color: "#08141b"
            }
            GradientStop {
                position: 0.55
                color: "#091820"
            }
            GradientStop {
                position: 1.0
                color: "#050b10"
            }
        }

        Rectangle {
            width: parent.width * 0.55
            height: parent.height * 0.75
            x: parent.width * 0.34
            y: -parent.height * 0.16
            radius: width / 2
            color: "#14314f66"
            opacity: 0.55
        }

        Rectangle {
            width: parent.width * 0.38
            height: parent.height * 0.55
            x: -parent.width * 0.08
            y: parent.height * 0.58
            radius: width / 2
            color: "#12356a2d"
            opacity: 0.9
        }
    }

    StackLayout {
        id: screens

        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: navigation.top
        currentIndex: navigation.currentIndex

        FlightScreen {}

        DiagnosticsScreen {
            title: "FLIGHT TELEMETRY"
            subtitle: "Autopilot, position, attitude, GPS, power, and RC input"
            metrics: diagnosticModel.flightMetrics
        }

        DiagnosticsScreen {
            title: "LINK DIAGNOSTICS"
            subtitle: "OpenHD radio cards, telemetry flow, video transport, and FEC"
            metrics: diagnosticModel.linkMetrics
        }

        DiagnosticsScreen {
            title: "OPENHD SYSTEM"
            subtitle: "Air and Ground core health, camera status, power, networking, and radio modes"
            metrics: diagnosticModel.systemMetrics
        }

        MessagesScreen {
            messages: diagnosticModel.messages
        }

        DiagnosticsScreen {
            title: "PROTOCOL INSPECTOR"
            subtitle: "Every MAVLink/OpenHD frame observed by this ground station connection"
            metrics: diagnosticModel.protocolMetrics
        }
    }

    NavigationBar {
        id: navigation

        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
    }
}
