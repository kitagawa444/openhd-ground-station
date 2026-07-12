#include "services/VehicleStateService.hpp"

namespace openhd {

VehicleStateService::VehicleStateService(QObject *parent)
    : QObject(parent) {}

const VehicleState &VehicleStateService::state() const {
    return m_state;
}

void VehicleStateService::updateState(const VehicleState &state) {
    m_state = state;
    emit stateChanged();
}

void VehicleStateService::setArmed(const bool armed) {
    if (m_state.armed == armed) {
        return;
    }

    m_state.armed = armed;
    emit stateChanged();
}

void VehicleStateService::setFlightMode(const QString &flightMode) {
    if (m_state.flightMode == flightMode) {
        return;
    }

    m_state.flightMode = flightMode;
    emit stateChanged();
}

}  // namespace openhd
