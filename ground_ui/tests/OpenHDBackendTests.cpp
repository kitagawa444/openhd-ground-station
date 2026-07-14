#include <QtTest>

#include <cstring>

#include <QCoreApplication>
#include <QTcpServer>
#include <QTcpSocket>

#include "integration/OpenHDBackend.hpp"
#include "services/AlertService.hpp"
#include "services/DiagnosticsService.hpp"
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

void appendFloat(QByteArray &buffer, const float value) {
    quint32 raw = 0;
    static_assert(sizeof(raw) == sizeof(value));
    std::memcpy(&raw, &value, sizeof(raw));
    appendU32(buffer, raw);
}

void writeU16(QByteArray &buffer, const int offset, const quint16 value) {
    buffer[offset] = static_cast<char>(value & 0xFFU);
    buffer[offset + 1] = static_cast<char>((value >> 8U) & 0xFFU);
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

QByteArray attitudePayload() {
    QByteArray payload;
    appendU32(payload, 1000);
    appendFloat(payload, 0.17453292F);
    appendFloat(payload, -0.08726646F);
    appendFloat(payload, 1.57079633F);
    appendFloat(payload, 0.0F);
    appendFloat(payload, 0.0F);
    appendFloat(payload, 0.0F);
    return payload;
}

QByteArray wifiCardPayload() {
    QByteArray payload(38, '\0');
    payload[23] = static_cast<char>(-58);
    payload[29] = static_cast<char>(83);
    payload[32] = static_cast<char>(2);
    return payload;
}

QByteArray channelAnalysisPayload() {
    QByteArray payload(191, '\0');
    writeU16(payload, 8, 5180);
    writeU16(payload, 68, 12);
    payload[190] = static_cast<char>(45);
    return payload;
}

const DiagnosticItem *findMetric(const QVector<DiagnosticItem> &metrics, const QString &label) {
    for (const auto &metric : metrics) {
        if (metric.label == label) {
            return &metric;
        }
    }
    return nullptr;
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
    DiagnosticsService diagnosticsService;
    OpenHDBackend backend(&vehicleService,
                          &linkService,
                          &videoService,
                          &alertService,
                          &diagnosticsService,
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
    frames.append(makeMavlinkV2Frame(30, attitudePayload()));
    frames.append(makeMavlinkV2Frame(1212, wifiCardPayload()));
    frames.append(makeMavlinkV2Frame(1219, cameraStatusPayload()));
    frames.append(makeMavlinkV2Frame(1224, channelAnalysisPayload()));
    QByteArray unknownPayload(64, '\0');
    unknownPayload[63] = static_cast<char>(0x7F);
    frames.append(makeMavlinkV2Frame(5000, unknownPayload));
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
    QVERIFY(qAbs(vehicleService.state().rollDegrees - 10.0) < 0.01);
    QVERIFY(qAbs(vehicleService.state().pitchDegrees + 5.0) < 0.01);
    QVERIFY(qAbs(vehicleService.state().yawDegrees - 90.0) < 0.01);
    QCOMPARE(vehicleService.state().batteryPercent, 78);
    QCOMPARE(vehicleService.state().batteryVoltage, 15.84);
    QCOMPARE(videoService.state().streamWidth, 1920);
    QCOMPARE(videoService.state().streamHeight, 1080);
    QCOMPARE(videoService.state().streamFps, 60);
    QCOMPARE(videoService.state().codec, QStringLiteral("H.264 RTP"));
    QVERIFY(linkService.state().backendConnected);
    QTRY_VERIFY(linkService.state().linkConnected);
    QCOMPARE(linkService.state().rssiDbm, -58);

    QTRY_VERIFY(findMetric(diagnosticsService.metrics(QStringLiteral("system")),
                           QStringLiteral("Analysis progress")) != nullptr);
    const auto systemMetrics = diagnosticsService.metrics(QStringLiteral("system"));
    const auto *analysisProgress = findMetric(systemMetrics, QStringLiteral("Analysis progress"));
    QVERIFY(analysisProgress != nullptr);
    QCOMPARE(analysisProgress->value, QStringLiteral("45 %"));
    const auto *foreignPackets = findMetric(systemMetrics, QStringLiteral("5180 MHz foreign packets"));
    QVERIFY(foreignPackets != nullptr);
    QCOMPARE(foreignPackets->value, QStringLiteral("12"));

    QTRY_VERIFY(findMetric(diagnosticsService.metrics(QStringLiteral("protocol")),
                           QStringLiteral("MAVLINK 5000")) != nullptr);
    const auto protocolMetrics = diagnosticsService.metrics(QStringLiteral("protocol"));
    const auto *unknownFrame = findMetric(protocolMetrics, QStringLiteral("MAVLINK 5000"));
    QVERIFY(unknownFrame != nullptr);
    QVERIFY(unknownFrame->detail.contains(QStringLiteral("64 bytes")));
    QVERIFY(unknownFrame->detail.endsWith(QStringLiteral("7f")));
}

}  // namespace openhd

int main(int argc, char **argv) {
    QCoreApplication application(argc, argv);
    openhd::OpenHDBackendTests tests;
    return QTest::qExec(&tests, argc, argv);
}

#include "OpenHDBackendTests.moc"
