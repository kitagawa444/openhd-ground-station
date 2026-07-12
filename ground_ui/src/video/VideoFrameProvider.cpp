#include "video/VideoFrameProvider.hpp"

#include <QMutexLocker>

namespace openhd {

VideoFrameProvider::VideoFrameProvider()
    : QQuickImageProvider(QQuickImageProvider::Image) {}

void VideoFrameProvider::setFrame(const QImage &frame) {
    QMutexLocker locker(&m_mutex);
    m_latestFrame = frame;
}

QImage VideoFrameProvider::requestImage(const QString &id, QSize *size, const QSize &requestedSize) {
    Q_UNUSED(id)
    Q_UNUSED(requestedSize)

    QMutexLocker locker(&m_mutex);
    if (m_latestFrame.isNull()) {
        QImage placeholder(1, 1, QImage::Format_RGBA8888);
        placeholder.fill(Qt::black);
        if (size != nullptr) {
            *size = placeholder.size();
        }
        return placeholder;
    }

    if (size != nullptr) {
        *size = m_latestFrame.size();
    }
    return m_latestFrame;
}

}  // namespace openhd
