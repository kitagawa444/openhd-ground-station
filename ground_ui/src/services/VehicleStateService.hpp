#pragma once

#include <QObject>

#include "common/types.hpp"

namespace openhd {

class VehicleStateService : public QObject {
    Q_OBJECT

public:
    explicit VehicleStateService(QObject *parent = nullptr);

    const VehicleState &state() const;
    void updateState(const VehicleState &state);
    void setArmed(bool armed);
    void setFlightMode(const QString &flightMode);

signals:
    void stateChanged();

private:
    VehicleState m_state;
};

}  // namespace openhd
