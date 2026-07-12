#pragma once

#include <QDateTime>
#include <QString>

namespace openhd {

enum class AlertSeverity {
    Info,
    Caution,
    Critical,
};

struct VehicleState {
    bool connected = false;
    bool armed = false;
    QString flightMode = QStringLiteral("WAITING");
    bool positionValid = false;
    bool batteryValid = false;
    bool homePositionValid = false;
    double latitude = 0.0;
    double longitude = 0.0;
    double relativeAltitude = 0.0;
    double groundSpeed = 0.0;
    double verticalSpeed = 0.0;
    int batteryPercent = -1;
    double batteryVoltage = 0.0;
    int satellites = 0;
    double headingDegrees = 0.0;
    double homeLatitude = 0.0;
    double homeLongitude = 0.0;
    double homeDistanceMeters = -1.0;
};

struct LinkState {
    bool backendConnected = false;
    bool linkConnected = false;
    int linkQualityPercent = -1;
    bool telemetryHealthy = false;
    bool videoHealthy = false;
    int videoBitrateKbps = 0;
    double packetLossPercent = -1.0;
    int latencyMs = -1;
    bool rcConnected = false;
};

struct VideoState {
    bool videoAvailable = false;
    bool frameAvailable = false;
    bool recording = false;
    int streamWidth = 0;
    int streamHeight = 0;
    int streamFps = 0;
    QString codec = QStringLiteral("Unknown");
    QString decoderState = QStringLiteral("Waiting for OpenHD");
    int lastFrameAgeMs = -1;
    quint64 frameSequence = 0;
};

struct AlertItem {
    QString id;
    AlertSeverity severity = AlertSeverity::Info;
    QString title;
    QString message;
    bool sticky = false;
    QDateTime timestamp = QDateTime::currentDateTimeUtc();
};

}  // namespace openhd
