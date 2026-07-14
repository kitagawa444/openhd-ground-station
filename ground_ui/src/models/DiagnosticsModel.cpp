#include "models/DiagnosticsModel.hpp"

#include "services/DiagnosticsService.hpp"

namespace openhd {

DiagnosticsModel::DiagnosticsModel(DiagnosticsService *service, QObject *parent)
    : QObject(parent),
      m_service(service) {
    connect(m_service, &DiagnosticsService::changed, this, &DiagnosticsModel::changed);
}

QVariantList DiagnosticsModel::flightMetrics() const {
    return metricMaps(QStringLiteral("flight"));
}

QVariantList DiagnosticsModel::linkMetrics() const {
    return metricMaps(QStringLiteral("link"));
}

QVariantList DiagnosticsModel::systemMetrics() const {
    return metricMaps(QStringLiteral("system"));
}

QVariantList DiagnosticsModel::protocolMetrics() const {
    return metricMaps(QStringLiteral("protocol"));
}

QVariantList DiagnosticsModel::messages() const {
    QVariantList result;
    for (const auto &message : m_service->messages()) {
        const QVariantMap messageMap{
            {QStringLiteral("timestamp"), message.timestamp},
            {QStringLiteral("source"), message.source},
            {QStringLiteral("severity"), message.severity},
            {QStringLiteral("text"), message.text},
        };
        result.push_back(messageMap);
    }
    return result;
}

QVariantList DiagnosticsModel::metricMaps(const QString &group) const {
    QVariantList result;
    for (const auto &metric : m_service->metrics(group)) {
        const QVariantMap metricMap{
            {QStringLiteral("section"), metric.section},
            {QStringLiteral("label"), metric.label},
            {QStringLiteral("value"), metric.value},
            {QStringLiteral("detail"), metric.detail},
        };
        result.push_back(metricMap);
    }
    return result;
}

}  // namespace openhd
