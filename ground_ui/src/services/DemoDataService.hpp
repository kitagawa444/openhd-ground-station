#pragma once

#include <QObject>
#include <QTimer>

#include "common/types.hpp"

namespace openhd {

class VehicleStateService;
class LinkStateService;
class VideoService;
class AlertService;

class DemoDataService : public QObject {
    Q_OBJECT

public:
    DemoDataService(VehicleStateService *vehicleStateService,
                    LinkStateService *linkStateService,
                    VideoService *videoService,
                    AlertService *alertService,
                    QObject *parent = nullptr);

    void start();

private slots:
    void refresh();

private:
    AlertItem makeAlert(const QString &id,
                        AlertSeverity severity,
                        const QString &title,
                        const QString &message,
                        bool sticky = false) const;

    VehicleStateService *m_vehicleStateService;
    LinkStateService *m_linkStateService;
    VideoService *m_videoService;
    AlertService *m_alertService;
    QTimer m_timer;
    double m_phase = 0.0;
};

}  // namespace openhd
