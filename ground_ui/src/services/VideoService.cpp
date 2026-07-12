#include "services/VideoService.hpp"

namespace openhd {

VideoService::VideoService(QObject *parent)
    : QObject(parent) {}

const VideoState &VideoService::state() const {
    return m_state;
}

void VideoService::updateState(const VideoState &state) {
    m_state = state;
    emit stateChanged();
}

void VideoService::setRecording(const bool recording) {
    if (m_state.recording == recording) {
        return;
    }

    m_state.recording = recording;
    emit stateChanged();
}

void VideoService::setDecoderState(const QString &decoderState) {
    if (m_state.decoderState == decoderState) {
        return;
    }

    m_state.decoderState = decoderState;
    emit stateChanged();
}

void VideoService::setVideoAvailable(const bool available) {
    if (m_state.videoAvailable == available) {
        return;
    }

    m_state.videoAvailable = available;
    emit stateChanged();
}

}  // namespace openhd
