#include "models/VehicleModel.hpp"

#include "services/VehicleStateService.hpp"

namespace openhd {

VehicleModel::VehicleModel(VehicleStateService *service, QObject *parent)
    : QObject(parent),
      m_service(service) {
    connect(m_service, &VehicleStateService::stateChanged, this, &VehicleModel::changed);
}

bool VehicleModel::connected() const {
    return m_service->state().connected;
}

bool VehicleModel::armed() const {
    return m_service->state().armed;
}

QString VehicleModel::flightMode() const {
    return m_service->state().flightMode;
}

bool VehicleModel::positionValid() const {
    return m_service->state().positionValid;
}

bool VehicleModel::batteryValid() const {
    return m_service->state().batteryValid;
}

bool VehicleModel::homePositionValid() const {
    return m_service->state().homePositionValid;
}

double VehicleModel::latitude() const {
    return m_service->state().latitude;
}

double VehicleModel::longitude() const {
    return m_service->state().longitude;
}

double VehicleModel::relativeAltitude() const {
    return m_service->state().relativeAltitude;
}

double VehicleModel::groundSpeed() const {
    return m_service->state().groundSpeed;
}

double VehicleModel::verticalSpeed() const {
    return m_service->state().verticalSpeed;
}

int VehicleModel::batteryPercent() const {
    return m_service->state().batteryPercent;
}

double VehicleModel::batteryVoltage() const {
    return m_service->state().batteryVoltage;
}

int VehicleModel::satellites() const {
    return m_service->state().satellites;
}

double VehicleModel::headingDegrees() const {
    return m_service->state().headingDegrees;
}

double VehicleModel::homeLatitude() const {
    return m_service->state().homeLatitude;
}

double VehicleModel::homeLongitude() const {
    return m_service->state().homeLongitude;
}

double VehicleModel::homeDistanceMeters() const {
    return m_service->state().homeDistanceMeters;
}

}  // namespace openhd
