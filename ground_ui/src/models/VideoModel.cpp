#include "models/VideoModel.hpp"

#include "services/VideoService.hpp"

namespace openhd {

VideoModel::VideoModel(VideoService *service, QObject *parent)
    : QObject(parent),
      m_service(service) {
    connect(m_service, &VideoService::stateChanged, this, &VideoModel::changed);
}

bool VideoModel::videoAvailable() const {
    return m_service->state().videoAvailable;
}

bool VideoModel::frameAvailable() const {
    return m_service->state().frameAvailable;
}

bool VideoModel::recording() const {
    return m_service->state().recording;
}

int VideoModel::streamWidth() const {
    return m_service->state().streamWidth;
}

int VideoModel::streamHeight() const {
    return m_service->state().streamHeight;
}

int VideoModel::streamFps() const {
    return m_service->state().streamFps;
}

QString VideoModel::codec() const {
    return m_service->state().codec;
}

QString VideoModel::decoderState() const {
    return m_service->state().decoderState;
}

int VideoModel::lastFrameAgeMs() const {
    return m_service->state().lastFrameAgeMs;
}

quint64 VideoModel::frameSequence() const {
    return m_service->state().frameSequence;
}

}  // namespace openhd
