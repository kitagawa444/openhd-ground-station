#pragma once

#include <QObject>

namespace openhd {

class VideoService;

class VideoModel : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool videoAvailable READ videoAvailable NOTIFY changed)
    Q_PROPERTY(bool frameAvailable READ frameAvailable NOTIFY changed)
    Q_PROPERTY(bool recording READ recording NOTIFY changed)
    Q_PROPERTY(int streamWidth READ streamWidth NOTIFY changed)
    Q_PROPERTY(int streamHeight READ streamHeight NOTIFY changed)
    Q_PROPERTY(int streamFps READ streamFps NOTIFY changed)
    Q_PROPERTY(QString codec READ codec NOTIFY changed)
    Q_PROPERTY(QString decoderState READ decoderState NOTIFY changed)
    Q_PROPERTY(int lastFrameAgeMs READ lastFrameAgeMs NOTIFY changed)
    Q_PROPERTY(quint64 frameSequence READ frameSequence NOTIFY changed)

public:
    VideoModel(VideoService *service, QObject *parent = nullptr);

    bool videoAvailable() const;
    bool frameAvailable() const;
    bool recording() const;
    int streamWidth() const;
    int streamHeight() const;
    int streamFps() const;
    QString codec() const;
    QString decoderState() const;
    int lastFrameAgeMs() const;
    quint64 frameSequence() const;

signals:
    void changed();

private:
    VideoService *m_service;
};

}  // namespace openhd
