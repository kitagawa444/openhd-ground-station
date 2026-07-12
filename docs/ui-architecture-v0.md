# Ground Station UI Architecture

Draft v0 for an Ubuntu 22.04 OpenHD ground station UI.

This version is intentionally biased toward:

- fast local bring-up
- clear separation between backend and UI
- low-risk integration with the current OpenHD 2.7.x environment
- a clean path to later add map, mission, and RC features

It is not trying to solve every future requirement up front.

## 1. Goal

Build a single ground station application that integrates:

- live FPV video
- vehicle status
- link health
- map and home position
- warnings
- settings

The target interaction model is closer to a compact DJI Fly-style operator UI than a traditional desktop-heavy GCS.

## 2. Non-Goals for v0

The following are explicitly out of scope for the first implementation:

- mission planning
- joystick or RC transmit control
- onboard parameter editing beyond a few essential items
- multi-vehicle support
- ROS2 dependency in the ground UI process
- direct OpenHD source modification

If needed later, these should be added as separate modules instead of being mixed into the first version.

## 3. Platform Assumptions

- OS: Ubuntu 22.04
- Window system: GNOME desktop session
- GUI stack: Qt 6 + QML
- Language: C++20
- Build: CMake
- Video and telemetry source: OpenHD ground backend

The application must be launchable from a desktop GUI session, not from a plain `tty`.

## 4. High-Level Architecture

```text
OpenHD Ground Backend
    |
    | telemetry / status / video endpoints
    v
Ground Services (C++)
    |
    | normalized state + commands
    v
QML-facing Models (QObject / QAbstractListModel)
    |
    | properties / signals
    v
Qt Quick UI
```

The key rule is:

- services talk to OpenHD
- models talk to services
- QML talks only to models and command objects

QML should not contain protocol logic, parsing, or system state ownership.

## 5. Design Principles

### 5.1 UI is declarative only

QML is responsible for:

- layout
- transitions
- animations
- visual state
- input routing

QML is not responsible for:

- OpenHD communications
- telemetry parsing
- state ownership
- business rules
- command validation

### 5.2 Read path and write path are separated

Reading state and sending commands should not be mixed into the same class.

This makes it easier to:

- audit operator actions
- simulate data for testing
- add safety checks before commands are sent

### 5.3 Models are QML-shaped

The UI should not bind directly to a giant backend manager with dozens of unrelated properties.

Instead, expose smaller read-focused models such as:

- vehicle model
- link model
- video model
- alert model
- settings model

### 5.4 OpenHD integration is replaceable

The application should depend on a small adapter layer so that:

- OpenHD transport details stay local to one module
- mock data can be injected during UI development
- future protocol changes do not force a QML rewrite

## 6. Revised Module Structure

```text
ground_ui/

├── CMakeLists.txt
├── src/
│   ├── main.cpp
│   ├── app/
│   │   ├── Application.cpp
│   │   ├── Application.hpp
│   │   ├── AppContext.cpp
│   │   └── AppContext.hpp
│   ├── integration/
│   │   ├── OpenHDBackend.cpp
│   │   ├── OpenHDBackend.hpp
│   │   ├── VideoStreamSource.cpp
│   │   └── VideoStreamSource.hpp
│   ├── services/
│   │   ├── VehicleStateService.cpp
│   │   ├── VehicleStateService.hpp
│   │   ├── LinkStateService.cpp
│   │   ├── LinkStateService.hpp
│   │   ├── AlertService.cpp
│   │   ├── AlertService.hpp
│   │   ├── SettingsService.cpp
│   │   ├── SettingsService.hpp
│   │   ├── CommandService.cpp
│   │   ├── CommandService.hpp
│   │   ├── VideoService.cpp
│   │   ├── VideoService.hpp
│   │   ├── TelemetryRecorder.cpp
│   │   └── TelemetryRecorder.hpp
│   ├── models/
│   │   ├── VehicleModel.cpp
│   │   ├── VehicleModel.hpp
│   │   ├── LinkModel.cpp
│   │   ├── LinkModel.hpp
│   │   ├── VideoModel.cpp
│   │   ├── VideoModel.hpp
│   │   ├── AlertListModel.cpp
│   │   ├── AlertListModel.hpp
│   │   ├── SettingsModel.cpp
│   │   └── SettingsModel.hpp
│   ├── commands/
│   │   ├── FlightCommands.cpp
│   │   ├── FlightCommands.hpp
│   │   ├── CameraCommands.cpp
│   │   ├── CameraCommands.hpp
│   │   ├── SystemCommands.cpp
│   │   └── SystemCommands.hpp
│   └── common/
│       ├── types.hpp
│       ├── enums.hpp
│       ├── units.hpp
│       └── time_utils.hpp
└── qml/
    ├── Main.qml
    ├── theme/
    │   ├── Colors.qml
    │   ├── Metrics.qml
    │   └── Typography.qml
    ├── screens/
    │   ├── FlightScreen.qml
    │   ├── SettingsScreen.qml
    │   └── DebugScreen.qml
    ├── panels/
    │   ├── TopStatusBar.qml
    │   ├── LeftHudPanel.qml
    │   ├── RightActionPanel.qml
    │   ├── BottomInfoPanel.qml
    │   └── MiniMapPanel.qml
    ├── components/
    │   ├── VideoCanvas.qml
    │   ├── BatteryIndicator.qml
    │   ├── LinkQualityIndicator.qml
    │   ├── AltitudeIndicator.qml
    │   ├── SpeedIndicator.qml
    │   ├── CompassRibbon.qml
    │   ├── WarningBanner.qml
    │   └── RecordBadge.qml
    └── assets/
```

