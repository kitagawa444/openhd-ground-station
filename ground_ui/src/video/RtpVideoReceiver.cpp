#include "video/RtpVideoReceiver.hpp"

#include <QDebug>

namespace openhd {

namespace {

constexpr int kOpenHDRtpPort = 5800;

}  // namespace

RtpVideoReceiver::RtpVideoReceiver(QObject *parent)
    : QObject(parent) {
    gst_init(nullptr, nullptr);
}

RtpVideoReceiver::~RtpVideoReceiver() {
    stop();
}

void RtpVideoReceiver::start(const Codec codec) {
    if (m_pipeline != nullptr && m_codec == codec) {
        return;
    }

    stop();
    m_codec = codec;

    GError *error = nullptr;
    const QByteArray description = pipelineDescription(codec).toUtf8();
    m_pipeline = gst_parse_launch(description.constData(), &error);
    if (m_pipeline == nullptr) {
        qWarning() << "Unable to create OpenHD RTP pipeline:"
                   << (error != nullptr ? error->message : "unknown error");
        if (error != nullptr) {
            g_error_free(error);
        }
        return;
    }

    m_appSink = gst_bin_get_by_name(GST_BIN(m_pipeline), "video_sink");
    if (m_appSink == nullptr) {
        qWarning() << "OpenHD RTP pipeline has no appsink";
        stop();
        return;
    }

    g_signal_connect(m_appSink, "new-sample", G_CALLBACK(&RtpVideoReceiver::onNewSample), this);
    const GstStateChangeReturn stateChange = gst_element_set_state(m_pipeline, GST_STATE_PLAYING);
    if (stateChange == GST_STATE_CHANGE_FAILURE) {
        qWarning() << "Unable to start OpenHD RTP pipeline";
        stop();
    }
}

void RtpVideoReceiver::stop() {
    if (m_pipeline != nullptr) {
        gst_element_set_state(m_pipeline, GST_STATE_NULL);
    }

    if (m_appSink != nullptr) {
        gst_object_unref(m_appSink);
        m_appSink = nullptr;
    }

    if (m_pipeline != nullptr) {
        gst_object_unref(m_pipeline);
        m_pipeline = nullptr;
    }
}

RtpVideoReceiver::Codec RtpVideoReceiver::codec() const {
    return m_codec;
}

GstFlowReturn RtpVideoReceiver::onNewSample(GstAppSink *appSink, gpointer userData) {
    auto *receiver = static_cast<RtpVideoReceiver *>(userData);
    GstSample *sample = gst_app_sink_pull_sample(appSink);
    if (sample == nullptr) {
        return GST_FLOW_ERROR;
    }

    receiver->publishSample(sample);
    gst_sample_unref(sample);
    return GST_FLOW_OK;
}

void RtpVideoReceiver::publishSample(GstSample *sample) {
    GstCaps *caps = gst_sample_get_caps(sample);
    GstBuffer *buffer = gst_sample_get_buffer(sample);
    if (caps == nullptr || buffer == nullptr) {
        return;
    }

    GstVideoInfo videoInfo;
    if (!gst_video_info_from_caps(&videoInfo, caps)) {
        return;
    }

    GstMapInfo mappedBuffer;
    if (!gst_buffer_map(buffer, &mappedBuffer, GST_MAP_READ)) {
        return;
    }

    const QImage frame(mappedBuffer.data,
                       GST_VIDEO_INFO_WIDTH(&videoInfo),
                       GST_VIDEO_INFO_HEIGHT(&videoInfo),
                       GST_VIDEO_INFO_PLANE_STRIDE(&videoInfo, 0),
                       QImage::Format_RGBA8888);
    emit frameReady(frame.copy());
    gst_buffer_unmap(buffer, &mappedBuffer);
}

QString RtpVideoReceiver::pipelineDescription(const Codec codec) const {
    QString depayAndDecode;
    switch (codec) {
    case Codec::H264:
        depayAndDecode = QStringLiteral(
            "caps=\"application/x-rtp,media=(string)video,encoding-name=(string)H264,payload=(int)96\" "
            "! rtpjitterbuffer latency=0 drop-on-latency=true ! rtph264depay ! h264parse ! avdec_h264");
        break;
    case Codec::H265:
        depayAndDecode = QStringLiteral(
            "caps=\"application/x-rtp,media=(string)video,encoding-name=(string)H265\" "
            "! rtpjitterbuffer latency=0 drop-on-latency=true ! rtph265depay ! h265parse ! avdec_h265");
        break;
    case Codec::Mjpeg:
        depayAndDecode = QStringLiteral(
            "caps=\"application/x-rtp,media=(string)video,encoding-name=(string)JPEG\" "
            "! rtpjitterbuffer latency=0 drop-on-latency=true ! rtpjpegdepay ! jpegdec");
        break;
    }

    return QStringLiteral("udpsrc address=127.0.0.1 port=%1 %2 "
                          "! queue max-size-buffers=2 leaky=downstream "
                          "! videoconvert ! video/x-raw,format=RGBA "
                          "! appsink name=video_sink emit-signals=true drop=true max-buffers=1 sync=false")
        .arg(kOpenHDRtpPort)
        .arg(depayAndDecode);
}

}  // namespace openhd
