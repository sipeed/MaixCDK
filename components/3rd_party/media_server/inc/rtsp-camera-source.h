#ifndef _rtsp_camera_source_h_
#define _rtsp_camera_source_h_

#include "rtsp-camera-reader.h"
#include "media-source.h"
#include "sys/process.h"
#include "time64.h"
#include "rtp.h"
#include <string>

class RtspCameraSource : public IMediaSource
{
public:
	RtspCameraSource(const char *file);
	virtual ~RtspCameraSource();

public:
	virtual int Play();
	virtual int Pause();
	virtual int Seek(int64_t pos);
	virtual int SetSpeed(double speed);
	virtual int GetDuration(int64_t& duration) const;
	virtual int GetSDPMedia(std::string& sdp) const;
	virtual int GetRTPInfo(const char* uri, char *rtpinfo, size_t bytes) const;
	virtual int SetTransport(const char* track, std::shared_ptr<IRTPTransport> transport);
	int Push(int64_t time, const uint8_t* nalu, size_t bytes);
	int SetPspFromFrame(const uint8_t* nalu, size_t bytes);
private:
	static void OnRTCPEvent(void* param, const struct rtcp_msg_t* msg);
	void OnRTCPEvent(const struct rtcp_msg_t* msg);
	int SendRTCP();

	static void* RTPAlloc(void* param, int bytes);
	static void RTPFree(void* param, void *packet);
	static int RTPPacket(void* param, const void *packet, int bytes, uint32_t timestamp, int flags);

private:
	void* m_rtp;
	uint32_t m_timestamp;
	time64_t m_rtp_clock;
	time64_t m_rtcp_clock;
    RtspCameraReader m_reader;
	std::shared_ptr<IRTPTransport> m_transport;

	int m_status;
	int64_t m_pos;
	double m_speed;

	void *m_rtppacker;
	unsigned char m_packet[MAX_UDP_PACKET+14];
};

#endif /* !_rtsp_camera_source_h_ */