## 7. Why This Structure is Better for v0

Compared with the original draft:

- `VehicleManager` is split into smaller services
- command sending is separated from state reading
- alerts are first-class instead of being buried in telemetry
- the video path is isolated
- QML reads focused models instead of backend managers

This is easier to test, easier to mock, and less likely to turn into a single large QObject.

## 8. Core Backend Modules

### 8.1 OpenHDBackend

This is the only module that knows how to talk to the OpenHD ground environment.

Responsibilities:

- attach to OpenHD telemetry/status endpoints
- subscribe to backend status updates
- normalize raw data into internal events
- expose video stream source details to `VideoService`

Not responsible for:

- UI-facing properties
- operator command policy
- long-term app state ownership

### 8.2 VehicleStateService

Central aggregator for vehicle-related state.

Responsibilities:

- connection state
- arm state
- flight mode
- GPS position
- home position
- altitude and speed
- battery state
- satellite count

This service owns canonical runtime vehicle state inside the app.

### 8.3 LinkStateService

Separate from vehicle state because link health often changes independently.

Responsibilities:

- OpenHD connection status
- RSSI / link quality summary
- packet loss summary
- video bitrate summary
- telemetry freshness
- RC link availability if present

### 8.4 VideoService

Keeps video concerns isolated from general telemetry.

Responsibilities:

- video stream availability
- current stream dimensions
- decode/display state
- record state if exposed by backend
- stream restart or fallback status

### 8.5 AlertService

Converts backend conditions into operator-visible warnings.

Examples:

- link weak
- telemetry stale
- battery low
- GPS degraded
- recording stopped
- no video

Alerts should have severity and TTL so transient warnings do not flicker uncontrollably.

### 8.6 SettingsService

Handles persistent local application settings.

Examples:

- units
- layout preferences
- whether minimap is shown
- debug overlays
- operator preferences

This should not directly edit deep OpenHD backend settings in v0 unless the path is very stable.

### 8.7 CommandService

Validates and dispatches operator actions.

Examples:

- arm
- disarm
- change flight mode
- start or stop recording
- reconnect video
- request reboot

This service should be the only path from UI actions to backend mutations.

### 8.8 TelemetryRecorder

Separates logging from live state handling.

Responsibilities:

- record selected telemetry snapshots
- persist debug logs
- write session files when needed

## 9. QML-Facing Models

These are designed for bindings, not for backend integration.

### 9.1 VehicleModel

Read-only properties such as:

```text
connected
armed
flightMode
latitude
longitude
relativeAltitude
groundSpeed
verticalSpeed
batteryPercent
batteryVoltage
satellites
headingDegrees
homeDistanceMeters
```

### 9.2 LinkModel

Properties such as:

