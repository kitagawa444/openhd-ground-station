#include "services/LinkStateService.hpp"

namespace openhd {

LinkStateService::LinkStateService(QObject *parent)
    : QObject(parent) {}

const LinkState &LinkStateService::state() const {
    return m_state;
}

void LinkStateService::updateState(const LinkState &state) {
    m_state = state;
    emit stateChanged();
}

void LinkStateService::setLinkConnected(const bool connected) {
    if (m_state.linkConnected == connected) {
        return;
    }

    m_state.linkConnected = connected;
    emit stateChanged();
}

}  // namespace openhd
