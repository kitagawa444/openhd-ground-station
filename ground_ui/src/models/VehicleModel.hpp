#pragma once

#include <QObject>

namespace openhd {

class VehicleStateService;

class VehicleModel : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool connected READ connected NOTIFY changed)
    Q_PROPERTY(bool armed READ armed NOTIFY changed)
    Q_PROPERTY(QString flightMode READ flightMode NOTIFY changed)
    Q_PROPERTY(bool positionValid READ positionValid NOTIFY changed)
    Q_PROPERTY(bool batteryValid READ batteryValid NOTIFY changed)
    Q_PROPERTY(bool homePositionValid READ homePositionValid NOTIFY changed)
    Q_PROPERTY(double latitude READ latitude NOTIFY changed)
    Q_PROPERTY(double longitude READ longitude NOTIFY changed)
    Q_PROPERTY(double relativeAltitude READ relativeAltitude NOTIFY changed)
    Q_PROPERTY(double groundSpeed READ groundSpeed NOTIFY changed)
    Q_PROPERTY(double verticalSpeed READ verticalSpeed NOTIFY changed)
    Q_PROPERTY(double rollDegrees READ rollDegrees NOTIFY changed)
    Q_PROPERTY(double pitchDegrees READ pitchDegrees NOTIFY changed)
    Q_PROPERTY(double yawDegrees READ yawDegrees NOTIFY changed)
    Q_PROPERTY(int batteryPercent READ batteryPercent NOTIFY changed)
    Q_PROPERTY(double batteryVoltage READ batteryVoltage NOTIFY changed)
    Q_PROPERTY(int satellites READ satellites NOTIFY changed)
    Q_PROPERTY(double headingDegrees READ headingDegrees NOTIFY changed)
    Q_PROPERTY(double homeLatitude READ homeLatitude NOTIFY changed)
    Q_PROPERTY(double homeLongitude READ homeLongitude NOTIFY changed)
    Q_PROPERTY(double homeDistanceMeters READ homeDistanceMeters NOTIFY changed)

public:
    VehicleModel(VehicleStateService *service, QObject *parent = nullptr);

    bool connected() const;
    bool armed() const;
    QString flightMode() const;
    bool positionValid() const;
    bool batteryValid() const;
    bool homePositionValid() const;
    double latitude() const;
    double longitude() const;
    double relativeAltitude() const;
    double groundSpeed() const;
    double verticalSpeed() const;
    double rollDegrees() const;
    double pitchDegrees() const;
    double yawDegrees() const;
    int batteryPercent() const;
    double batteryVoltage() const;
    int satellites() const;
    double headingDegrees() const;
    double homeLatitude() const;
    double homeLongitude() const;
    double homeDistanceMeters() const;

signals:
    void changed();

private:
    VehicleStateService *m_service;
};

}  // namespace openhd
