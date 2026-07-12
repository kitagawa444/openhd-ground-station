#include "models/LinkModel.hpp"

#include "services/LinkStateService.hpp"

namespace openhd {

LinkModel::LinkModel(LinkStateService *service, QObject *parent)
    : QObject(parent),
      m_service(service) {
    connect(m_service, &LinkStateService::stateChanged, this, &LinkModel::changed);
}

bool LinkModel::backendConnected() const {
    return m_service->state().backendConnected;
}

bool LinkModel::linkConnected() const {
    return m_service->state().linkConnected;
}

int LinkModel::linkQualityPercent() const {
    return m_service->state().linkQualityPercent;
}

bool LinkModel::telemetryHealthy() const {
    return m_service->state().telemetryHealthy;
}

bool LinkModel::videoHealthy() const {
    return m_service->state().videoHealthy;
}

int LinkModel::videoBitrateKbps() const {
    return m_service->state().videoBitrateKbps;
}

double LinkModel::packetLossPercent() const {
    return m_service->state().packetLossPercent;
}

int LinkModel::latencyMs() const {
    return m_service->state().latencyMs;
}

bool LinkModel::rcConnected() const {
    return m_service->state().rcConnected;
}

}  // namespace openhd
