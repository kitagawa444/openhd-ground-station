#pragma once

#include <QElapsedTimer>
#include <QObject>
#include <QTcpSocket>
#include <QTimer>

#include "integration/MavlinkParser.hpp"
#include "video/RtpVideoReceiver.hpp"

namespace openhd {

class AlertService;
class LinkStateService;
class VehicleStateService;
class VideoService;
class VideoFrameProvider;

class OpenHDBackend : public QObject {
    Q_OBJECT

public:
    static constexpr quint16 DefaultTelemetryTcpPort = 5760;

    OpenHDBackend(VehicleStateService *vehicleStateService,
                  LinkStateService *linkStateService,
                  VideoService *videoService,
                  AlertService *alertService,
                  VideoFrameProvider *videoFrameProvider,
                  QObject *parent = nullptr,
                  quint16 telemetryTcpPort = DefaultTelemetryTcpPort);

    void start();
    void reconnect();

private slots:
    void connectToGround();
    void onTelemetryReadyRead();
    void refreshHealth();

private:
    void handleFrame(const MavlinkFrame &frame);
    void handleHeartbeat(const MavlinkFrame &frame);
    void handleSystemStatus(const MavlinkFrame &frame);
    void handleBatteryStatus(const MavlinkFrame &frame);
    void handleGlobalPosition(const MavlinkFrame &frame);
    void handleGpsRaw(const MavlinkFrame &frame);
    void handleVfrHud(const MavlinkFrame &frame);
    void handleHomePosition(const MavlinkFrame &frame);
    void handleOpenHDLinkStats(const MavlinkFrame &frame);
    void handleOpenHDVideoStats(const MavlinkFrame &frame);
    void handleOpenHDCameraStatus(const MavlinkFrame &frame);
    void refreshAlerts();
    void setBackendConnected(bool connected);
    bool isFlightControllerFrame(const MavlinkFrame &frame) const;

    VehicleStateService *m_vehicleStateService;
    LinkStateService *m_linkStateService;
    VideoService *m_videoService;
    AlertService *m_alertService;
    VideoFrameProvider *m_videoFrameProvider;
    quint16 m_telemetryTcpPort;
    QTcpSocket m_telemetrySocket;
    QTimer m_reconnectTimer;
    QTimer m_healthTimer;
    MavlinkParser m_parser;
    QElapsedTimer m_telemetryAge;
    QElapsedTimer m_videoAge;
    bool m_seenFlightTelemetry = false;
    RtpVideoReceiver m_rtpVideoReceiver;
};

}  // namespace openhd
