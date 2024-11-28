#ifndef __MAIX_RTSP_SERVER_HPP__
#define __MAIX_RTSP_SERVER_HPP__

#include "xop/RtspServer.h"
#include "net/Timer.h"
#include <string>
#include <memory>
#include <iostream>

class MaixRtspServer
{
    int *clients;
    xop::MediaSession *_session = nullptr;
    bool _has_audio = false;
public:
    xop::MediaSessionId session_id;
    std::shared_ptr<xop::RtspServer> server;
    std::shared_ptr<xop::EventLoop> event_loop;

    MaixRtspServer(int *clients, std::shared_ptr<xop::RtspServer> server_ptr, xop::MediaSessionId session_id, std::shared_ptr<xop::EventLoop> event_loop_ptr, xop::MediaSession *session, bool has_audio)
    {
        this->clients = clients;
        this->server = std::move(server_ptr);
        this->session_id = session_id;
        this->event_loop = std::move(event_loop_ptr);
        this->_session = session;
        this->_has_audio = has_audio;
    }

    ~MaixRtspServer() {
        server->RemoveSession(session_id);
        server->Stop();

        if (clients != nullptr) {
            free(clients);
            clients = nullptr;
        }

        server.reset();
        event_loop.reset();
    }

    int get_clients() {
        int value = 0;
        if (clients != nullptr) {
            value = *clients;
        }
        return value;
    }

    int video_frame_push(uint8_t *frame, size_t frame_size, size_t timestamp, bool i_or_b = true) {
        xop::AVFrame videoFrame = {0};
        videoFrame.type = 0;
        videoFrame.size = frame_size;
        // videoFrame.timestamp = timestamp;
        videoFrame.timestamp = xop::H264Source::GetTimestamp();
        videoFrame.buffer.reset(new uint8_t[videoFrame.size]);
        memcpy(videoFrame.buffer.get(), frame, videoFrame.size);

        xop::RtspServer *rtsp_server = this->server.get();
        rtsp_server->PushFrame(this->session_id, xop::channel_0, videoFrame); //送到服务器进行转发, 接口线程安全
        return 0;
    }

    int audio_frame_push(uint8_t *frame, size_t frame_size, size_t timestamp) {
        xop::AVFrame audioFrame = {0};
        audioFrame.type = xop::AUDIO_FRAME;
        audioFrame.size = frame_size;
        // audioFrame.timestamp = timestamp;
        audioFrame.timestamp = xop::AACSource::GetTimestamp(48000);
        audioFrame.buffer.reset(new uint8_t[audioFrame.size]);
        memcpy(audioFrame.buffer.get(), frame, audioFrame.size);

        xop::RtspServer *rtsp_server = this->server.get();
        rtsp_server->PushFrame(this->session_id, xop::channel_1, audioFrame);
        return 0;
    }
};

class MaixRtspServerBuilder {
    std::string _ip = "0.0.0.0";
    int _port = 8554;
    std::string _session_name = "live";
    bool _has_audio = false;
    int _audio_sample_rate = 48000;
    int _audio_channels = 1;
public:
    MaixRtspServerBuilder() {

    }

    ~MaixRtspServerBuilder() {

    }

    MaixRtspServerBuilder &set_ip(std::string ip) {
        _ip = ip;
        return *this;
    }

    MaixRtspServerBuilder &set_port(int port) {
        _port = port;
        return *this;
    }

    MaixRtspServerBuilder &set_session_name(std::string session_name) {
        _session_name = session_name;
        return *this;
    }

    MaixRtspServerBuilder &set_audio(bool en) {
        _has_audio = en;
        return *this;
    }

    MaixRtspServerBuilder &set_audio_sample_rate(int sample_rate) {
        _audio_sample_rate = sample_rate;
        return *this;
    }

    MaixRtspServerBuilder &set_audio_channels(int channels) {
        _audio_channels = channels;
        return *this;
    }

    MaixRtspServer *build() {
        std::string ip = _ip;
        int port = _port;
        std::string session_name = _session_name;
        int audio_sample_rate = _audio_sample_rate;
        int audio_channels = _audio_channels;
        // printf("ip=%s, port=%d, session_name=%s, audio_sample_rate=%d, audio_channels=%d\r\n", ip.c_str(), port, session_name.c_str(), audio_sample_rate, audio_channels);

        std::shared_ptr<xop::EventLoop> event_loop(new xop::EventLoop());
        std::shared_ptr<xop::RtspServer> server = xop::RtspServer::Create(event_loop.get());
        if (!server->Start(ip, port)) {
            throw "rtsp server start failed";
        }

        xop::MediaSession *session = xop::MediaSession::CreateNew(session_name);
        session->AddSource(xop::channel_0, xop::H264Source::CreateNew());
        if (_has_audio) {
            session->AddSource(xop::channel_1, xop::AACSource::CreateNew(audio_sample_rate, audio_channels));
        }
        int *clients = (int *)malloc(sizeof(int));
        if (clients == nullptr) {
            throw "alloc memory failed";
        }
        *clients = 0;
        session->AddNotifyConnectedCallback([clients] (xop::MediaSessionId sessionId, std::string peer_ip, uint16_t peer_port){
            printf("RTSP client connect, ip=%s, port=%hu \n", peer_ip.c_str(), peer_port);
            (*clients) ++;
        });
        session->AddNotifyDisconnectedCallback([clients](xop::MediaSessionId sessionId, std::string peer_ip, uint16_t peer_port) {
            printf("RTSP client disconnect, ip=%s, port=%hu \n", peer_ip.c_str(), peer_port);
            if (*clients > 0) {
                (*clients) --;
            }
        });
        xop::MediaSessionId session_id = server->AddSession(session);
        MaixRtspServer *new_server(new MaixRtspServer(clients, server, session_id, event_loop, session, _has_audio));

        return new_server;
    }
};

#endif // __MAIX_RTSP_SERVER_HPP__