#pragma once

#include <QObject>

namespace openhd {

class LinkStateService;

class LinkModel : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool backendConnected READ backendConnected NOTIFY changed)
    Q_PROPERTY(bool linkConnected READ linkConnected NOTIFY changed)
    Q_PROPERTY(int linkQualityPercent READ linkQualityPercent NOTIFY changed)
    Q_PROPERTY(bool telemetryHealthy READ telemetryHealthy NOTIFY changed)
    Q_PROPERTY(bool videoHealthy READ videoHealthy NOTIFY changed)
    Q_PROPERTY(int videoBitrateKbps READ videoBitrateKbps NOTIFY changed)
    Q_PROPERTY(double packetLossPercent READ packetLossPercent NOTIFY changed)
    Q_PROPERTY(int latencyMs READ latencyMs NOTIFY changed)
    Q_PROPERTY(int rssiDbm READ rssiDbm NOTIFY changed)
    Q_PROPERTY(bool rcConnected READ rcConnected NOTIFY changed)

public:
    LinkModel(LinkStateService *service, QObject *parent = nullptr);

    bool backendConnected() const;
    bool linkConnected() const;
    int linkQualityPercent() const;
    bool telemetryHealthy() const;
    bool videoHealthy() const;
    int videoBitrateKbps() const;
    double packetLossPercent() const;
    int latencyMs() const;
    int rssiDbm() const;
    bool rcConnected() const;

signals:
    void changed();

private:
    LinkStateService *m_service;
};

}  // namespace openhd
