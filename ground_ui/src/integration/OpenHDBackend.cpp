#include "integration/OpenHDBackend.hpp"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <limits>

#include <QDateTime>
#include <QHostAddress>

#include "common/types.hpp"
#include "services/AlertService.hpp"
#include "services/LinkStateService.hpp"
#include "services/VehicleStateService.hpp"
#include "services/VideoService.hpp"
#include "video/VideoFrameProvider.hpp"

namespace openhd {

namespace {

constexpr int kReconnectIntervalMs = 1500;
constexpr int kTelemetryStaleMs = 2500;
constexpr int kVideoStaleMs = 1500;
constexpr double kEarthRadiusMeters = 6371000.0;
constexpr double kDegreesToRadians = 0.017453292519943295;

constexpr quint32 kHeartbeat = 0;
constexpr quint32 kSystemStatus = 1;
constexpr quint32 kGpsRawInt = 24;
constexpr quint32 kGlobalPositionInt = 33;
constexpr quint32 kVfrHud = 74;
constexpr quint32 kBatteryStatus = 147;
constexpr quint32 kHomePosition = 242;
constexpr quint32 kOpenHDMonitorModeWifiLink = 1211;
constexpr quint32 kOpenHDMonitorModeWifiCard = 1212;
constexpr quint32 kOpenHDTelemetryStats = 1213;
constexpr quint32 kOpenHDVideoGroundStats = 1216;
constexpr quint32 kOpenHDCameraStatusAir = 1219;

quint8 readU8(const QByteArray &payload, const int offset) {
    return static_cast<quint8>(payload.at(offset));
}

qint8 readI8(const QByteArray &payload, const int offset) {
    return static_cast<qint8>(payload.at(offset));
}

quint16 readU16(const QByteArray &payload, const int offset) {
    return static_cast<quint16>(readU8(payload, offset)) |
           (static_cast<quint16>(readU8(payload, offset + 1)) << 8U);
}

qint16 readI16(const QByteArray &payload, const int offset) {
    return static_cast<qint16>(readU16(payload, offset));
}

quint32 readU32(const QByteArray &payload, const int offset) {
    return static_cast<quint32>(readU8(payload, offset)) |
           (static_cast<quint32>(readU8(payload, offset + 1)) << 8U) |
           (static_cast<quint32>(readU8(payload, offset + 2)) << 16U) |
           (static_cast<quint32>(readU8(payload, offset + 3)) << 24U);
}

qint32 readI32(const QByteArray &payload, const int offset) {
    return static_cast<qint32>(readU32(payload, offset));
}

float readFloat(const QByteArray &payload, const int offset) {
    const quint32 raw = readU32(payload, offset);
    float value = 0.0F;
    static_assert(sizeof(value) == sizeof(raw));
    std::memcpy(&value, &raw, sizeof(value));
    return value;
}

bool hasBytes(const MavlinkFrame &frame, const int required) {
    return frame.payload.size() >= required;
}

AlertItem makeAlert(const QString &id,
                    const AlertSeverity severity,
                    const QString &title,
                    const QString &message,
                    const bool sticky = true) {
    AlertItem alert;
    alert.id = id;
    alert.severity = severity;
    alert.title = title;
    alert.message = message;
    alert.sticky = sticky;
    alert.timestamp = QDateTime::currentDateTimeUtc();
    return alert;
}

QString arduCopterModeName(const quint32 customMode) {
    switch (customMode) {
    case 0:
        return QStringLiteral("STABILIZE");
    case 1:
        return QStringLiteral("ACRO");
    case 2:
        return QStringLiteral("ALT HOLD");
    case 3:
        return QStringLiteral("AUTO");
    case 4:
        return QStringLiteral("GUIDED");
    case 5:
        return QStringLiteral("LOITER");
    case 6:
        return QStringLiteral("RTL");
    case 9:
        return QStringLiteral("LAND");
    case 16:
        return QStringLiteral("POSHOLD");
    case 17:
        return QStringLiteral("BRAKE");
    case 20:
        return QStringLiteral("GUIDED NOGPS");
    case 21:
        return QStringLiteral("SMART RTL");
    default:
        return QStringLiteral("MODE %1").arg(customMode);
    }
}

QString px4ModeName(const quint32 customMode) {
    const quint8 mainMode = static_cast<quint8>((customMode >> 16U) & 0xFFU);
    const quint8 subMode = static_cast<quint8>((customMode >> 24U) & 0xFFU);

    switch (mainMode) {
    case 1:
        return QStringLiteral("MANUAL");
    case 2:
        return QStringLiteral("ALTCTL");
    case 3:
        return QStringLiteral("POSCTL");
    case 4:
        return subMode == 4 ? QStringLiteral("AUTO RTL")
                            : subMode == 5 ? QStringLiteral("AUTO LAND")
                                           : QStringLiteral("AUTO");
    case 5:
        return QStringLiteral("ACRO");
    case 6:
        return QStringLiteral("OFFBOARD");
    case 7:
        return QStringLiteral("STABILIZED");
    case 8:
        return QStringLiteral("RATTITUDE");
    default:
        return QStringLiteral("PX4 MODE %1").arg(mainMode);
    }
}

QString flightModeName(const quint8 autopilot, const quint32 customMode) {
    // MAV_AUTOPILOT_ARDUPILOTMEGA = 3 and MAV_AUTOPILOT_PX4 = 12.
    if (autopilot == 3) {
        return arduCopterModeName(customMode);
    }

    if (autopilot == 12) {
        return px4ModeName(customMode);
    }

    return QStringLiteral("MODE %1").arg(customMode);
}

double distanceMeters(const double latitudeA,
                      const double longitudeA,
                      const double latitudeB,
                      const double longitudeB) {
    const double latA = latitudeA * kDegreesToRadians;
    const double latB = latitudeB * kDegreesToRadians;
    const double deltaLat = (latitudeB - latitudeA) * kDegreesToRadians;
    const double deltaLon = (longitudeB - longitudeA) * kDegreesToRadians;
    const double sinLat = std::sin(deltaLat / 2.0);
    const double sinLon = std::sin(deltaLon / 2.0);
    const double a = sinLat * sinLat + std::cos(latA) * std::cos(latB) * sinLon * sinLon;
    return kEarthRadiusMeters * 2.0 * std::atan2(std::sqrt(a), std::sqrt(1.0 - a));
}

}  // namespace

OpenHDBackend::OpenHDBackend(VehicleStateService *vehicleStateService,
                             LinkStateService *linkStateService,
                             VideoService *videoService,
                             AlertService *alertService,
                             VideoFrameProvider *videoFrameProvider,
                             QObject *parent,
                             const quint16 telemetryTcpPort)
    : QObject(parent),
      m_vehicleStateService(vehicleStateService),
      m_linkStateService(linkStateService),
      m_videoService(videoService),
      m_alertService(alertService),
      m_videoFrameProvider(videoFrameProvider),
      m_telemetryTcpPort(telemetryTcpPort),
      m_telemetrySocket(this),
      m_rtpVideoReceiver(this) {
    m_reconnectTimer.setInterval(kReconnectIntervalMs);
    m_healthTimer.setInterval(250);

    connect(&m_reconnectTimer, &QTimer::timeout, this, &OpenHDBackend::connectToGround);
    connect(&m_healthTimer, &QTimer::timeout, this, &OpenHDBackend::refreshHealth);
    connect(&m_telemetrySocket, &QTcpSocket::connected, this, [this]() {
        setBackendConnected(true);
        m_reconnectTimer.stop();
        refreshAlerts();
    });
    connect(&m_telemetrySocket, &QTcpSocket::disconnected, this, [this]() {
        setBackendConnected(false);
        m_seenFlightTelemetry = false;
        m_reconnectTimer.start();
        refreshAlerts();
    });
    connect(&m_telemetrySocket,
            &QTcpSocket::errorOccurred,
            this,
            [this](const QAbstractSocket::SocketError) {
                if (m_telemetrySocket.state() == QAbstractSocket::UnconnectedState) {
                    m_reconnectTimer.start();
                    refreshAlerts();
                }
    });
    connect(&m_telemetrySocket, &QTcpSocket::readyRead, this, &OpenHDBackend::onTelemetryReadyRead);
    connect(&m_rtpVideoReceiver, &RtpVideoReceiver::frameReady, this, [this](const QImage &frame) {
        if (m_videoFrameProvider == nullptr) {
            return;
        }

        m_videoFrameProvider->setFrame(frame);
        m_videoAge.restart();

        VideoState video = m_videoService->state();
        video.videoAvailable = true;
        video.frameAvailable = true;
        video.lastFrameAgeMs = 0;
        video.decoderState = QStringLiteral("Rendering RTP");
        ++video.frameSequence;
        m_videoService->updateState(video);
    });
}

void OpenHDBackend::start() {
    m_telemetryAge.start();
    m_videoAge.start();
    if (m_videoFrameProvider != nullptr) {
        m_rtpVideoReceiver.start(RtpVideoReceiver::Codec::H264);
    }

    m_healthTimer.start();
    connectToGround();
    refreshAlerts();
}

void OpenHDBackend::reconnect() {
    m_telemetrySocket.abort();
    m_reconnectTimer.start();
    connectToGround();
}

void OpenHDBackend::connectToGround() {
    if (m_telemetrySocket.state() != QAbstractSocket::UnconnectedState) {
        return;
    }

    m_telemetrySocket.connectToHost(QHostAddress::LocalHost, m_telemetryTcpPort);
}

void OpenHDBackend::onTelemetryReadyRead() {
    const auto frames = m_parser.consume(m_telemetrySocket.readAll());
    for (const auto &frame : frames) {
        handleFrame(frame);
    }
}

void OpenHDBackend::refreshHealth() {
    const bool telemetryFresh = m_seenFlightTelemetry && m_telemetryAge.elapsed() < kTelemetryStaleMs;
    LinkState link = m_linkStateService->state();
    link.telemetryHealthy = telemetryFresh;
    link.linkConnected = telemetryFresh;

    VehicleState vehicle = m_vehicleStateService->state();
    vehicle.connected = telemetryFresh;
    if (!telemetryFresh) {
        vehicle.armed = false;
        vehicle.flightMode = QStringLiteral("WAITING");
    }
    m_vehicleStateService->updateState(vehicle);

    VideoState video = m_videoService->state();
    if (video.frameAvailable && m_videoAge.elapsed() < kVideoStaleMs) {
        video.videoAvailable = true;
        video.lastFrameAgeMs = static_cast<int>(m_videoAge.elapsed());
        link.videoHealthy = true;
    } else {
        video.videoAvailable = false;
        video.frameAvailable = false;
        video.lastFrameAgeMs = -1;
        video.decoderState = QStringLiteral("Waiting for decoded RTP");
        link.videoHealthy = false;
        if (!telemetryFresh) {
            link.videoBitrateKbps = 0;
        }
    }
    m_videoService->updateState(video);
    m_linkStateService->updateState(link);
    refreshAlerts();
}

void OpenHDBackend::handleFrame(const MavlinkFrame &frame) {
    switch (frame.messageId) {
    case kHeartbeat:
        handleHeartbeat(frame);
        break;
    case kSystemStatus:
        handleSystemStatus(frame);
        break;
    case kBatteryStatus:
        handleBatteryStatus(frame);
        break;
    case kGpsRawInt:
        handleGpsRaw(frame);
        break;
    case kGlobalPositionInt:
        handleGlobalPosition(frame);
        break;
    case kVfrHud:
        handleVfrHud(frame);
        break;
    case kHomePosition:
        handleHomePosition(frame);
        break;
    case kOpenHDMonitorModeWifiLink:
    case kOpenHDMonitorModeWifiCard:
    case kOpenHDTelemetryStats:
        handleOpenHDLinkStats(frame);
        break;
    case kOpenHDVideoGroundStats:
        handleOpenHDVideoStats(frame);
        break;
    case kOpenHDCameraStatusAir:
        handleOpenHDCameraStatus(frame);
        break;
    default:
        break;
    }
}

void OpenHDBackend::handleHeartbeat(const MavlinkFrame &frame) {
    if (!isFlightControllerFrame(frame) || !hasBytes(frame, 9)) {
        return;
    }

    const quint32 customMode = readU32(frame.payload, 0);
    const quint8 autopilot = readU8(frame.payload, 5);
    const quint8 baseMode = readU8(frame.payload, 6);

    VehicleState vehicle = m_vehicleStateService->state();
    vehicle.armed = (baseMode & 0x80U) != 0;
    vehicle.flightMode = flightModeName(autopilot, customMode);
    vehicle.connected = true;
    m_vehicleStateService->updateState(vehicle);
    m_seenFlightTelemetry = true;
    m_telemetryAge.restart();
}

void OpenHDBackend::handleSystemStatus(const MavlinkFrame &frame) {
    if (!isFlightControllerFrame(frame) || !hasBytes(frame, 31)) {
        return;
    }

    VehicleState vehicle = m_vehicleStateService->state();
    const quint16 voltageMillivolts = readU16(frame.payload, 14);
    const qint8 batteryRemaining = readI8(frame.payload, 30);
    if (voltageMillivolts != std::numeric_limits<quint16>::max()) {
        vehicle.batteryVoltage = static_cast<double>(voltageMillivolts) / 1000.0;
        vehicle.batteryValid = true;
    }
    if (batteryRemaining >= 0) {
        vehicle.batteryPercent = batteryRemaining;
        vehicle.batteryValid = true;
    }
    vehicle.connected = true;
    m_vehicleStateService->updateState(vehicle);
    m_seenFlightTelemetry = true;
    m_telemetryAge.restart();
}

void OpenHDBackend::handleBatteryStatus(const MavlinkFrame &frame) {
    if (!isFlightControllerFrame(frame) || !hasBytes(frame, 36)) {
        return;
    }

    int voltageMillivolts = 0;
    for (int index = 0; index < 10; ++index) {
        const quint16 cellVoltage = readU16(frame.payload, 10 + index * 2);
        if (cellVoltage > 1 && cellVoltage != std::numeric_limits<quint16>::max()) {
            voltageMillivolts += cellVoltage;
        }
    }

    VehicleState vehicle = m_vehicleStateService->state();
    if (voltageMillivolts > 0) {
        vehicle.batteryVoltage = static_cast<double>(voltageMillivolts) / 1000.0;
        vehicle.batteryValid = true;
    }
    const qint8 batteryRemaining = readI8(frame.payload, 35);
    if (batteryRemaining >= 0) {
        vehicle.batteryPercent = batteryRemaining;
        vehicle.batteryValid = true;
    }
    vehicle.connected = true;
    m_vehicleStateService->updateState(vehicle);
    m_seenFlightTelemetry = true;
    m_telemetryAge.restart();
}

void OpenHDBackend::handleGlobalPosition(const MavlinkFrame &frame) {
    if (!isFlightControllerFrame(frame) || !hasBytes(frame, 28)) {
        return;
    }

    VehicleState vehicle = m_vehicleStateService->state();
    vehicle.latitude = static_cast<double>(readI32(frame.payload, 4)) / 10000000.0;
    vehicle.longitude = static_cast<double>(readI32(frame.payload, 8)) / 10000000.0;
    vehicle.relativeAltitude = static_cast<double>(readI32(frame.payload, 16)) / 1000.0;
    const double northSpeed = static_cast<double>(readI16(frame.payload, 20)) / 100.0;
    const double eastSpeed = static_cast<double>(readI16(frame.payload, 22)) / 100.0;
    vehicle.groundSpeed = std::hypot(northSpeed, eastSpeed);
    vehicle.verticalSpeed = -static_cast<double>(readI16(frame.payload, 24)) / 100.0;
    const quint16 heading = readU16(frame.payload, 26);
    if (heading != std::numeric_limits<quint16>::max()) {
        vehicle.headingDegrees = static_cast<double>(heading) / 100.0;
    }
    vehicle.positionValid = true;
    vehicle.connected = true;
    if (vehicle.homePositionValid) {
        vehicle.homeDistanceMeters = distanceMeters(vehicle.homeLatitude,
                                                    vehicle.homeLongitude,
                                                    vehicle.latitude,
                                                    vehicle.longitude);
    }
    m_vehicleStateService->updateState(vehicle);
    m_seenFlightTelemetry = true;
    m_telemetryAge.restart();
}

void OpenHDBackend::handleGpsRaw(const MavlinkFrame &frame) {
    if (!isFlightControllerFrame(frame) || !hasBytes(frame, 30)) {
        return;
    }

    VehicleState vehicle = m_vehicleStateService->state();
    const quint8 satellites = readU8(frame.payload, 29);
    if (satellites != std::numeric_limits<quint8>::max()) {
        vehicle.satellites = satellites;
    }
    vehicle.connected = true;
    m_vehicleStateService->updateState(vehicle);
    m_seenFlightTelemetry = true;
    m_telemetryAge.restart();
}

void OpenHDBackend::handleVfrHud(const MavlinkFrame &frame) {
    if (!isFlightControllerFrame(frame) || !hasBytes(frame, 20)) {
        return;
    }

    VehicleState vehicle = m_vehicleStateService->state();
    vehicle.groundSpeed = readFloat(frame.payload, 4);
    vehicle.verticalSpeed = readFloat(frame.payload, 12);
    const qint16 heading = readI16(frame.payload, 16);
    if (heading >= 0 && heading <= 360) {
        vehicle.headingDegrees = heading;
    }
    vehicle.connected = true;
    m_vehicleStateService->updateState(vehicle);
    m_seenFlightTelemetry = true;
    m_telemetryAge.restart();
}

void OpenHDBackend::handleHomePosition(const MavlinkFrame &frame) {
    if (!isFlightControllerFrame(frame) || !hasBytes(frame, 12)) {
        return;
    }

    VehicleState vehicle = m_vehicleStateService->state();
    vehicle.homeLatitude = static_cast<double>(readI32(frame.payload, 0)) / 10000000.0;
    vehicle.homeLongitude = static_cast<double>(readI32(frame.payload, 4)) / 10000000.0;
    vehicle.homePositionValid = true;
    if (vehicle.positionValid) {
        vehicle.homeDistanceMeters = distanceMeters(vehicle.homeLatitude,
                                                    vehicle.homeLongitude,
                                                    vehicle.latitude,
                                                    vehicle.longitude);
    }
    m_vehicleStateService->updateState(vehicle);
    m_seenFlightTelemetry = true;
    m_telemetryAge.restart();
}

void OpenHDBackend::handleOpenHDLinkStats(const MavlinkFrame &frame) {
    LinkState link = m_linkStateService->state();

    if (frame.messageId == kOpenHDMonitorModeWifiCard && hasBytes(frame, 33)) {
        const int quality = readI8(frame.payload, 29);
        if (quality >= 0 && quality <= 100) {
            link.linkQualityPercent = quality;
        }
        const int loss = readI8(frame.payload, 32);
        if (loss >= 0 && loss <= 100) {
            link.packetLossPercent = loss;
        }
    } else if (frame.messageId == kOpenHDMonitorModeWifiLink && hasBytes(frame, 33)) {
        const int loss = readI8(frame.payload, 32);
        if (loss >= 0 && loss <= 100) {
            link.packetLossPercent = loss;
        }
    } else if (frame.messageId == kOpenHDTelemetryStats && hasBytes(frame, 18)) {
        const int loss = readI16(frame.payload, 16);
        if (loss >= 0 && loss <= 100) {
            link.packetLossPercent = loss;
        }
    }

    m_linkStateService->updateState(link);
}

void OpenHDBackend::handleOpenHDVideoStats(const MavlinkFrame &frame) {
    if (!hasBytes(frame, 4)) {
        return;
    }

    LinkState link = m_linkStateService->state();
    link.videoBitrateKbps = std::max(0, readI32(frame.payload, 0) / 1000);
    m_linkStateService->updateState(link);
}

void OpenHDBackend::handleOpenHDCameraStatus(const MavlinkFrame &frame) {
    if (!hasBytes(frame, 21)) {
        return;
    }

    VideoState video = m_videoService->state();
    video.streamWidth = readU16(frame.payload, 6);
    video.streamHeight = readU16(frame.payload, 8);
    video.streamFps = readU16(frame.payload, 10);
    video.recording = readU8(frame.payload, 17) != 0;
    RtpVideoReceiver::Codec codec = RtpVideoReceiver::Codec::H264;
    switch (readU8(frame.payload, 18)) {
    case 0:
        video.codec = QStringLiteral("H.264 RTP");
        codec = RtpVideoReceiver::Codec::H264;
        break;
    case 1:
        video.codec = QStringLiteral("H.265 RTP");
        codec = RtpVideoReceiver::Codec::H265;
        break;
    case 2:
        video.codec = QStringLiteral("MJPEG RTP");
        codec = RtpVideoReceiver::Codec::Mjpeg;
        break;
    default:
        video.codec = QStringLiteral("RTP");
        break;
    }
    const quint8 status = readU8(frame.payload, 15);
    if (status == 2) {
        video.decoderState = QStringLiteral("Air camera restarting");
    }
    m_videoService->updateState(video);

    if (m_videoFrameProvider != nullptr && m_rtpVideoReceiver.codec() != codec) {
        VideoState restartingVideo = m_videoService->state();
        restartingVideo.frameAvailable = false;
        restartingVideo.videoAvailable = false;
        restartingVideo.decoderState = QStringLiteral("Restarting RTP decoder");
        m_videoService->updateState(restartingVideo);
        m_rtpVideoReceiver.start(codec);
    }
}

void OpenHDBackend::refreshAlerts() {
    QVector<AlertItem> alerts;
    const LinkState link = m_linkStateService->state();
    const VehicleState vehicle = m_vehicleStateService->state();
    const VideoState video = m_videoService->state();

    if (!link.backendConnected) {
        alerts.push_back(makeAlert(QStringLiteral("openhd-backend-offline"),
                                   AlertSeverity::Caution,
                                   QStringLiteral("OpenHD Ground is not running"),
                                   QStringLiteral("Waiting for the local telemetry server on TCP 5760.")));
    } else if (!link.linkConnected) {
        alerts.push_back(makeAlert(QStringLiteral("air-telemetry-waiting"),
                                   AlertSeverity::Caution,
                                   QStringLiteral("Waiting for Air telemetry"),
                                   QStringLiteral("Ground is connected; start the Air unit or check the wireless link.")));
    }

    if (link.linkConnected && link.linkQualityPercent >= 0 && link.linkQualityPercent < 45) {
        alerts.push_back(makeAlert(QStringLiteral("link-quality-low"),
                                   AlertSeverity::Caution,
                                   QStringLiteral("Link quality low"),
                                   QStringLiteral("OpenHD reports %1% receive quality.").arg(link.linkQualityPercent)));
    }

    if (vehicle.batteryValid && vehicle.batteryPercent >= 0 && vehicle.batteryPercent < 30) {
        alerts.push_back(makeAlert(QStringLiteral("battery-low"),
                                   AlertSeverity::Caution,
                                   QStringLiteral("Battery reserve low"),
                                   QStringLiteral("Flight controller reports %1% remaining.").arg(vehicle.batteryPercent)));
    }

    if (link.linkConnected && !video.videoAvailable) {
        alerts.push_back(makeAlert(QStringLiteral("video-waiting"),
                                   AlertSeverity::Info,
                                   QStringLiteral("Waiting for video RTP"),
                                   QStringLiteral("Telemetry is live, but no packets have reached UDP 5800."),
                                   false));
    }

    m_alertService->setSystemAlerts(alerts);
}

void OpenHDBackend::setBackendConnected(const bool connected) {
    LinkState link = m_linkStateService->state();
    link.backendConnected = connected;
    if (!connected) {
        link.linkConnected = false;
        link.telemetryHealthy = false;
        link.videoHealthy = false;
        link.linkQualityPercent = -1;
        link.packetLossPercent = -1.0;
        link.videoBitrateKbps = 0;
    }
    m_linkStateService->updateState(link);
}

bool OpenHDBackend::isFlightControllerFrame(const MavlinkFrame &frame) const {
    if (frame.systemId == 100 || frame.systemId == 101) {
        return false;
    }

    // MAV_AUTOPILOT_INVALID is 8. A non-invalid heartbeat identifies the FC;
    // other standard messages are accepted after that FC begins reporting.
    if (frame.messageId == kHeartbeat && hasBytes(frame, 6)) {
        return readU8(frame.payload, 5) != 8;
    }

    return frame.systemId != 0;
}

}  // namespace openhd
