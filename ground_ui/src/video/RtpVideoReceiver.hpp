#pragma once

#include <QImage>
#include <QObject>

#include <gst/app/gstappsink.h>
#include <gst/video/video.h>

namespace openhd {

class RtpVideoReceiver : public QObject {
    Q_OBJECT

public:
    enum class Codec {
        H264,
        H265,
        Mjpeg,
    };

    explicit RtpVideoReceiver(QObject *parent = nullptr);
    ~RtpVideoReceiver() override;

    void start(Codec codec);
    void stop();
    Codec codec() const;

signals:
    void frameReady(const QImage &frame);

private:
    static GstFlowReturn onNewSample(GstAppSink *appSink, gpointer userData);
    void publishSample(GstSample *sample);
    QString pipelineDescription(Codec codec) const;

    GstElement *m_pipeline = nullptr;
    GstElement *m_appSink = nullptr;
    Codec m_codec = Codec::H264;
};

}  // namespace openhd
