#pragma once

#include <QImage>
#include <QMutex>
#include <QQuickImageProvider>

namespace openhd {

class VideoFrameProvider : public QQuickImageProvider {
public:
    VideoFrameProvider();

    void setFrame(const QImage &frame);
    QImage requestImage(const QString &id, QSize *size, const QSize &requestedSize) override;

private:
    QMutex m_mutex;
    QImage m_latestFrame;
};

}  // namespace openhd
