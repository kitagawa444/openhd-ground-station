#include "integration/OpenHDBackend.hpp"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <limits>

#include <QDateTime>
#include <QHostAddress>

#include "common/types.hpp"
#include "services/AlertService.hpp"
#include "services/DiagnosticsService.hpp"
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
constexpr quint32 kAttitude = 30;
constexpr quint32 kGlobalPositionInt = 33;
constexpr quint32 kRcChannels = 65;
constexpr quint32 kVfrHud = 74;
constexpr quint32 kBatteryStatus = 147;
constexpr quint32 kHomePosition = 242;
constexpr quint32 kStatusText = 253;
constexpr quint32 kOpenHDMonitorModeWifiLink = 1211;
constexpr quint32 kOpenHDMonitorModeWifiCard = 1212;
constexpr quint32 kOpenHDTelemetryStats = 1213;
constexpr quint32 kOpenHDVideoAirStats = 1214;
constexpr quint32 kOpenHDVideoAirFecStats = 1215;
constexpr quint32 kOpenHDVideoGroundStats = 1216;
constexpr quint32 kOpenHDVideoGroundFecStats = 1217;
constexpr quint32 kOpenHDOnboardComputerStatusExtension = 1218;
constexpr quint32 kOpenHDCameraStatusAir = 1219;
constexpr quint32 kOpenHDVersionMessage = 1221;
constexpr quint32 kOpenHDSupportedChannels = 1222;
constexpr quint32 kOpenHDScanChannelsProgress = 1223;
constexpr quint32 kOpenHDAnalyzeChannelsProgress = 1224;
constexpr quint32 kOpenHDGroundOperatingMode = 1225;
constexpr quint32 kOpenHDSysStatus = 1226;
constexpr quint32 kOpenHDCoreStatus = 1227;
constexpr quint32 kOpenHDPowerStatus = 1228;
constexpr quint32 kOpenHDMicrohardStatus = 1229;
constexpr quint32 kOpenHDRtspConfiguration = 1230;

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

QString sourceName(const MavlinkFrame &frame) {
    if (frame.systemId == 100) {
        return QStringLiteral("Ground");
    }
    if (frame.systemId == 101) {
        return QStringLiteral("Air");
    }
    return QStringLiteral("FC %1").arg(frame.systemId);
}

QString messageName(const quint32 messageId) {
    switch (messageId) {
    case kHeartbeat:
        return QStringLiteral("HEARTBEAT");
    case kSystemStatus:
        return QStringLiteral("SYS_STATUS");
    case kGpsRawInt:
        return QStringLiteral("GPS_RAW_INT");
    case kAttitude:
        return QStringLiteral("ATTITUDE");
    case kGlobalPositionInt:
        return QStringLiteral("GLOBAL_POSITION_INT");
    case kRcChannels:
        return QStringLiteral("RC_CHANNELS");
    case kVfrHud:
        return QStringLiteral("VFR_HUD");
    case kBatteryStatus:
        return QStringLiteral("BATTERY_STATUS");
    case kHomePosition:
        return QStringLiteral("HOME_POSITION");
    case kStatusText:
        return QStringLiteral("STATUSTEXT");
    case kOpenHDMonitorModeWifiLink:
        return QStringLiteral("OHD WIFI LINK");
    case kOpenHDMonitorModeWifiCard:
        return QStringLiteral("OHD WIFI CARD");
    case kOpenHDTelemetryStats:
        return QStringLiteral("OHD TELEMETRY");
    case kOpenHDVideoAirStats:
        return QStringLiteral("OHD VIDEO AIR");
    case kOpenHDVideoAirFecStats:
        return QStringLiteral("OHD VIDEO AIR FEC");
    case kOpenHDVideoGroundStats:
        return QStringLiteral("OHD VIDEO GROUND");
    case kOpenHDVideoGroundFecStats:
        return QStringLiteral("OHD VIDEO GROUND FEC");
    case kOpenHDOnboardComputerStatusExtension:
        return QStringLiteral("OHD CPU EXTENSION");
    case kOpenHDCameraStatusAir:
        return QStringLiteral("OHD CAMERA STATUS");
    case kOpenHDVersionMessage:
        return QStringLiteral("OHD VERSION");
    case kOpenHDSupportedChannels:
        return QStringLiteral("OHD SUPPORTED CHANNELS");
    case kOpenHDScanChannelsProgress:
        return QStringLiteral("OHD CHANNEL SCAN");
    case kOpenHDAnalyzeChannelsProgress:
        return QStringLiteral("OHD CHANNEL ANALYZE");
    case kOpenHDGroundOperatingMode:
        return QStringLiteral("OHD GROUND MODE");
    case kOpenHDSysStatus:
        return QStringLiteral("OHD SYS STATUS");
    case kOpenHDCoreStatus:
        return QStringLiteral("OHD CORE STATUS");
    case kOpenHDPowerStatus:
        return QStringLiteral("OHD POWER STATUS");
    case kOpenHDMicrohardStatus:
        return QStringLiteral("OHD MICROHARD");
    case kOpenHDRtspConfiguration:
        return QStringLiteral("OHD RTSP CONFIG");
    default:
        return QStringLiteral("MAVLINK %1").arg(messageId);
    }
}

QString severityName(const quint8 severity) {
    if (severity <= 3) {
        return QStringLiteral("critical");
    }
    if (severity <= 4) {
        return QStringLiteral("caution");
    }
    if (severity <= 6) {
        return QStringLiteral("notice");
    }
    return QStringLiteral("info");
}

}  // namespace

