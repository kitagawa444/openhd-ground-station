#pragma once

#include <QObject>

#include "common/types.hpp"

namespace openhd {

class LinkStateService : public QObject {
    Q_OBJECT

public:
    explicit LinkStateService(QObject *parent = nullptr);

    const LinkState &state() const;
    void updateState(const LinkState &state);
    void setLinkConnected(bool connected);

signals:
    void stateChanged();

private:
    LinkState m_state;
};

}  // namespace openhd
