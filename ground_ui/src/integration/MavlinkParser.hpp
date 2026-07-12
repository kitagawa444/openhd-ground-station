#pragma once

#include <QByteArray>
#include <QVector>

namespace openhd {

struct MavlinkFrame {
    quint32 messageId = 0;
    quint8 systemId = 0;
    quint8 componentId = 0;
    QByteArray payload;
};

// Splits a TCP byte stream into complete MAVLink v1/v2 frames. The Ground
// backend is local and trusted; message-specific validation happens upstream.
class MavlinkParser {
public:
    QVector<MavlinkFrame> consume(const QByteArray &data);

private:
    QByteArray m_buffer;
};

}  // namespace openhd