OpenHDBackend::OpenHDBackend(VehicleStateService *vehicleStateService,
                             LinkStateService *linkStateService,
                             VideoService *videoService,
                             AlertService *alertService,
                             DiagnosticsService *diagnosticsService,
                             VideoFrameProvider *videoFrameProvider,
                             QObject *parent,
                             const quint16 telemetryTcpPort)
    : QObject(parent),
      m_vehicleStateService(vehicleStateService),
      m_linkStateService(linkStateService),
      m_videoService(videoService),
      m_alertService(alertService),
      m_diagnosticsService(diagnosticsService),
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
    if (m_diagnosticsService != nullptr) {
        m_diagnosticsService->observeFrame(frame.messageId,
                                           frame.systemId,
                                           frame.componentId,
                                           messageName(frame.messageId),
                                           frame.payload);
    }

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
    case kAttitude:
        handleAttitude(frame);
        break;
    case kGlobalPositionInt:
        handleGlobalPosition(frame);
        break;
    case kVfrHud:
        handleVfrHud(frame);
        break;
    case kRcChannels:
        handleRcChannels(frame);
        break;
    case kHomePosition:
        handleHomePosition(frame);
        break;
    case kStatusText:
        handleStatusText(frame);
        break;
    case kOpenHDMonitorModeWifiLink:
    case kOpenHDMonitorModeWifiCard:
    case kOpenHDTelemetryStats:
        handleOpenHDLinkStats(frame);
        break;
    case kOpenHDVideoAirStats:
    case kOpenHDVideoAirFecStats:
    case kOpenHDVideoGroundStats:
    case kOpenHDVideoGroundFecStats:
        handleOpenHDVideoStats(frame);
        break;
    case kOpenHDCameraStatusAir:
        handleOpenHDCameraStatus(frame);
        break;
    case kOpenHDOnboardComputerStatusExtension:
    case kOpenHDVersionMessage:
    case kOpenHDSupportedChannels:
    case kOpenHDScanChannelsProgress:
    case kOpenHDAnalyzeChannelsProgress:
    case kOpenHDGroundOperatingMode:
    case kOpenHDSysStatus:
    case kOpenHDCoreStatus:
    case kOpenHDPowerStatus:
    case kOpenHDMicrohardStatus:
    case kOpenHDRtspConfiguration:
        handleOpenHDSystemStatus(frame);
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
    const QString section = sourceName(frame);
    setMetric(QStringLiteral("flight"), section, QStringLiteral("mode"),
              QStringLiteral("Flight mode"), vehicle.flightMode);
    setMetric(QStringLiteral("flight"), section, QStringLiteral("armed"),
              QStringLiteral("Armed"), vehicle.armed ? QStringLiteral("YES") : QStringLiteral("NO"));
    setMetric(QStringLiteral("flight"), section, QStringLiteral("autopilot"),
              QStringLiteral("Autopilot"), QString::number(autopilot));
    setMetric(QStringLiteral("flight"), section, QStringLiteral("custom-mode"),
              QStringLiteral("Custom mode"), QString::number(customMode));
    setMetric(QStringLiteral("flight"), section, QStringLiteral("base-mode"),
              QStringLiteral("Base mode"), QStringLiteral("0x%1").arg(baseMode, 2, 16, QLatin1Char('0')));
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
    const QString section = sourceName(frame);
    setMetric(QStringLiteral("flight"), section, QStringLiteral("battery-voltage"),
              QStringLiteral("Battery voltage"), QStringLiteral("%1 V").arg(vehicle.batteryVoltage, 0, 'f', 2));
    setMetric(QStringLiteral("flight"), section, QStringLiteral("battery-remaining"),
              QStringLiteral("Battery remaining"), vehicle.batteryPercent >= 0
                                                   ? QStringLiteral("%1 %").arg(vehicle.batteryPercent)
                                                   : QStringLiteral("N/A"));
    setMetric(QStringLiteral("flight"), section, QStringLiteral("cpu-load"),
              QStringLiteral("Autopilot load"), QStringLiteral("%1 %").arg(readU16(frame.payload, 12) / 10.0, 0, 'f', 1));
    setMetric(QStringLiteral("flight"), section, QStringLiteral("comm-drop"),
              QStringLiteral("FC comm drop"), QStringLiteral("%1 %").arg(readU16(frame.payload, 18) / 100.0, 0, 'f', 2));
    setMetric(QStringLiteral("flight"), section, QStringLiteral("comm-errors"),
              QStringLiteral("FC comm errors"), QString::number(readU16(frame.payload, 20)));
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
    const QString section = sourceName(frame);
    setMetric(QStringLiteral("flight"), section, QStringLiteral("battery-status-voltage"),
              QStringLiteral("Battery pack voltage"), QStringLiteral("%1 V").arg(vehicle.batteryVoltage, 0, 'f', 2));
    setMetric(QStringLiteral("flight"), section, QStringLiteral("battery-status-remaining"),
              QStringLiteral("Battery status remaining"), QStringLiteral("%1 %").arg(vehicle.batteryPercent));
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
    const QString section = sourceName(frame);
    setMetric(QStringLiteral("flight"), section, QStringLiteral("latitude"),
              QStringLiteral("Latitude"), QString::number(vehicle.latitude, 'f', 7));
    setMetric(QStringLiteral("flight"), section, QStringLiteral("longitude"),
              QStringLiteral("Longitude"), QString::number(vehicle.longitude, 'f', 7));
    setMetric(QStringLiteral("flight"), section, QStringLiteral("altitude-msl"),
              QStringLiteral("Altitude MSL"), QStringLiteral("%1 m").arg(readI32(frame.payload, 12) / 1000.0, 0, 'f', 1));
    setMetric(QStringLiteral("flight"), section, QStringLiteral("altitude-relative"),
              QStringLiteral("Relative altitude"), QStringLiteral("%1 m").arg(vehicle.relativeAltitude, 0, 'f', 1));
    setMetric(QStringLiteral("flight"), section, QStringLiteral("ground-speed"),
              QStringLiteral("Ground speed"), QStringLiteral("%1 m/s").arg(vehicle.groundSpeed, 0, 'f', 2));
    setMetric(QStringLiteral("flight"), section, QStringLiteral("vertical-speed"),
              QStringLiteral("Vertical speed"), QStringLiteral("%1 m/s").arg(vehicle.verticalSpeed, 0, 'f', 2));
    setMetric(QStringLiteral("flight"), section, QStringLiteral("heading"),
              QStringLiteral("Heading"), QStringLiteral("%1 deg").arg(vehicle.headingDegrees, 0, 'f', 1));
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
    const QString section = sourceName(frame);
    setMetric(QStringLiteral("flight"), section, QStringLiteral("gps-fix"),
              QStringLiteral("GPS fix type"), QString::number(readU8(frame.payload, 28)));
    setMetric(QStringLiteral("flight"), section, QStringLiteral("gps-satellites"),
              QStringLiteral("GPS satellites"), QString::number(vehicle.satellites));
    setMetric(QStringLiteral("flight"), section, QStringLiteral("gps-hdop"),
              QStringLiteral("GPS HDOP"), QString::number(readU16(frame.payload, 20) / 100.0, 'f', 2));
    setMetric(QStringLiteral("flight"), section, QStringLiteral("gps-vdop"),
              QStringLiteral("GPS VDOP"), QString::number(readU16(frame.payload, 22) / 100.0, 'f', 2));
    setMetric(QStringLiteral("flight"), section, QStringLiteral("gps-course"),
              QStringLiteral("GPS course"), QStringLiteral("%1 deg").arg(readU16(frame.payload, 26) / 100.0, 0, 'f', 1));
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
    const QString section = sourceName(frame);
    setMetric(QStringLiteral("flight"), section, QStringLiteral("airspeed"),
              QStringLiteral("Airspeed"), QStringLiteral("%1 m/s").arg(readFloat(frame.payload, 0), 0, 'f', 2));
    setMetric(QStringLiteral("flight"), section, QStringLiteral("throttle"),
              QStringLiteral("Throttle"), QStringLiteral("%1 %").arg(readU16(frame.payload, 18)));
    setMetric(QStringLiteral("flight"), section, QStringLiteral("hud-altitude"),
              QStringLiteral("HUD altitude"), QStringLiteral("%1 m").arg(readFloat(frame.payload, 8), 0, 'f', 1));
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
    const QString section = sourceName(frame);
    setMetric(QStringLiteral("flight"), section, QStringLiteral("home-position"),
              QStringLiteral("Home position"),
              QStringLiteral("%1, %2").arg(vehicle.homeLatitude, 0, 'f', 7).arg(vehicle.homeLongitude, 0, 'f', 7));
    setMetric(QStringLiteral("flight"), section, QStringLiteral("home-distance"),
              QStringLiteral("Home distance"), vehicle.homeDistanceMeters >= 0
                                                 ? QStringLiteral("%1 m").arg(vehicle.homeDistanceMeters, 0, 'f', 1)
                                                 : QStringLiteral("N/A"));
    m_seenFlightTelemetry = true;
    m_telemetryAge.restart();
}

