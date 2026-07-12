#include <QtTest>

#include <QCoreApplication>
#include <QTcpServer>
#include <QTcpSocket>

#include "integration/OpenHDBackend.hpp"
#include "services/AlertService.hpp"
#include "services/LinkStateService.hpp"
#include "services/VehicleStateService.hpp"
#include "services/VideoService.hpp"

namespace openhd {

namespace {

void appendU16(QByteArray &buffer, const quint16 value) {
    buffer.append(static_cast<char>(value & 0xFFU));
    buffer.append(static_cast<char>((value >> 8U) & 0xFFU));
}

void appendI16(QByteArray &buffer, const qint16 value) {
    appendU16(buffer, static_cast<quint16>(value));
}

void appendU32(QByteArray &buffer, const quint32 value) {
    buffer.append(static_cast<char>(value & 0xFFU));
    buffer.append(static_cast<char>((value >> 8U) & 0xFFU));
    buffer.append(static_cast<char>((value >> 16U) & 0xFFU));
    buffer.append(static_cast<char>((value >> 24U) & 0xFFU));
}

void appendI32(QByteArray &buffer, const qint32 value) {
    appendU32(buffer, static_cast<quint32>(value));
}

QByteArray makeMavlinkV2Frame(const quint32 messageId, const QByteArray &payload) {
    QByteArray frame;
    frame.append(static_cast<char>(0xFD));
    frame.append(static_cast<char>(payload.size()));
    frame.append(static_cast<char>(0));
    frame.append(static_cast<char>(0));
    frame.append(static_cast<char>(1));
    frame.append(static_cast<char>(1));
    frame.append(static_cast<char>(1));
    frame.append(static_cast<char>(messageId & 0xFFU));
    frame.append(static_cast<char>((messageId >> 8U) & 0xFFU));
    frame.append(static_cast<char>((messageId >> 16U) & 0xFFU));
    frame.append(payload);
    frame.append(static_cast<char>(0));
    frame.append(static_cast<char>(0));
    return frame;
}

QByteArray heartbeatPayload() {
    QByteArray payload;
    appendU32(payload, 5);  // ArduCopter LOITER
    payload.append(static_cast<char>(2));
    payload.append(static_cast<char>(3));
    payload.append(static_cast<char>(0x80));
    payload.append(static_cast<char>(4));
    payload.append(static_cast<char>(3));
    return payload;
}

QByteArray globalPositionPayload() {
    QByteArray payload;
    appendU32(payload, 1000);
    appendI32(payload, 356236000);
    appendI32(payload, 1397768000);
    appendI32(payload, 32000);
    appendI32(payload, 12000);
    appendI16(payload, 300);
    appendI16(payload, 400);
    appendI16(payload, -150);
    appendU16(payload, 9000);
    return payload;
}

QByteArray systemStatusPayload() {
    QByteArray payload(31, '\0');
    payload[14] = static_cast<char>(0xE0);
    payload[15] = static_cast<char>(0x3D);  // 15.84 V
    payload[30] = static_cast<char>(78);
    return payload;
}

QByteArray cameraStatusPayload() {
    QByteArray payload(21, '\0');
    payload[6] = static_cast<char>(0x80);
    payload[7] = static_cast<char>(0x07);   // 1920
    payload[8] = static_cast<char>(0x38);
    payload[9] = static_cast<char>(0x04);   // 1080
    payload[10] = static_cast<char>(60);
    payload[15] = static_cast<char>(1);
    payload[18] = static_cast<char>(0);
    return payload;
}

}  // namespace

class OpenHDBackendTests : public QObject {
    Q_OBJECT

private slots:
    void mapsLiveMavlinkFramesIntoServices();
};

void OpenHDBackendTests::mapsLiveMavlinkFramesIntoServices() {
    QTcpServer server;
    constexpr quint16 testTelemetryPort = 5761;
    QVERIFY(server.listen(QHostAddress::LocalHost, testTelemetryPort));

    VehicleStateService vehicleService;
    LinkStateService linkService;
    VideoService videoService;
    AlertService alertService;
    OpenHDBackend backend(&vehicleService,
                          &linkService,
                          &videoService,
                          &alertService,
                          nullptr,
                          nullptr,
                          testTelemetryPort);
    backend.start();

    QTRY_VERIFY(server.hasPendingConnections());
    QTcpSocket *client = server.nextPendingConnection();
    QVERIFY(client != nullptr);

    QByteArray frames;
    frames.append(makeMavlinkV2Frame(0, heartbeatPayload()));
    frames.append(makeMavlinkV2Frame(1, systemStatusPayload()));
    frames.append(makeMavlinkV2Frame(33, globalPositionPayload()));
    frames.append(makeMavlinkV2Frame(1219, cameraStatusPayload()));
    client->write(frames);
    QVERIFY(client->waitForBytesWritten());

    QTRY_VERIFY(vehicleService.state().connected);
    QCOMPARE(vehicleService.state().flightMode, QStringLiteral("LOITER"));
    QVERIFY(vehicleService.state().armed);
    QVERIFY(vehicleService.state().positionValid);
    QVERIFY(vehicleService.state().batteryValid);
    QCOMPARE(vehicleService.state().satellites, 0);
    QCOMPARE(vehicleService.state().latitude, 35.6236);
    QCOMPARE(vehicleService.state().longitude, 139.7768);
    QCOMPARE(vehicleService.state().relativeAltitude, 12.0);
    QCOMPARE(vehicleService.state().groundSpeed, 5.0);
    QCOMPARE(vehicleService.state().verticalSpeed, 1.5);
    QCOMPARE(vehicleService.state().headingDegrees, 90.0);
    QCOMPARE(vehicleService.state().batteryPercent, 78);
    QCOMPARE(vehicleService.state().batteryVoltage, 15.84);
    QCOMPARE(videoService.state().streamWidth, 1920);
    QCOMPARE(videoService.state().streamHeight, 1080);
    QCOMPARE(videoService.state().streamFps, 60);
    QCOMPARE(videoService.state().codec, QStringLiteral("H.264 RTP"));
    QVERIFY(linkService.state().backendConnected);
    QTRY_VERIFY(linkService.state().linkConnected);
}

}  // namespace openhd

int main(int argc, char **argv) {
    QCoreApplication application(argc, argv);
    openhd::OpenHDBackendTests tests;
    return QTest::qExec(&tests, argc, argv);
}

#include "OpenHDBackendTests.moc"
