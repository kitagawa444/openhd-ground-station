#include "commands/FlightCommands.hpp"

#include <QDateTime>
#include <QStringList>

#include "services/AlertService.hpp"
#include "services/VehicleStateService.hpp"

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

QString nextFlightMode(const QString &currentMode) {
    static const QStringList modes = {
        QStringLiteral("POSHOLD"),
        QStringLiteral("CRUISE"),
        QStringLiteral("AUTO"),
        QStringLiteral("RTL"),
    };

    const int currentIndex = modes.indexOf(currentMode);
    if (currentIndex < 0) {
        return modes.front();
    }

    return modes.at((currentIndex + 1) % modes.size());
}

}  // namespace

FlightCommands::FlightCommands(VehicleStateService *vehicleStateService,
                               AlertService *alertService,
                               const bool demoMode,
                               QObject *parent)
    : QObject(parent),
      m_vehicleStateService(vehicleStateService),
      m_alertService(alertService),
      m_demoMode(demoMode) {}

void FlightCommands::arm() {
    if (!m_demoMode) {
        return;
    }

    m_vehicleStateService->setArmed(true);
    m_alertService->pushTransientAlert(makeTransientAlert(QStringLiteral("arm"),
                                                          AlertSeverity::Info,
                                                          QStringLiteral("Armed"),
                                                          QStringLiteral("Demo arm command accepted.")));
}

void FlightCommands::disarm() {
    if (!m_demoMode) {
        return;
    }

    m_vehicleStateService->setArmed(false);
    m_alertService->pushTransientAlert(makeTransientAlert(QStringLiteral("disarm"),
                                                          AlertSeverity::Info,
                                                          QStringLiteral("Disarmed"),
                                                          QStringLiteral("Demo disarm command accepted.")));
}

void FlightCommands::cycleFlightMode() {
    if (!m_demoMode) {
        return;
    }

    const QString mode = nextFlightMode(m_vehicleStateService->state().flightMode);
    m_vehicleStateService->setFlightMode(mode);
    m_alertService->pushTransientAlert(makeTransientAlert(QStringLiteral("mode-change"),
                                                          AlertSeverity::Info,
                                                          QStringLiteral("Mode changed"),
                                                          QStringLiteral("Demo mode switched to %1.").arg(mode)));
}

}  // namespace openhd
