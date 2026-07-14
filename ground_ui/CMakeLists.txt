cmake_minimum_required(VERSION 3.22)

project(OpenHDGroundUI VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 6.2 REQUIRED COMPONENTS Core Gui Network Qml Quick QuickControls2)
find_package(PkgConfig REQUIRED)
pkg_check_modules(GSTREAMER REQUIRED IMPORTED_TARGET
    gstreamer-1.0
    gstreamer-app-1.0
    gstreamer-video-1.0
)

qt_add_executable(openhd_ground_ui
    src/main.cpp
    src/app/Application.cpp
    src/app/Application.hpp
    src/app/AppContext.cpp
    src/app/AppContext.hpp
    src/common/types.hpp
    src/integration/MavlinkParser.cpp
    src/integration/MavlinkParser.hpp
    src/integration/OpenHDBackend.cpp
    src/integration/OpenHDBackend.hpp
    src/video/RtpVideoReceiver.cpp
    src/video/RtpVideoReceiver.hpp
    src/video/VideoFrameProvider.cpp
    src/video/VideoFrameProvider.hpp
    src/services/VehicleStateService.cpp
    src/services/VehicleStateService.hpp
    src/services/LinkStateService.cpp
    src/services/LinkStateService.hpp
    src/services/VideoService.cpp
    src/services/VideoService.hpp
    src/services/AlertService.cpp
    src/services/AlertService.hpp
    src/services/DiagnosticsService.cpp
    src/services/DiagnosticsService.hpp
    src/services/DemoDataService.cpp
    src/services/DemoDataService.hpp
    src/models/VehicleModel.cpp
    src/models/VehicleModel.hpp
    src/models/LinkModel.cpp
    src/models/LinkModel.hpp
    src/models/VideoModel.cpp
    src/models/VideoModel.hpp
    src/models/AlertListModel.cpp
    src/models/AlertListModel.hpp
    src/models/DiagnosticsModel.cpp
    src/models/DiagnosticsModel.hpp
    src/commands/FlightCommands.cpp
    src/commands/FlightCommands.hpp
    src/commands/CameraCommands.cpp
    src/commands/CameraCommands.hpp
    src/commands/SystemCommands.cpp
    src/commands/SystemCommands.hpp
)

qt_add_qml_module(openhd_ground_ui
    URI OpenHDGroundUI
    VERSION 1.0
    RESOURCE_PREFIX /qt/qml
    QML_FILES
        qml/Main.qml
        qml/theme/Colors.qml
        qml/theme/Metrics.qml
        qml/theme/Typography.qml
        qml/screens/FlightScreen.qml
        qml/screens/DiagnosticsScreen.qml
        qml/screens/MessagesScreen.qml
        qml/panels/TopStatusBar.qml
        qml/panels/LeftHudPanel.qml
        qml/panels/RightActionPanel.qml
        qml/panels/BottomInfoPanel.qml
        qml/panels/MiniMapPanel.qml
        qml/components/StatusChip.qml
        qml/components/GaugeCard.qml
        qml/components/ActionButton.qml
        qml/components/WarningBanner.qml
        qml/components/VideoCanvas.qml
        qml/components/OsdReadout.qml
        qml/components/FlightTape.qml
        qml/components/CompassRibbon.qml
        qml/components/HorizonOverlay.qml
        qml/components/DiagnosticCard.qml
        qml/components/NavigationBar.qml
)

target_include_directories(openhd_ground_ui PRIVATE src)

target_link_libraries(openhd_ground_ui
    PRIVATE
        Qt6::Core
        Qt6::Gui
        Qt6::Network
        Qt6::Qml
        Qt6::Quick
        Qt6::QuickControls2
        PkgConfig::GSTREAMER
)

include(CTest)

if(BUILD_TESTING)
    find_package(Qt6 6.2 REQUIRED COMPONENTS Test)

    qt_add_executable(mavlink_parser_tests
        tests/MavlinkParserTests.cpp
        src/integration/MavlinkParser.cpp
        src/integration/MavlinkParser.hpp
    )

    target_include_directories(mavlink_parser_tests PRIVATE src)

    target_link_libraries(mavlink_parser_tests
        PRIVATE
            Qt6::Core
            Qt6::Test
    )

    add_test(NAME mavlink_parser_tests COMMAND mavlink_parser_tests)

    qt_add_executable(openhd_backend_tests
        tests/OpenHDBackendTests.cpp
        src/integration/MavlinkParser.cpp
        src/integration/MavlinkParser.hpp
        src/integration/OpenHDBackend.cpp
        src/integration/OpenHDBackend.hpp
        src/video/RtpVideoReceiver.cpp
        src/video/RtpVideoReceiver.hpp
        src/video/VideoFrameProvider.cpp
        src/video/VideoFrameProvider.hpp
        src/common/types.hpp
        src/services/VehicleStateService.cpp
        src/services/VehicleStateService.hpp
        src/services/LinkStateService.cpp
        src/services/LinkStateService.hpp
        src/services/VideoService.cpp
        src/services/VideoService.hpp
        src/services/AlertService.cpp
        src/services/AlertService.hpp
        src/services/DiagnosticsService.cpp
        src/services/DiagnosticsService.hpp
    )

    target_include_directories(openhd_backend_tests PRIVATE src)

    target_link_libraries(openhd_backend_tests
        PRIVATE
            Qt6::Core
            Qt6::Network
            Qt6::Quick
            Qt6::Test
            PkgConfig::GSTREAMER
    )

    add_test(NAME openhd_backend_tests COMMAND openhd_backend_tests)
endif()

install(TARGETS openhd_ground_ui
    BUNDLE DESTINATION .
    RUNTIME DESTINATION bin
)
