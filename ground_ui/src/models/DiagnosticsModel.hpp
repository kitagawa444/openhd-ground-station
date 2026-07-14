#pragma once

#include <QObject>
#include <QVariantList>

namespace openhd {

class DiagnosticsService;

class DiagnosticsModel : public QObject {
    Q_OBJECT
    Q_PROPERTY(QVariantList flightMetrics READ flightMetrics NOTIFY changed)
    Q_PROPERTY(QVariantList linkMetrics READ linkMetrics NOTIFY changed)
    Q_PROPERTY(QVariantList systemMetrics READ systemMetrics NOTIFY changed)
    Q_PROPERTY(QVariantList protocolMetrics READ protocolMetrics NOTIFY changed)
    Q_PROPERTY(QVariantList messages READ messages NOTIFY changed)

public:
    DiagnosticsModel(DiagnosticsService *service, QObject *parent = nullptr);

    QVariantList flightMetrics() const;
    QVariantList linkMetrics() const;
    QVariantList systemMetrics() const;
    QVariantList protocolMetrics() const;
    QVariantList messages() const;

signals:
    void changed();

private:
    QVariantList metricMaps(const QString &group) const;

    DiagnosticsService *m_service;
};

}  // namespace openhd
