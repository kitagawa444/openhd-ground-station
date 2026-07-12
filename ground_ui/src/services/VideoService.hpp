#pragma once

#include <QObject>

#include "common/types.hpp"

namespace openhd {

class VideoService : public QObject {
    Q_OBJECT

public:
    explicit VideoService(QObject *parent = nullptr);

    const VideoState &state() const;
    void updateState(const VideoState &state);
    void setRecording(bool recording);
    void setDecoderState(const QString &decoderState);
    void setVideoAvailable(bool available);

signals:
    void stateChanged();

private:
    VideoState m_state;
};

}  // namespace openhd