```text
linkConnected
linkQualityPercent
telemetryHealthy
videoHealthy
videoBitrateKbps
packetLossPercent
latencyMs
rcConnected
```

### 9.3 VideoModel

Properties such as:

```text
videoAvailable
recording
streamWidth
streamHeight
decoderState
lastFrameAgeMs
```

### 9.4 AlertListModel

A list model for banners and warning trays.

Roles such as:

```text
id
severity
title
message
sticky
timestamp
```

### 9.5 SettingsModel

Expose only what the UI needs to render.

Examples:

- unit system
- map visible
- debug visible
- color theme

## 10. Command Objects Exposed to QML

Instead of binding buttons directly to a generic manager, expose narrow command objects.

### 10.1 FlightCommands

Examples:

- `arm()`
- `disarm()`
- `setFlightMode(mode)`

### 10.2 CameraCommands

Examples:

- `startRecording()`
- `stopRecording()`
- `restartVideo()`

### 10.3 SystemCommands

Examples:

- `reconnectLink()`
- `restartGroundBackend()`
- `openDebugPanel()`

This keeps QML expressive without giving it access to internal service structure.

## 11. UI Screen Layout for v0

v0 should optimize for a single excellent flight screen before building a full app shell.

### 11.1 Main Flight Screen

```text
+------------------------------------------------------+
| Top Status Bar                                       |
|------------------------------------------------------|
|                                                      |
|                  Live Video Area                     |
|                                                      |
|  Left HUD                     Right Action Strip     |
|                                                      |
|                Bottom Info Panel                     |
|                                      Mini Map        |
+------------------------------------------------------+
```

### 11.2 Top Status Bar

Show only high-value items:

- link status
- battery
- GPS
- flight mode
- recording badge
- time since last telemetry update

### 11.3 Left HUD

Compact and readable:

- altitude
- ground speed
- vertical speed
- heading

### 11.4 Right Action Strip

Large operator-safe actions:

- arm or disarm
- record
- map expand
- settings

### 11.5 Bottom Info Panel

Good place for:

- home distance
- warnings summary
- backend status text

### 11.6 Mini Map

In v0 this can be:

- a simple minimap
- home marker
- vehicle marker
- heading indicator

Mission planning does not need to be here yet.

## 12. Visual Design Guidance

To avoid a generic desktop-tool look:

- keep the chrome light
- make video the dominant layer
- use a compact aviation-inspired HUD
- use restrained color accents
- reserve red and amber for true warnings
- do not show every numeric field at all times

Recommended base visual language:

- dark translucent panels over video
- strong white or warm-gray typography
- green for healthy
- amber for caution
- red for critical

## 13. Data Flow

### Read flow

```text
OpenHDBackend
  -> services
  -> models
  -> QML
```

### Write flow

```text
QML
  -> command objects
  -> CommandService
  -> OpenHDBackend
```

This should remain true even as new features are added.

## 14. Suggested Milestones

### Milestone 0

App shell only:

- Qt app starts
- dummy models
- QML flight screen mock

### Milestone 1

Read-only live status:

- vehicle model wired
- link model wired
- alert model wired
- no commands yet

### Milestone 2

Video integration:

- live video visible
- video health and reconnection states

### Milestone 3

Operator actions:

- arm or disarm
- record toggle
- safe reconnect actions

### Milestone 4

Map and settings:

- minimap
- persistent local settings

## 15. Risks

### 15.1 Video rendering complexity

The highest-risk integration area is the video pipeline:

- OpenHD output format
- Qt6 rendering path
- GStreamer integration
- X11 vs Wayland behavior

Treat video as its own track early.

### 15.2 Too much logic in QObject classes

If services and models are not kept separate, the backend layer will become hard to maintain quickly.

### 15.3 Premature mission or RC support

Adding mission planning or joystick calibration in v0 will slow down the core flight UI significantly.

## 16. Recommended Next Step

Start a new `ground_ui/` project with:

- `Application`
- `OpenHDBackend`
- `VehicleStateService`
- `LinkStateService`
- `VideoService`
- `AlertService`
- `VehicleModel`
- `LinkModel`
- `VideoModel`
- `AlertListModel`
- `FlightScreen.qml`

That is enough to build a convincing first operator UI without dragging in too much complexity.
