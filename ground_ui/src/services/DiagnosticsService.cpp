#include "services/DiagnosticsService.hpp"

#include <algorithm>

#include <QDateTime>
#include <QTimer>

namespace openhd {

DiagnosticsService::DiagnosticsService(QObject *parent)
    : QObject(parent) {}

QVector<DiagnosticItem> DiagnosticsService::metrics(const QString &group) const {
    QVector<DiagnosticItem> result;
    for (const auto &item : m_metrics) {
        if (item.group == group) {
            result.push_back(item);
        }
    }

    std::sort(result.begin(), result.end(), [](const DiagnosticItem &left, const DiagnosticItem &right) {
        if (left.section == right.section) {
            return left.label < right.label;
        }
        return left.section < right.section;
    });
    return result;
}

QVector<DiagnosticMessage> DiagnosticsService::messages() const {
    return m_messages;
}

void DiagnosticsService::setMetric(const QString &group,
                                   const QString &section,
                                   const QString &id,
                                   const QString &label,
                                   const QString &value,
                                   const QString &detail) {
    const QString key = group + QLatin1Char('.') + section + QLatin1Char('.') + id;
    const DiagnosticItem updated{key, group, section, label, value, detail};
    const auto current = m_metrics.constFind(key);
    if (current != m_metrics.cend() && current->value == updated.value && current->detail == updated.detail) {
        return;
    }

    m_metrics.insert(key, updated);
    scheduleChanged();
}

void DiagnosticsService::observeFrame(const quint32 messageId,
                                      const quint8 systemId,
                                      const quint8 componentId,
                                      const QString &name,
                                      const QByteArray &payload) {
    const QString id = QStringLiteral("%1.%2.%3").arg(messageId).arg(systemId).arg(componentId);
    const QString existingValue = m_metrics.value(QStringLiteral("protocol.Frame.%1").arg(id)).value;
    const int count = existingValue.section(QLatin1Char(' '), 1, 1).toInt() + 1;
    const QString detail = QStringLiteral("ID %1 | SYS %2 | COMP %3 | %4 bytes | %5")
                               .arg(messageId)
                               .arg(systemId)
                               .arg(componentId)
                               .arg(payload.size())
                               .arg(QString::fromLatin1(payload.toHex(' ')));
    setMetric(QStringLiteral("protocol"),
              QStringLiteral("Frame"),
              id,
              name,
              QStringLiteral("# %1").arg(count),
              detail);
}

void DiagnosticsService::appendMessage(const QString &source,
                                       const QString &severity,
                                       const QString &text) {
    if (text.trimmed().isEmpty()) {
        return;
    }

    DiagnosticMessage message;
    message.timestamp = QDateTime::currentDateTime().toString(QStringLiteral("HH:mm:ss"));
    message.source = source;
    message.severity = severity;
    message.text = text.trimmed();
    m_messages.prepend(message);
    constexpr int kMaxMessages = 100;
    if (m_messages.size() > kMaxMessages) {
        m_messages.resize(kMaxMessages);
    }
    scheduleChanged();
}

void DiagnosticsService::scheduleChanged() {
    if (m_changeQueued) {
        return;
    }

    m_changeQueued = true;
    QTimer::singleShot(0, this, [this]() {
        m_changeQueued = false;
        emit changed();
    });
}

}  // namespace openhd
