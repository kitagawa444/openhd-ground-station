#include "models/AlertListModel.hpp"

#include "services/AlertService.hpp"

namespace openhd {

AlertListModel::AlertListModel(AlertService *service, QObject *parent)
    : QAbstractListModel(parent),
      m_service(service) {
    connect(m_service, &AlertService::alertsChanged, this, &AlertListModel::reload);
}

int AlertListModel::count() const {
    return rowCount();
}

int AlertListModel::rowCount(const QModelIndex &parent) const {
    if (parent.isValid()) {
        return 0;
    }

    return m_service->alerts().size();
}

QVariant AlertListModel::data(const QModelIndex &index, const int role) const {
    const auto alerts = m_service->alerts();
    if (!index.isValid() || index.row() < 0 || index.row() >= alerts.size()) {
        return {};
    }

    const auto &alert = alerts.at(index.row());
    switch (role) {
    case IdRole:
        return alert.id;
    case SeverityRole:
        return severityToString(static_cast<int>(alert.severity));
    case TitleRole:
        return alert.title;
    case MessageRole:
        return alert.message;
    case StickyRole:
        return alert.sticky;
    case TimestampRole:
        return alert.timestamp.toLocalTime().toString(QStringLiteral("HH:mm:ss"));
    default:
        return {};
    }
}

QHash<int, QByteArray> AlertListModel::roleNames() const {
    return {
        {IdRole, "id"},
        {SeverityRole, "severity"},
        {TitleRole, "title"},
        {MessageRole, "message"},
        {StickyRole, "sticky"},
        {TimestampRole, "timestamp"},
    };
}

QVariantMap AlertListModel::get(const int index) const {
    const QModelIndex modelIndex = this->index(index, 0);
    if (!modelIndex.isValid()) {
        return {};
    }

    return {
        {"id", data(modelIndex, IdRole)},
        {"severity", data(modelIndex, SeverityRole)},
        {"title", data(modelIndex, TitleRole)},
        {"message", data(modelIndex, MessageRole)},
        {"sticky", data(modelIndex, StickyRole)},
        {"timestamp", data(modelIndex, TimestampRole)},
    };
}

void AlertListModel::reload() {
    beginResetModel();
    endResetModel();
    emit countChanged();
}

QString AlertListModel::severityToString(const int severity) {
    switch (static_cast<AlertSeverity>(severity)) {
    case AlertSeverity::Info:
        return QStringLiteral("info");
    case AlertSeverity::Caution:
        return QStringLiteral("caution");
    case AlertSeverity::Critical:
        return QStringLiteral("critical");
    }

    return QStringLiteral("info");
}

}  // namespace openhd
