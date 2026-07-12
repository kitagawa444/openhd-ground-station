#pragma once

#include <memory>

#include <QObject>

#include "commands/CameraCommands.hpp"
#include "commands/FlightCommands.hpp"
#include "commands/SystemCommands.hpp"
#include "models/AlertListModel.hpp"
#include "models/LinkModel.hpp"
#include "models/VehicleModel.hpp"
#include "models/VideoModel.hpp"
#include "integration/OpenHDBackend.hpp"
#include "services/AlertService.hpp"
#include "services/DemoDataService.hpp"
#include "services/LinkStateService.hpp"
#include "services/VehicleStateService.hpp"
#include "services/VideoService.hpp"
#include "video/VideoFrameProvider.hpp"

namespace openhd {

class AppContext : public QObject {
    Q_OBJECT

public:
    explicit AppContext(QObject *parent = nullptr);

    VehicleModel *vehicleModel();
    LinkModel *linkModel();
    VideoModel *videoModel();
    AlertListModel *alertListModel();
    FlightCommands *flightCommands();
    CameraCommands *cameraCommands();
    SystemCommands *systemCommands();
    bool isDemoMode() const;
    void start();
    std::unique_ptr<VideoFrameProvider> takeVideoFrameProvider();

private:
    const bool m_demoMode;
    VehicleStateService m_vehicleStateService;
    LinkStateService m_linkStateService;
    VideoService m_videoService;
    AlertService m_alertService;
    VehicleModel m_vehicleModel;
    LinkModel m_linkModel;
    VideoModel m_videoModel;
    AlertListModel m_alertListModel;
    FlightCommands m_flightCommands;
    CameraCommands m_cameraCommands;
    SystemCommands m_systemCommands;
    DemoDataService m_demoDataService;
    std::unique_ptr<VideoFrameProvider> m_videoFrameProvider;
    OpenHDBackend m_openHDBackend;
};

}  // namespace openhd
