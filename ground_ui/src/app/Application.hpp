#pragma once

#include <QGuiApplication>
#include <QQmlApplicationEngine>

#include "app/AppContext.hpp"

namespace openhd {

class Application {
public:
    Application(int &argc, char **argv);
    int run();

private:
    void configureApplication();
    void configureContext();

    QGuiApplication m_app;
    QQmlApplicationEngine m_engine;
    AppContext m_context;
};

}  // namespace openhd
