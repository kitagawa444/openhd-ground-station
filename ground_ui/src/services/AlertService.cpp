#include "services/AlertService.hpp"

#include <algorithm>

#include <QTimer>

namespace openhd {

AlertService::AlertService(QObject *parent)
    : QObject(parent) {}

QVector<AlertItem> AlertService::alerts() const {
    return mergedAlerts();
}

void AlertService::setSystemAlerts(const QVector<AlertItem> &alerts) {
    m_systemAlerts = alerts;
    emit alertsChanged();
}

void AlertService::pushTransientAlert(const AlertItem &alert, const int ttlMs) {
    m_transientAlerts.prepend(alert);
    emit alertsChanged();

    QTimer::singleShot(ttlMs, this, [this, id = alert.id]() {
        removeTransientAlertById(id);
    });
}

void AlertService::removeTransientAlertById(const QString &id) {
    const auto newEnd = std::remove_if(m_transientAlerts.begin(), m_transientAlerts.end(),
                                       [&id](const AlertItem &alert) { return alert.id == id; });

    if (newEnd == m_transientAlerts.end()) {
        return;
    }

    m_transientAlerts.erase(newEnd, m_transientAlerts.end());
    emit alertsChanged();
}

QVector<AlertItem> AlertService::mergedAlerts() const {
    QVector<AlertItem> merged;
    merged.reserve(m_transientAlerts.size() + m_systemAlerts.size());

    for (const auto &alert : m_transientAlerts) {
        merged.push_back(alert);
    }

    for (const auto &alert : m_systemAlerts) {
        merged.push_back(alert);
    }

    return merged;
}

}  // namespace openhd
