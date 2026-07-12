#include "services/DemoDataService.hpp"

#include <algorithm>
#include <cmath>

#include "services/AlertService.hpp"
#include "services/LinkStateService.hpp"
#include "services/VehicleStateService.hpp"
#include "services/VideoService.hpp"

namespace openhd {

namespace {

constexpr double kPi = 3.14159265358979323846;

}  // namespace

DemoDataService::DemoDataService(VehicleStateService *vehicleStateService,
                                 LinkStateService *linkStateService,
                                 VideoService *videoService,
                                 AlertService *alertService,
                                 QObject *parent)
    : QObject(parent),
      m_vehicleStateService(vehicleStateService),
      m_linkStateService(linkStateService),
      m_videoService(videoService),
      m_alertService(alertService) {
    connect(&m_timer, &QTimer::timeout, this, &DemoDataService::refresh);
}

void DemoDataService::start() {
    m_timer.start(850);
    refresh();
}

void DemoDataService::refresh() {
    m_phase += 0.22;

    VehicleState vehicle = m_vehicleStateService->state();
    vehicle.connected = true;
    vehicle.latitude = 35.623600 + 0.0018 * std::sin(m_phase * 0.70);
    vehicle.longitude = 139.776800 + 0.0022 * std::cos(m_phase * 0.55);
    vehicle.relativeAltitude = 43.0 + 8.0 * std::sin(m_phase * 0.65);
    vehicle.groundSpeed = 11.0 + 4.2 * (0.5 + 0.5 * std::sin(m_phase * 0.9));
    vehicle.verticalSpeed = 1.8 * std::cos(m_phase * 0.75);
    vehicle.headingDegrees = std::fmod(vehicle.headingDegrees + 3.6, 360.0);
    vehicle.homeDistanceMeters = 58.0 + 18.0 * (0.5 + 0.5 * std::sin(m_phase * 0.33 + kPi / 4.0));
    vehicle.satellites = 17 + static_cast<int>(4.0 * (0.5 + 0.5 * std::sin(m_phase * 0.5)));

    const int drainedPercent = std::clamp(92 - static_cast<int>(m_phase * 0.55), 28, 92);
    vehicle.batteryPercent = drainedPercent;
    vehicle.batteryVoltage = 16.2 - static_cast<double>(92 - drainedPercent) * 0.028;
    vehicle.positionValid = true;
    vehicle.batteryValid = true;
    vehicle.homePositionValid = true;
    m_vehicleStateService->updateState(vehicle);

    LinkState link = m_linkStateService->state();
    if (link.linkConnected) {
        link.linkQualityPercent = 82 + static_cast<int>(14.0 * (0.5 + 0.5 * std::sin(m_phase * 0.8)));
        link.packetLossPercent = 0.2 + 1.8 * (0.5 + 0.5 * std::sin(m_phase * 1.2 + 0.6));
        link.videoBitrateKbps = 9800 + static_cast<int>(3800.0 * (0.5 + 0.5 * std::sin(m_phase * 0.95)));
        link.latencyMs = 26 + static_cast<int>(18.0 * (0.5 + 0.5 * std::sin(m_phase * 1.1)));
        link.telemetryHealthy = true;
        link.videoHealthy = true;
        link.rcConnected = true;
    } else {
        link.linkQualityPercent = 0;
        link.packetLossPercent = 100.0;
        link.videoBitrateKbps = 0;
        link.latencyMs = 0;
        link.telemetryHealthy = false;
        link.videoHealthy = false;
        link.rcConnected = false;
    }
    link.backendConnected = true;
    m_linkStateService->updateState(link);

    VideoState video = m_videoService->state();
    video.videoAvailable = link.linkConnected;
    video.streamWidth = 1920;
    video.streamHeight = 1080;
    video.lastFrameAgeMs = link.linkConnected
                               ? 16 + static_cast<int>(14.0 * (0.5 + 0.5 * std::sin(m_phase * 1.35)))
                               : 999;

    if (!link.linkConnected) {
        video.decoderState = QStringLiteral("Waiting");
    } else if (video.decoderState != QStringLiteral("Restarting")) {
        video.decoderState = QStringLiteral("Stable");
    }
    m_videoService->updateState(video);

    QVector<AlertItem> alerts;
    if (!link.linkConnected) {
        alerts.push_back(makeAlert(QStringLiteral("link-lost"),
                                   AlertSeverity::Critical,
                                   QStringLiteral("Link lost"),
                                   QStringLiteral("Ground backend is waiting for the wireless link."),
                                   true));
    } else if (link.linkQualityPercent < 86) {
        alerts.push_back(makeAlert(QStringLiteral("link-weak"),
                                   AlertSeverity::Caution,
                                   QStringLiteral("Link margin narrowing"),
                                   QStringLiteral("Packet delivery is still healthy, but quality is dropping.")));
    }

    if (vehicle.batteryPercent < 35) {
        alerts.push_back(makeAlert(QStringLiteral("battery-low"),
                                   AlertSeverity::Caution,
                                   QStringLiteral("Battery reserve low"),
                                   QStringLiteral("Prepare for return or landing soon.")));
    }

    if (video.recording) {
        alerts.push_back(makeAlert(QStringLiteral("recording"),
                                   AlertSeverity::Info,
                                   QStringLiteral("Recording"),
                                   QStringLiteral("Demo recording flag is active."),
                                   true));
    }

    m_alertService->setSystemAlerts(alerts);
}

AlertItem DemoDataService::makeAlert(const QString &id,
                                     const AlertSeverity severity,
                                     const QString &title,
                                     const QString &message,
                                     const bool sticky) const {
    AlertItem alert;
    alert.id = id;
    alert.severity = severity;
    alert.title = title;
    alert.message = message;
    alert.sticky = sticky;
    alert.timestamp = QDateTime::currentDateTimeUtc();
    return alert;
}

}  // namespace openhd
