#include "integration/MavlinkParser.hpp"

#include <algorithm>

namespace openhd {

namespace {

constexpr quint8 kMavlinkV1Magic = 0xFE;
constexpr quint8 kMavlinkV2Magic = 0xFD;
constexpr quint8 kMavlinkV2SignedFlag = 0x01;

quint8 byteAt(const QByteArray &data, const int index) {
    return static_cast<quint8>(data.at(index));
}

int nextFrameStart(const QByteArray &data) {
    const int v1 = data.indexOf(static_cast<char>(kMavlinkV1Magic));
    const int v2 = data.indexOf(static_cast<char>(kMavlinkV2Magic));

    if (v1 < 0) {
        return v2;
    }

    if (v2 < 0) {
        return v1;
    }

    return std::min(v1, v2);
}

}  // namespace

QVector<MavlinkFrame> MavlinkParser::consume(const QByteArray &data) {
    m_buffer.append(data);

    QVector<MavlinkFrame> frames;
    while (m_buffer.size() >= 2) {
        const int frameStart = nextFrameStart(m_buffer);
        if (frameStart < 0) {
            // Preserve a possible magic byte that may be completed by the next read.
            m_buffer = m_buffer.right(1);
            break;
        }

        if (frameStart > 0) {
            m_buffer.remove(0, frameStart);
        }

        if (m_buffer.size() < 2) {
            break;
        }

        const quint8 magic = byteAt(m_buffer, 0);
        const quint8 payloadLength = byteAt(m_buffer, 1);
        const bool isV2 = magic == kMavlinkV2Magic;
        const int headerLength = isV2 ? 10 : 6;

        if (!isV2 && magic != kMavlinkV1Magic) {
            m_buffer.remove(0, 1);
            continue;
        }

        if (m_buffer.size() < headerLength) {
            break;
        }

        const int signatureLength = isV2 && (byteAt(m_buffer, 2) & kMavlinkV2SignedFlag) ? 13 : 0;
        const int frameLength = headerLength + payloadLength + 2 + signatureLength;
        if (m_buffer.size() < frameLength) {
            break;
        }

        MavlinkFrame frame;
        if (isV2) {
            frame.systemId = byteAt(m_buffer, 5);
            frame.componentId = byteAt(m_buffer, 6);
            frame.messageId = static_cast<quint32>(byteAt(m_buffer, 7)) |
                              (static_cast<quint32>(byteAt(m_buffer, 8)) << 8U) |
                              (static_cast<quint32>(byteAt(m_buffer, 9)) << 16U);
        } else {
            frame.systemId = byteAt(m_buffer, 3);
            frame.componentId = byteAt(m_buffer, 4);
            frame.messageId = byteAt(m_buffer, 5);
        }

        frame.payload = m_buffer.mid(headerLength, payloadLength);
        frames.push_back(std::move(frame));
        m_buffer.remove(0, frameLength);
    }

    return frames;
}

}  // namespace openhd
