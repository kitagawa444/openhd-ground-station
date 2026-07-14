#include "app/AppContext.hpp"

#include <QCoreApplication>

namespace openhd {

AppContext::AppContext(QObject *parent)
    : QObject(parent),
      m_demoMode(QCoreApplication::arguments().contains(QStringLiteral("--demo"))),
      m_vehicleStateService(this),
      m_linkStateService(this),
      m_videoService(this),
      m_alertService(this),
      m_diagnosticsService(this),
      m_vehicleModel(&m_vehicleStateService, this),
      m_linkModel(&m_linkStateService, this),
      m_videoModel(&m_videoService, this),
      m_alertListModel(&m_alertService, this),
      m_diagnosticsModel(&m_diagnosticsService, this),
      m_flightCommands(&m_vehicleStateService, &m_alertService, m_demoMode, this),
      m_cameraCommands(&m_videoService, &m_alertService, m_demoMode, this),
      m_systemCommands(&m_linkStateService, &m_videoService, &m_alertService, m_demoMode, this),
      m_demoDataService(&m_vehicleStateService, &m_linkStateService, &m_videoService, &m_alertService, this),
      m_videoFrameProvider(std::make_unique<VideoFrameProvider>()),
      m_openHDBackend(&m_vehicleStateService,
                      &m_linkStateService,
                      &m_videoService,
                      &m_alertService,
                      &m_diagnosticsService,
                      m_videoFrameProvider.get(),
                      this) {
}

void AppContext::start() {
    if (m_demoMode) {
        m_demoDataService.start();
    } else {
        m_openHDBackend.start();
    }
}

VehicleModel *AppContext::vehicleModel() {
    return &m_vehicleModel;
}

LinkModel *AppContext::linkModel() {
    return &m_linkModel;
}

VideoModel *AppContext::videoModel() {
    return &m_videoModel;
}

AlertListModel *AppContext::alertListModel() {
    return &m_alertListModel;
}

DiagnosticsModel *AppContext::diagnosticsModel() {
    return &m_diagnosticsModel;
}

FlightCommands *AppContext::flightCommands() {
    return &m_flightCommands;
}

CameraCommands *AppContext::cameraCommands() {
    return &m_cameraCommands;
}

SystemCommands *AppContext::systemCommands() {
    return &m_systemCommands;
}

bool AppContext::isDemoMode() const {
    return m_demoMode;
}

std::unique_ptr<VideoFrameProvider> AppContext::takeVideoFrameProvider() {
    return std::move(m_videoFrameProvider);
}

}  // namespace openhd