void OpenHDBackend::handleAttitude(const MavlinkFrame &frame) {
    if (!isFlightControllerFrame(frame) || !hasBytes(frame, 28)) {
        return;
    }

    constexpr double radiansToDegrees = 57.29577951308232;
    VehicleState vehicle = m_vehicleStateService->state();
    vehicle.rollDegrees = readFloat(frame.payload, 4) * radiansToDegrees;
    vehicle.pitchDegrees = readFloat(frame.payload, 8) * radiansToDegrees;
    vehicle.yawDegrees = readFloat(frame.payload, 12) * radiansToDegrees;
    vehicle.connected = true;
    m_vehicleStateService->updateState(vehicle);
    const QString section = sourceName(frame);
    setMetric(QStringLiteral("flight"), section, QStringLiteral("roll"),
              QStringLiteral("Roll"), QStringLiteral("%1 deg").arg(vehicle.rollDegrees, 0, 'f', 1));
    setMetric(QStringLiteral("flight"), section, QStringLiteral("pitch"),
              QStringLiteral("Pitch"), QStringLiteral("%1 deg").arg(vehicle.pitchDegrees, 0, 'f', 1));
    setMetric(QStringLiteral("flight"), section, QStringLiteral("yaw"),
              QStringLiteral("Yaw"), QStringLiteral("%1 deg").arg(vehicle.yawDegrees, 0, 'f', 1));
    setMetric(QStringLiteral("flight"), section, QStringLiteral("roll-rate"),
              QStringLiteral("Roll rate"), QStringLiteral("%1 deg/s").arg(readFloat(frame.payload, 16) * radiansToDegrees, 0, 'f', 1));
    setMetric(QStringLiteral("flight"), section, QStringLiteral("pitch-rate"),
              QStringLiteral("Pitch rate"), QStringLiteral("%1 deg/s").arg(readFloat(frame.payload, 20) * radiansToDegrees, 0, 'f', 1));
    setMetric(QStringLiteral("flight"), section, QStringLiteral("yaw-rate"),
              QStringLiteral("Yaw rate"), QStringLiteral("%1 deg/s").arg(readFloat(frame.payload, 24) * radiansToDegrees, 0, 'f', 1));
    m_seenFlightTelemetry = true;
    m_telemetryAge.restart();
}

