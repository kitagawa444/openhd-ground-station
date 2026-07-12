#include "commands/SystemCommands.hpp"

#include <QDateTime>
#include <QTimer>

#include "services/AlertService.hpp"
#include "services/LinkStateService.hpp"
#include "services/VideoService.hpp"

namespace openhd {

namespace {

AlertItem makeTransientAlert(const QString &id,
                             const AlertSeverity severity,
                             const QString &title,
                             const QString &message) {
    AlertItem alert;
    alert.id = id;
    alert.severity = severity;
    alert.title = title;
    alert.message = message;
    alert.timestamp = QDateTime::currentDateTimeUtc();
    return alert;
}

}  // namespace

SystemCommands::SystemCommands(LinkStateService *linkStateService,
                               VideoService *videoService,
                               AlertService *alertService,
                               const bool demoMode,
                               QObject *parent)
    : QObject(parent),
      m_linkStateService(linkStateService),
      m_videoService(videoService),
      m_alertService(alertService),
      m_demoMode(demoMode) {}

void SystemCommands::reconnectLink() {
    if (!m_demoMode) {
        return;
    }

    m_linkStateService->setLinkConnected(false);
    m_videoService->setVideoAvailable(false);
    m_videoService->setDecoderState(QStringLiteral("Reconnecting"));
    m_alertService->pushTransientAlert(makeTransientAlert(QStringLiteral("link-restart"),
                                                          AlertSeverity::Caution,
                                                          QStringLiteral("Link restart"),
                                                          QStringLiteral("Demo reconnect cycle started.")));

    QTimer::singleShot(1800, this, [this]() {
        m_linkStateService->setLinkConnected(true);
        m_videoService->setVideoAvailable(true);
        m_videoService->setDecoderState(QStringLiteral("Stable"));
        m_alertService->pushTransientAlert(makeTransientAlert(QStringLiteral("link-restored"),
                                                              AlertSeverity::Info,
                                                              QStringLiteral("Link restored"),
                                                              QStringLiteral("Demo reconnect cycle completed.")));
    });
}

}  // namespace openhd
