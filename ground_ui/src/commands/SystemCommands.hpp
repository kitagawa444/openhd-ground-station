#pragma once

#include <QObject>

namespace openhd {

class LinkStateService;
class VideoService;
class AlertService;

class SystemCommands : public QObject {
    Q_OBJECT

public:
    SystemCommands(LinkStateService *linkStateService,
                   VideoService *videoService,
                   AlertService *alertService,
                   bool demoMode,
                   QObject *parent = nullptr);

    Q_INVOKABLE void reconnectLink();

private:
    LinkStateService *m_linkStateService;
    VideoService *m_videoService;
    AlertService *m_alertService;
    bool m_demoMode;
};

}  // namespace openhd