void OpenHDBackend::handleRcChannels(const MavlinkFrame &frame) {
    if (!isFlightControllerFrame(frame) || !hasBytes(frame, 42)) {
        return;
    }

    const QString section = QStringLiteral("RC input");
    setMetric(QStringLiteral("flight"), section, QStringLiteral("channel-count"),
              QStringLiteral("Channel count"), QString::number(readU8(frame.payload, 40)));
    setMetric(QStringLiteral("flight"), section, QStringLiteral("rssi"),
              QStringLiteral("RC RSSI"), QString::number(readU8(frame.payload, 41)));
    for (int channel = 0; channel < 18; ++channel) {
        setMetric(QStringLiteral("flight"), section, QStringLiteral("channel-%1").arg(channel + 1),
                  QStringLiteral("CH %1").arg(channel + 1),
                  QStringLiteral("%1 us").arg(readU16(frame.payload, 4 + channel * 2)));
    }
}

void OpenHDBackend::handleStatusText(const MavlinkFrame &frame) {
    if (!hasBytes(frame, 51) || m_diagnosticsService == nullptr) {
        return;
    }

    const QString text = QString::fromLatin1(frame.payload.mid(1, 50)).split(QLatin1Char('\0')).front();
    m_diagnosticsService->appendMessage(sourceName(frame), severityName(readU8(frame.payload, 0)), text);
}

void OpenHDBackend::handleOpenHDLinkStats(const MavlinkFrame &frame) {
    LinkState link = m_linkStateService->state();
    const QString source = sourceName(frame);

    if (frame.messageId == kOpenHDMonitorModeWifiCard && hasBytes(frame, 38)) {
        const QString section = source + QStringLiteral(" Wi-Fi card %1").arg(readU8(frame.payload, 20));
        const int quality = readI8(frame.payload, 29);
        if (quality >= 0 && quality <= 100) {
            link.linkQualityPercent = quality;
        }
        link.rssiDbm = readI8(frame.payload, 23);
        const int loss = readI8(frame.payload, 32);
        if (loss >= 0 && loss <= 100) {
            link.packetLossPercent = loss;
        }
        setMetric(QStringLiteral("link"), section, QStringLiteral("received"),
                  QStringLiteral("RX packets"), QString::number(readU32(frame.payload, 0)));
        setMetric(QStringLiteral("link"), section, QStringLiteral("injected"),
                  QStringLiteral("TX packets"), QString::number(readU32(frame.payload, 4)));
        setMetric(QStringLiteral("link"), section, QStringLiteral("quality"),
                  QStringLiteral("Signal quality"), QStringLiteral("%1 %").arg(quality));
        setMetric(QStringLiteral("link"), section, QStringLiteral("loss"),
                  QStringLiteral("Packet loss"), QStringLiteral("%1 %").arg(loss));
        setMetric(QStringLiteral("link"), section, QStringLiteral("rssi"),
                  QStringLiteral("RSSI"), QStringLiteral("%1 dBm").arg(readI8(frame.payload, 23)));
        setMetric(QStringLiteral("link"), section, QStringLiteral("rssi-antennas"),
                  QStringLiteral("Antenna RSSI"), QStringLiteral("%1 / %2 dBm").arg(readI8(frame.payload, 24)).arg(readI8(frame.payload, 25)));
        setMetric(QStringLiteral("link"), section, QStringLiteral("noise"),
                  QStringLiteral("Noise"), QStringLiteral("%1 dBm").arg(readI8(frame.payload, 26)));
        setMetric(QStringLiteral("link"), section, QStringLiteral("snr"),
                  QStringLiteral("Antenna SNR"), QStringLiteral("%1 / %2 dB").arg(readI8(frame.payload, 34)).arg(readI8(frame.payload, 35)));
        setMetric(QStringLiteral("link"), section, QStringLiteral("temperature"),
                  QStringLiteral("Card temperature"), QStringLiteral("%1 C").arg(readI8(frame.payload, 36)));
        setMetric(QStringLiteral("link"), section, QStringLiteral("tx-power"),
                  QStringLiteral("TX power"), QString::number(readI16(frame.payload, 12)));
        setMetric(QStringLiteral("link"), section, QStringLiteral("tx-power-armed"),
                  QStringLiteral("TX power armed"), QString::number(readI16(frame.payload, 14)));
        setMetric(QStringLiteral("link"), section, QStringLiteral("tx-power-disarmed"),
                  QStringLiteral("TX power disarmed"), QString::number(readI16(frame.payload, 16)));
        setMetric(QStringLiteral("link"), section, QStringLiteral("card-type"),
                  QStringLiteral("Card type"), QString::number(readU8(frame.payload, 21)));
        setMetric(QStringLiteral("link"), section, QStringLiteral("tx-active"),
                  QStringLiteral("TX active"), readU8(frame.payload, 22) != 0 ? QStringLiteral("YES") : QStringLiteral("NO"));
        setMetric(QStringLiteral("link"), section, QStringLiteral("noise-antennas"),
                  QStringLiteral("Antenna noise"), QStringLiteral("%1 / %2 dBm").arg(readI8(frame.payload, 27)).arg(readI8(frame.payload, 28)));
        setMetric(QStringLiteral("link"), section, QStringLiteral("quality-antennas"),
                  QStringLiteral("Antenna quality"), QStringLiteral("%1 / %2 %").arg(readI8(frame.payload, 30)).arg(readI8(frame.payload, 31)));
        setMetric(QStringLiteral("link"), section, QStringLiteral("driver-status"),
                  QStringLiteral("Driver status"), QString::number(readU8(frame.payload, 33)));
    } else if (frame.messageId == kOpenHDMonitorModeWifiLink && hasBytes(frame, 42)) {
        const QString section = source + QStringLiteral(" radio link");
        const int loss = readI8(frame.payload, 32);
        if (loss >= 0 && loss <= 100) {
            link.packetLossPercent = loss;
        }
        setMetric(QStringLiteral("link"), section, QStringLiteral("tx-bitrate"),
                  QStringLiteral("TX bitrate"), QStringLiteral("%1 kbps").arg(readI32(frame.payload, 0) / 1000));
        setMetric(QStringLiteral("link"), section, QStringLiteral("rx-bitrate"),
                  QStringLiteral("RX bitrate"), QStringLiteral("%1 kbps").arg(readI32(frame.payload, 4) / 1000));
        setMetric(QStringLiteral("link"), section, QStringLiteral("tx-pps"),
                  QStringLiteral("TX packets/s"), QString::number(readI16(frame.payload, 20)));
        setMetric(QStringLiteral("link"), section, QStringLiteral("rx-pps"),
                  QStringLiteral("RX packets/s"), QString::number(readI16(frame.payload, 22)));
        setMetric(QStringLiteral("link"), section, QStringLiteral("loss"),
                  QStringLiteral("RX packet loss"), QStringLiteral("%1 %").arg(loss));
        setMetric(QStringLiteral("link"), section, QStringLiteral("channel"),
                  QStringLiteral("TX channel"), QStringLiteral("%1 MHz / %2 MHz").arg(readU16(frame.payload, 26)).arg(readU8(frame.payload, 33)));
        setMetric(QStringLiteral("link"), section, QStringLiteral("rate"),
                  QStringLiteral("TX data rate"), QStringLiteral("%1 kbps (MCS %2)").arg(readU16(frame.payload, 28)).arg(readU8(frame.payload, 34)));
        setMetric(QStringLiteral("link"), section, QStringLiteral("pollution"),
                  QStringLiteral("Channel pollution"), QStringLiteral("%1 %").arg(readU8(frame.payload, 37)));
        setMetric(QStringLiteral("link"), section, QStringLiteral("gaps"),
                  QStringLiteral("RX big gaps"), QString::number(readI16(frame.payload, 24)));
        setMetric(QStringLiteral("link"), section, QStringLiteral("tx-inject-errors"),
                  QStringLiteral("TX inject error hints"), QString::number(readU32(frame.payload, 8)));
        setMetric(QStringLiteral("link"), section, QStringLiteral("tx-dropped"),
                  QStringLiteral("TX dropped packets"), QString::number(readU32(frame.payload, 12)));
        setMetric(QStringLiteral("link"), section, QStringLiteral("rate-adjustments"),
                  QStringLiteral("Rate adjustments"), QString::number(readU8(frame.payload, 35)));
        setMetric(QStringLiteral("link"), section, QStringLiteral("flags"),
                  QStringLiteral("Link flags"), QStringLiteral("0x%1").arg(readU8(frame.payload, 36), 2, 16, QLatin1Char('0')));
        setMetric(QStringLiteral("link"), section, QStringLiteral("snr"),
                  QStringLiteral("Antenna SNR"), QStringLiteral("%1 / %2 dB").arg(readI8(frame.payload, 38)).arg(readI8(frame.payload, 39)));
        setMetric(QStringLiteral("link"), section, QStringLiteral("card-temperature"),
                  QStringLiteral("Card temperature"), QStringLiteral("%1 C").arg(readI8(frame.payload, 40)));
    } else if (frame.messageId == kOpenHDTelemetryStats && hasBytes(frame, 21)) {
        const QString section = source + QStringLiteral(" telemetry");
        const int loss = readI16(frame.payload, 16);
        if (loss >= 0 && loss <= 100) {
            link.packetLossPercent = loss;
        }
        setMetric(QStringLiteral("link"), section, QStringLiteral("tx-bitrate"),
                  QStringLiteral("TX bitrate"), QStringLiteral("%1 kbps").arg(readI32(frame.payload, 0) / 1000));
        setMetric(QStringLiteral("link"), section, QStringLiteral("rx-bitrate"),
                  QStringLiteral("RX bitrate"), QStringLiteral("%1 kbps").arg(readI32(frame.payload, 4) / 1000));
        setMetric(QStringLiteral("link"), section, QStringLiteral("tx-pps"),
                  QStringLiteral("TX packets/s"), QString::number(readI16(frame.payload, 12)));
        setMetric(QStringLiteral("link"), section, QStringLiteral("rx-pps"),
                  QStringLiteral("RX packets/s"), QString::number(readI16(frame.payload, 14)));
        setMetric(QStringLiteral("link"), section, QStringLiteral("loss"),
                  QStringLiteral("RX packet loss"), QStringLiteral("%1 %").arg(loss));
    }

    m_linkStateService->updateState(link);
}

