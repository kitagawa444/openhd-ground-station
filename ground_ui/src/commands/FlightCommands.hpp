#pragma once

#include <QObject>

namespace openhd {

class VehicleStateService;
class AlertService;

class FlightCommands : public QObject {
    Q_OBJECT

public:
    FlightCommands(VehicleStateService *vehicleStateService,
                   AlertService *alertService,
                   bool demoMode,
                   QObject *parent = nullptr);

    Q_INVOKABLE void arm();
    Q_INVOKABLE void disarm();
    Q_INVOKABLE void cycleFlightMode();

private:
    VehicleStateService *m_vehicleStateService;
    AlertService *m_alertService;
    bool m_demoMode;
};

}  // namespace openhd
