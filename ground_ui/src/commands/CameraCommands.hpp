#pragma once

#include <QObject>

namespace openhd {

class VideoService;
class AlertService;

class CameraCommands : public QObject {
    Q_OBJECT

public:
    CameraCommands(VideoService *videoService,
                   AlertService *alertService,
                   bool demoMode,
                   QObject *parent = nullptr);

    Q_INVOKABLE void toggleRecording();
    Q_INVOKABLE void restartVideo();

private:
    VideoService *m_videoService;
    AlertService *m_alertService;
    bool m_demoMode;
};

}  // namespace openhd
