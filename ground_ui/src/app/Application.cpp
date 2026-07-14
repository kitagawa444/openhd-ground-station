#include "app/Application.hpp"

#include <QQmlContext>
#include <QQuickStyle>

namespace openhd {

Application::Application(int &argc, char **argv)
    : m_app(argc, argv),
      m_context() {
    configureApplication();
    configureContext();
    m_context.start();
}

int Application::run() {
    m_engine.load(QUrl(QStringLiteral("qrc:/qt/qml/OpenHDGroundUI/qml/Main.qml")));
    if (m_engine.rootObjects().isEmpty()) {
        return -1;
    }

    return m_app.exec();
}

void Application::configureApplication() {
    QQuickStyle::setStyle(QStringLiteral("Basic"));
    m_app.setApplicationName(QStringLiteral("OpenHD Ground UI"));
    m_app.setOrganizationName(QStringLiteral("kitagawa444"));
}

void Application::configureContext() {
    auto videoFrameProvider = m_context.takeVideoFrameProvider();
    m_engine.addImageProvider(QStringLiteral("openhd"), videoFrameProvider.release());

    QQmlContext *rootContext = m_engine.rootContext();
    rootContext->setContextProperty(QStringLiteral("vehicleModel"), m_context.vehicleModel());
    rootContext->setContextProperty(QStringLiteral("linkModel"), m_context.linkModel());
    rootContext->setContextProperty(QStringLiteral("videoModel"), m_context.videoModel());
    rootContext->setContextProperty(QStringLiteral("alertModel"), m_context.alertListModel());
    rootContext->setContextProperty(QStringLiteral("diagnosticModel"), m_context.diagnosticsModel());
    rootContext->setContextProperty(QStringLiteral("flightCommands"), m_context.flightCommands());
    rootContext->setContextProperty(QStringLiteral("cameraCommands"), m_context.cameraCommands());
    rootContext->setContextProperty(QStringLiteral("systemCommands"), m_context.systemCommands());
    rootContext->setContextProperty(QStringLiteral("demoMode"), m_context.isDemoMode());
}

}  // namespace openhd
