#pragma once

#include <QAbstractListModel>

namespace openhd {

class AlertService;

class AlertListModel : public QAbstractListModel {
    Q_OBJECT
    Q_PROPERTY(int count READ count NOTIFY countChanged)

public:
    enum Roles {
        IdRole = Qt::UserRole + 1,
        SeverityRole,
        TitleRole,
        MessageRole,
        StickyRole,
        TimestampRole,
    };
    Q_ENUM(Roles)

    AlertListModel(AlertService *service, QObject *parent = nullptr);

    int count() const;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE QVariantMap get(int index) const;

signals:
    void countChanged();

private:
    void reload();
    static QString severityToString(int severity);

    AlertService *m_service;
};

}  // namespace openhd
