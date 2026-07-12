#pragma once

#include <QObject>
#include <QVector>

#include "common/types.hpp"

namespace openhd {

class AlertService : public QObject {
    Q_OBJECT

public:
    explicit AlertService(QObject *parent = nullptr);

    QVector<AlertItem> alerts() const;
    void setSystemAlerts(const QVector<AlertItem> &alerts);
    void pushTransientAlert(const AlertItem &alert, int ttlMs = 4000);

signals:
    void alertsChanged();

private:
    void removeTransientAlertById(const QString &id);
    QVector<AlertItem> mergedAlerts() const;

    QVector<AlertItem> m_systemAlerts;
    QVector<AlertItem> m_transientAlerts;
};

}  // namespace openhd
