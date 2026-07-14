#pragma once

#include <QHash>
#include <QObject>
#include <QVector>

#include "common/types.hpp"

namespace openhd {

class DiagnosticsService : public QObject {
    Q_OBJECT

public:
    explicit DiagnosticsService(QObject *parent = nullptr);

    QVector<DiagnosticItem> metrics(const QString &group) const;
    QVector<DiagnosticMessage> messages() const;
    void setMetric(const QString &group,
                   const QString &section,
                   const QString &id,
                   const QString &label,
                   const QString &value,
                   const QString &detail = {});
    void observeFrame(quint32 messageId,
                      quint8 systemId,
                      quint8 componentId,
                      const QString &name,
                      const QByteArray &payload);
    void appendMessage(const QString &source, const QString &severity, const QString &text);

signals:
    void changed();

private:
    void scheduleChanged();

    QHash<QString, DiagnosticItem> m_metrics;
    QVector<DiagnosticMessage> m_messages;
    bool m_changeQueued = false;
};

}  // namespace openhd
