import QtQuick
import QtQuick.Layouts
import OpenHDGroundUI

Item {
    id: root

    implicitHeight: metrics.bottomPanelHeight

    Colors {
        id: palette
    }

    Metrics {
        id: metrics
    }

    Typography {
        id: type
    }

    Rectangle {
        anchors.fill: parent
        radius: metrics.radiusLarge
        color: palette.panelGlass
        border.width: 1
        border.color: palette.panelStroke
    }

    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: metrics.panelPadding
        anchors.rightMargin: metrics.panelPadding
        anchors.topMargin: 14
        anchors.bottomMargin: 14
        spacing: 24

        ColumnLayout {
            spacing: 6

            Text {
                text: "HOME DISTANCE"
                color: palette.textDim
                font.family: type.bodyFamily
                font.pixelSize: type.captionSize
                font.weight: Font.DemiBold
                font.letterSpacing: 1.2
            }

            Text {
                text: vehicleModel.homePositionValid ? vehicleModel.homeDistanceMeters.toFixed(0) + " m" : "N/A"
                color: palette.textPrimary
                font.family: type.displayFamily
                font.pixelSize: 24
                font.weight: Font.Bold
            }
        }

        ColumnLayout {
            spacing: 6

            Text {
                text: "POSITION"
                color: palette.textDim
                font.family: type.bodyFamily
                font.pixelSize: type.captionSize
                font.weight: Font.DemiBold
                font.letterSpacing: 1.2
            }

            Text {
                text: vehicleModel.positionValid
                      ? vehicleModel.latitude.toFixed(5) + ", " + vehicleModel.longitude.toFixed(5)
                      : "Waiting for GPS"
                color: palette.textPrimary
                font.family: type.monoFamily
                font.pixelSize: 15
            }
        }

        ColumnLayout {
            spacing: 6

            Text {
                text: "VIDEO PIPELINE"
                color: palette.textDim
                font.family: type.bodyFamily
                font.pixelSize: type.captionSize
                font.weight: Font.DemiBold
                font.letterSpacing: 1.2
            }

            Text {
                text: videoModel.codec + "  |  " + linkModel.videoBitrateKbps + " kbps"
                color: palette.textPrimary
                font.family: type.monoFamily
                font.pixelSize: 15
            }
        }

        Item {
            Layout.fillWidth: true
        }

        Rectangle {
            Layout.preferredWidth: 360
            Layout.fillHeight: true
            radius: metrics.radiusMedium
            color: palette.panelSoft
            border.width: 1
            border.color: palette.panelStroke

            Column {
                anchors.fill: parent
                anchors.leftMargin: 14
                anchors.rightMargin: 14
                anchors.topMargin: 10
                anchors.bottomMargin: 10
                spacing: 4

                Text {
                    text: "LATEST ALERT"
                    color: palette.textDim
                    font.family: type.bodyFamily
                    font.pixelSize: type.captionSize
                    font.weight: Font.DemiBold
                    font.letterSpacing: 1.2
                }

                Text {
                    text: alertModel.count > 0 ? alertModel.get(0).title : "No active warnings"
                    color: palette.textPrimary
                    font.family: type.displayFamily
                    font.pixelSize: 16
                    font.weight: Font.DemiBold
                }

                Text {
                    text: alertModel.count > 0
                          ? alertModel.get(0).message
                          : (demoMode ? "Telemetry, video, and battery are all within nominal demo bounds."
                                      : "Live OpenHD telemetry is healthy.")
                    color: palette.textMuted
                    font.family: type.bodyFamily
                    font.pixelSize: 13
                    wrapMode: Text.Wrap
                    maximumLineCount: 2
                }
            }
        }
    }
}
