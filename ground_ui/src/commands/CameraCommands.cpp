#include "commands/CameraCommands.hpp"

#include <QDateTime>
#include <QTimer>

#include "services/AlertService.hpp"
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

CameraCommands::CameraCommands(VideoService *videoService,
                               AlertService *alertService,
                               const bool demoMode,
                               QObject *parent)
    : QObject(parent),
      m_videoService(videoService),
      m_alertService(alertService),
      m_demoMode(demoMode) {}

void CameraCommands::toggleRecording() {
    if (!m_demoMode) {
        return;
    }

    const bool recording = !m_videoService->state().recording;
    m_videoService->setRecording(recording);

    m_alertService->pushTransientAlert(makeTransientAlert(recording ? QStringLiteral("record-start")
                                                                    : QStringLiteral("record-stop"),
                                                          AlertSeverity::Info,
                                                          recording ? QStringLiteral("Recording started")
                                                                    : QStringLiteral("Recording stopped"),
                                                          recording ? QStringLiteral("Demo recorder is now active.")
                                                                    : QStringLiteral("Demo recorder is now idle.")));
}

void CameraCommands::restartVideo() {
    if (!m_demoMode) {
        return;
    }

    m_videoService->setDecoderState(QStringLiteral("Restarting"));
    m_alertService->pushTransientAlert(makeTransientAlert(QStringLiteral("video-restart"),
                                                          AlertSeverity::Caution,
                                                          QStringLiteral("Video restart"),
                                                          QStringLiteral("Demo decoder restart in progress.")));

    QTimer::singleShot(1400, this, [this]() {
        m_videoService->setDecoderState(QStringLiteral("Stable"));
    });
}

}  // namespace openhd