void OpenHDBackend::handleOpenHDVideoStats(const MavlinkFrame &frame) {
    if (!hasBytes(frame, 4)) {
        return;
    }

    const QString source = sourceName(frame);
    if (frame.messageId == kOpenHDVideoGroundStats && hasBytes(frame, 28)) {
        LinkState link = m_linkStateService->state();
        link.videoBitrateKbps = std::max(0, readI32(frame.payload, 0) / 1000);
        m_linkStateService->updateState(link);
        const QString section = source + QStringLiteral(" video RX %1").arg(readU8(frame.payload, 26));
        setMetric(QStringLiteral("link"), section, QStringLiteral("bitrate"),
                  QStringLiteral("Incoming bitrate"), QStringLiteral("%1 kbps").arg(link.videoBitrateKbps));
        setMetric(QStringLiteral("link"), section, QStringLiteral("blocks-total"),
                  QStringLiteral("FEC blocks total"), QString::number(readU32(frame.payload, 4)));
        setMetric(QStringLiteral("link"), section, QStringLiteral("blocks-lost"),
                  QStringLiteral("FEC blocks lost"), QString::number(readU32(frame.payload, 8)));
        setMetric(QStringLiteral("link"), section, QStringLiteral("blocks-recovered"),
                  QStringLiteral("FEC blocks recovered"), QString::number(readU32(frame.payload, 12)));
        setMetric(QStringLiteral("link"), section, QStringLiteral("fragments-recovered"),
                  QStringLiteral("Fragments recovered"), QString::number(readU32(frame.payload, 16)));
    } else if (frame.messageId == kOpenHDVideoAirStats && hasBytes(frame, 28)) {
        const QString section = source + QStringLiteral(" video TX %1").arg(readU8(frame.payload, 26));
        setMetric(QStringLiteral("link"), section, QStringLiteral("encoder-bitrate"),
                  QStringLiteral("Encoder bitrate"), QStringLiteral("%1 kbps").arg(readI32(frame.payload, 0) / 1000));
        setMetric(QStringLiteral("link"), section, QStringLiteral("injected-bitrate"),
                  QStringLiteral("Injected bitrate"), QStringLiteral("%1 kbps").arg(readI32(frame.payload, 4) / 1000));
        setMetric(QStringLiteral("link"), section, QStringLiteral("injected-pps"),
                  QStringLiteral("Injected packets/s"), QString::number(readI32(frame.payload, 8)));
        setMetric(QStringLiteral("link"), section, QStringLiteral("dropped-frames"),
                  QStringLiteral("Dropped frames"), QString::number(readI32(frame.payload, 12)));
        setMetric(QStringLiteral("link"), section, QStringLiteral("recommended-bitrate"),
                  QStringLiteral("Recommended bitrate"), QStringLiteral("%1 kbps").arg(readI16(frame.payload, 20)));
        setMetric(QStringLiteral("link"), section, QStringLiteral("fec"),
                  QStringLiteral("FEC overhead"), QStringLiteral("%1 %").arg(readI16(frame.payload, 22)));
    } else if (frame.messageId == kOpenHDVideoAirFecStats && hasBytes(frame, 32)) {
        const QString section = source + QStringLiteral(" video TX FEC");
        setMetric(QStringLiteral("link"), section, QStringLiteral("encode-average"),
                  QStringLiteral("Encode average"), QStringLiteral("%1 us").arg(readU32(frame.payload, 0)));
        setMetric(QStringLiteral("link"), section, QStringLiteral("encode-range"),
                  QStringLiteral("Encode min / max"), QStringLiteral("%1 / %2 us").arg(readU32(frame.payload, 4)).arg(readU32(frame.payload, 8)));
        setMetric(QStringLiteral("link"), section, QStringLiteral("block-size"),
                  QStringLiteral("FEC block avg / min / max"), QStringLiteral("%1 / %2 / %3")
                                                           .arg(readU16(frame.payload, 16))
                                                           .arg(readU16(frame.payload, 18))
                                                           .arg(readU16(frame.payload, 20)));
        setMetric(QStringLiteral("link"), section, QStringLiteral("tx-delay"),
                  QStringLiteral("TX delay avg / min / max"), QStringLiteral("%1 / %2 / %3 us")
                                                            .arg(readU16(frame.payload, 22))
                                                            .arg(readU16(frame.payload, 24))
                                                            .arg(readU16(frame.payload, 26)));
    } else if (frame.messageId == kOpenHDVideoGroundFecStats && hasBytes(frame, 20)) {
        const QString section = source + QStringLiteral(" video RX FEC");
        setMetric(QStringLiteral("link"), section, QStringLiteral("decode-average"),
                  QStringLiteral("Decode average"), QStringLiteral("%1 us").arg(readU32(frame.payload, 0)));
        setMetric(QStringLiteral("link"), section, QStringLiteral("decode-range"),
                  QStringLiteral("Decode min / max"), QStringLiteral("%1 / %2 us").arg(readU32(frame.payload, 4)).arg(readU32(frame.payload, 8)));
    }
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

    const QString section = sourceName(frame) + QStringLiteral(" camera %1").arg(frame.componentId);
    setMetric(QStringLiteral("system"), section, QStringLiteral("status"),
              QStringLiteral("Camera status"), status == 1 ? QStringLiteral("STREAMING")
                                                       : status == 2 ? QStringLiteral("RESTARTING")
                                                                     : QStringLiteral("UNKNOWN"));
    setMetric(QStringLiteral("system"), section, QStringLiteral("camera-type"),
              QStringLiteral("Camera type"), QString::number(readU8(frame.payload, 14)));
    setMetric(QStringLiteral("system"), section, QStringLiteral("resolution"),
              QStringLiteral("Resolution"), QStringLiteral("%1 x %2 @ %3 fps")
                                                 .arg(video.streamWidth)
                                                 .arg(video.streamHeight)
                                                 .arg(video.streamFps));
    setMetric(QStringLiteral("system"), section, QStringLiteral("codec"),
              QStringLiteral("Codec"), video.codec);
    setMetric(QStringLiteral("system"), section, QStringLiteral("bitrate"),
              QStringLiteral("Configured bitrate"), QStringLiteral("%1 kbps").arg(readU16(frame.payload, 4)));
    setMetric(QStringLiteral("system"), section, QStringLiteral("recording"),
              QStringLiteral("Air recording"), video.recording ? QStringLiteral("ACTIVE") : QStringLiteral("IDLE"));
    setMetric(QStringLiteral("system"), section, QStringLiteral("variable-bitrate"),
              QStringLiteral("Variable bitrate"), readU8(frame.payload, 16) != 0 ? QStringLiteral("SUPPORTED") : QStringLiteral("NO"));
    setMetric(QStringLiteral("system"), section, QStringLiteral("keyframe-interval"),
              QStringLiteral("Keyframe interval"), QString::number(readU8(frame.payload, 19)));

    if (m_videoFrameProvider != nullptr && m_rtpVideoReceiver.codec() != codec) {
        VideoState restartingVideo = m_videoService->state();
        restartingVideo.frameAvailable = false;
        restartingVideo.videoAvailable = false;
        restartingVideo.decoderState = QStringLiteral("Restarting RTP decoder");
        m_videoService->updateState(restartingVideo);
        m_rtpVideoReceiver.start(codec);
    }
}

