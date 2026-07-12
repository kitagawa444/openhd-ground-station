#include <QtTest>

#include "integration/MavlinkParser.hpp"

namespace openhd {

class MavlinkParserTests : public QObject {
    Q_OBJECT

private slots:
    void parsesFragmentedMavlinkV2Frame();
    void parsesMavlinkV1FrameAfterNoise();
};

void MavlinkParserTests::parsesFragmentedMavlinkV2Frame() {
    MavlinkParser parser;
    const QByteArray frame = QByteArray::fromHex("fd030000010101210000aabbcc0000");

    QCOMPARE(parser.consume(frame.left(7)).size(), 0);
    const auto frames = parser.consume(frame.mid(7));

    QCOMPARE(frames.size(), 1);
    QCOMPARE(frames.front().messageId, 33U);
    QCOMPARE(frames.front().systemId, 1U);
    QCOMPARE(frames.front().componentId, 1U);
    QCOMPARE(frames.front().payload, QByteArray::fromHex("aabbcc"));
}

void MavlinkParserTests::parsesMavlinkV1FrameAfterNoise() {
    MavlinkParser parser;
    const auto frames = parser.consume(QByteArray::fromHex("1020fe020201014a11220000"));

    QCOMPARE(frames.size(), 1);
    QCOMPARE(frames.front().messageId, 74U);
    QCOMPARE(frames.front().systemId, 1U);
    QCOMPARE(frames.front().componentId, 1U);
    QCOMPARE(frames.front().payload, QByteArray::fromHex("1122"));
}

}  // namespace openhd

QTEST_APPLESS_MAIN(openhd::MavlinkParserTests)

#include "MavlinkParserTests.moc"