void OpenHDBackend::handleOpenHDSystemStatus(const MavlinkFrame &frame) {
    const QString source = sourceName(frame);
    if (frame.messageId == kOpenHDCoreStatus && hasBytes(frame, 22)) {
        const QString section = source + QStringLiteral(" core");
        setMetric(QStringLiteral("system"), section, QStringLiteral("storage"),
                  QStringLiteral("Storage left"), QStringLiteral("%1 MiB").arg(readU32(frame.payload, 0)));
        setMetric(QStringLiteral("system"), section, QStringLiteral("cpu-clock"),
                  QStringLiteral("CPU clock"), QStringLiteral("%1 MHz").arg(readU16(frame.payload, 4)));
        setMetric(QStringLiteral("system"), section, QStringLiteral("gpu-clocks"),
                  QStringLiteral("ISP / H264 / Core / V3D"), QStringLiteral("%1 / %2 / %3 / %4 MHz")
                                                        .arg(readU16(frame.payload, 6))
                                                        .arg(readU16(frame.payload, 8))
                                                        .arg(readU16(frame.payload, 10))
                                                        .arg(readU16(frame.payload, 12)));
        setMetric(QStringLiteral("system"), section, QStringLiteral("cpu-temp"),
                  QStringLiteral("CPU temperature"), QStringLiteral("%1 C").arg(readI8(frame.payload, 14)));
        setMetric(QStringLiteral("system"), section, QStringLiteral("wifi-temp"),
                  QStringLiteral("Wi-Fi temperatures"), QStringLiteral("%1 / %2 C").arg(readI8(frame.payload, 15)).arg(readI8(frame.payload, 16)));
        setMetric(QStringLiteral("system"), section, QStringLiteral("undervolt"),
                  QStringLiteral("Undervolt"), readU8(frame.payload, 17) != 0 ? QStringLiteral("DETECTED") : QStringLiteral("CLEAR"));
        setMetric(QStringLiteral("system"), section, QStringLiteral("encryption"),
                  QStringLiteral("Encryption"), readU8(frame.payload, 19) != 0 ? QStringLiteral("ENABLED") : QStringLiteral("DISABLED"));
        setMetric(QStringLiteral("system"), section, QStringLiteral("fc-system"),
                  QStringLiteral("Configured FC system"), QString::number(readU8(frame.payload, 20)));
        setMetric(QStringLiteral("system"), section, QStringLiteral("platform"),
                  QStringLiteral("Platform type"), QString::number(readU8(frame.payload, 18)));
        setMetric(QStringLiteral("system"), section, QStringLiteral("operating-mode"),
                  QStringLiteral("Operating mode"), QString::number(readU8(frame.payload, 21)));
    } else if (frame.messageId == kOpenHDSysStatus && hasBytes(frame, 15)) {
        const QString section = source + QStringLiteral(" networking");
        setMetric(QStringLiteral("system"), section, QStringLiteral("hotspot"),
                  QStringLiteral("Wi-Fi hotspot state"), QString::number(readU8(frame.payload, 12)));
        setMetric(QStringLiteral("system"), section, QStringLiteral("hotspot-frequency"),
                  QStringLiteral("Hotspot frequency"), QStringLiteral("%1 MHz").arg(readU16(frame.payload, 8)));
        setMetric(QStringLiteral("system"), section, QStringLiteral("ethernet"),
                  QStringLiteral("Ethernet hotspot state"), QString::number(readU8(frame.payload, 13)));
        setMetric(QStringLiteral("system"), section, QStringLiteral("external-devices"),
                  QStringLiteral("External devices"), QString::number(readU8(frame.payload, 11)));
        setMetric(QStringLiteral("system"), section, QStringLiteral("platform"),
                  QStringLiteral("Platform type"), QString::number(readU8(frame.payload, 10)));
    } else if (frame.messageId == kOpenHDPowerStatus && hasBytes(frame, 6)) {
        const QString section = source + QStringLiteral(" power");
        setMetric(QStringLiteral("system"), section, QStringLiteral("voltage"),
                  QStringLiteral("Voltage"), QStringLiteral("%1 V").arg(readU16(frame.payload, 0) / 1000.0, 0, 'f', 2));
        setMetric(QStringLiteral("system"), section, QStringLiteral("current"),
                  QStringLiteral("Current"), QStringLiteral("%1 mA").arg(readI16(frame.payload, 2)));
        setMetric(QStringLiteral("system"), section, QStringLiteral("battery"),
                  QStringLiteral("Battery"), QStringLiteral("%1 %").arg(readU8(frame.payload, 5)));
        setMetric(QStringLiteral("system"), section, QStringLiteral("charging"),
                  QStringLiteral("Charging state"), QString::number(readU8(frame.payload, 4)));
    } else if (frame.messageId == kOpenHDOnboardComputerStatusExtension && hasBytes(frame, 11)) {
        const QString section = source + QStringLiteral(" core");
        setMetric(QStringLiteral("system"), section, QStringLiteral("core-voltage"),
                  QStringLiteral("CPU core voltage"), QStringLiteral("%1 mV").arg(readU16(frame.payload, 0)));
        setMetric(QStringLiteral("system"), section, QStringLiteral("over-current"),
                  QStringLiteral("Over-current"), readU8(frame.payload, 10) != 0 ? QStringLiteral("DETECTED") : QStringLiteral("CLEAR"));
    } else if (frame.messageId == kOpenHDVersionMessage && hasBytes(frame, 5)) {
        QString releaseType;
        switch (readU8(frame.payload, 3)) {
        case 0:
            releaseType = QStringLiteral("release");
            break;
        case 1:
            releaseType = QStringLiteral("beta");
            break;
        case 2:
            releaseType = QStringLiteral("alpha");
            break;
        default:
            releaseType = QStringLiteral("development");
            break;
        }
        setMetric(QStringLiteral("system"), source, QStringLiteral("version"),
                  QStringLiteral("OpenHD version"), QStringLiteral("%1.%2.%3 (%4)")
                                                   .arg(readU8(frame.payload, 0))
                                                   .arg(readU8(frame.payload, 1))
                                                   .arg(readU8(frame.payload, 2))
                                                   .arg(releaseType));
    } else if (frame.messageId == kOpenHDGroundOperatingMode && hasBytes(frame, 12)) {
        const QString section = source + QStringLiteral(" radio mode");
        QString mode;
        switch (readU8(frame.payload, 10)) {
        case 0:
            mode = QStringLiteral("NORMAL");
            break;
        case 1:
            mode = QStringLiteral("SCANNING");
            break;
        case 2:
            mode = QStringLiteral("ANALYZING");
            break;
        default:
            mode = QStringLiteral("UNKNOWN");
            break;
        }
        setMetric(QStringLiteral("system"), section, QStringLiteral("operating-mode"),
                  QStringLiteral("Operating mode"), mode);
        setMetric(QStringLiteral("system"), section, QStringLiteral("passive"),
                  QStringLiteral("Passive RX mode"), readI8(frame.payload, 11) != 0 ? QStringLiteral("ENABLED") : QStringLiteral("DISABLED"));
    } else if (frame.messageId == kOpenHDMicrohardStatus && hasBytes(frame, 10)) {
        const QString section = source + QStringLiteral(" Microhard");
        setMetric(QStringLiteral("system"), section, QStringLiteral("enabled"),
                  QStringLiteral("Enabled"), readU8(frame.payload, 4) != 0 ? QStringLiteral("YES") : QStringLiteral("NO"));
        setMetric(QStringLiteral("system"), section, QStringLiteral("frequency"),
                  QStringLiteral("Frequency"), QStringLiteral("%1 Hz").arg(readU32(frame.payload, 0)));
        setMetric(QStringLiteral("system"), section, QStringLiteral("signal"),
                  QStringLiteral("RSSI / Noise / SNR"), QStringLiteral("%1 / %2 / %3").arg(readI8(frame.payload, 5)).arg(readI8(frame.payload, 8)).arg(readU8(frame.payload, 9)));
        setMetric(QStringLiteral("system"), section, QStringLiteral("tx-power"),
                  QStringLiteral("TX power"), QString::number(readU8(frame.payload, 6)));
        setMetric(QStringLiteral("system"), section, QStringLiteral("bandwidth"),
                  QStringLiteral("Bandwidth"), QString::number(readU8(frame.payload, 7)));
    } else if (frame.messageId == kOpenHDSupportedChannels && hasBytes(frame, 181)) {
        QStringList channels;
        QStringList bandwidths;
        for (int index = 0; index < 60; ++index) {
            const quint16 channel = readU16(frame.payload, index * 2);
            if (channel != 0) {
                channels.push_back(QStringLiteral("%1 MHz").arg(channel));
                bandwidths.push_back(QStringLiteral("%1 MHz: %2 MHz")
                                         .arg(channel)
                                         .arg(readU8(frame.payload, 121 + index)));
            }
        }
        const QString section = source + QStringLiteral(" radio card %1").arg(readU8(frame.payload, 120));
        setMetric(QStringLiteral("system"), section, QStringLiteral("supported-channels"),
                  QStringLiteral("Supported channels"), channels.join(QStringLiteral(", ")));
        setMetric(QStringLiteral("system"), section, QStringLiteral("channel-bandwidths"),
                  QStringLiteral("Channel bandwidths"), bandwidths.join(QStringLiteral(", ")));
    } else if (frame.messageId == kOpenHDScanChannelsProgress && hasBytes(frame, 5)) {
        setMetric(QStringLiteral("system"), source + QStringLiteral(" radio scan"), QStringLiteral("progress"),
                  QStringLiteral("Scan progress"), QStringLiteral("%1 % at %2 MHz / %3 MHz, Air %4")
                                                  .arg(readU8(frame.payload, 2))
                                                  .arg(readU16(frame.payload, 0))
                                                  .arg(readU8(frame.payload, 3))
                                                  .arg(readU8(frame.payload, 4) != 0 ? QStringLiteral("found") : QStringLiteral("not found")));
    } else if (frame.messageId == kOpenHDAnalyzeChannelsProgress && hasBytes(frame, 191)) {
        const QString section = source + QStringLiteral(" channel analysis");
        setMetric(QStringLiteral("system"), section, QStringLiteral("progress"),
                  QStringLiteral("Analysis progress"), QStringLiteral("%1 %").arg(readI8(frame.payload, 190)));
        for (int index = 0; index < 30; ++index) {
            const quint16 channel = readU16(frame.payload, 8 + index * 2);
            if (channel == 0) {
                continue;
            }
            setMetric(QStringLiteral("system"), section, QStringLiteral("foreign-%1").arg(channel),
                      QStringLiteral("%1 MHz foreign packets").arg(channel),
                      QString::number(readU16(frame.payload, 68 + index * 2)));
        }
    } else if (frame.messageId == kOpenHDRtspConfiguration && !frame.payload.isEmpty()) {
        const QString rtsp = QString::fromLatin1(frame.payload).split(QLatin1Char('\0')).front();
        setMetric(QStringLiteral("system"), source + QStringLiteral(" video"), QStringLiteral("rtsp"),
                  QStringLiteral("RTSP configuration"), rtsp);
    }
}

void OpenHDBackend::setMetric(const QString &group,
                              const QString &section,
                              const QString &id,
                              const QString &label,
                              const QString &value,
                              const QString &detail) {
    if (m_diagnosticsService != nullptr) {
        m_diagnosticsService->setMetric(group, section, id, label, value, detail);
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
        link.rssiDbm = -127;
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
