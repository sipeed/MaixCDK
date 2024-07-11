#include "rtsp-camera-source.h"
#include "cstringext.h"
#include "base64.h"
#include "rtp-profile.h"
#include "rtp-payload.h"
#include <assert.h>

extern "C" uint32_t rtp_ssrc(void);

RtspCameraSource::RtspCameraSource(const char *file)
:m_reader(file)
{
	m_speed = 1.0;
	m_status = 0;
	m_rtp_clock = 0;
	m_rtcp_clock = 0;
	m_timestamp = 0;

	uint32_t ssrc = rtp_ssrc();
	static struct rtp_payload_t s_rtpfunc = {
		RtspCameraSource::RTPAlloc,
		RtspCameraSource::RTPFree,
		RtspCameraSource::RTPPacket,
	};
	m_rtppacker = rtp_payload_encode_create(RTP_PAYLOAD_H265, "H265", (uint16_t)ssrc, ssrc, &s_rtpfunc, this);

	struct rtp_event_t event;
	event.on_rtcp = OnRTCPEvent;
	m_rtp = rtp_create(&event, this, ssrc, m_timestamp, 90000, 4*1024, 1);
	rtp_set_info(m_rtp, "RTSPServer", "szj.h265");
}

RtspCameraSource::~RtspCameraSource()
{
	if(m_rtp)
		rtp_destroy(m_rtp);

	if(m_rtppacker)
		rtp_payload_encode_destroy(m_rtppacker);
}

int RtspCameraSource::SetTransport(const char* /*track*/, std::shared_ptr<IRTPTransport> transport)
{
	m_transport = transport;
	return 0;
}

int RtspCameraSource::Push(int64_t time, const uint8_t* nalu, size_t bytes)
{
	return m_reader.PushNextFrame(time, nalu, bytes);
}

int RtspCameraSource::SetPspFromFrame(const uint8_t* nalu, size_t bytes)
{
	return m_reader.SetPspFromFrame(nalu, bytes);
}

int RtspCameraSource::Play()
{
	m_status = 1;

	//uint32_t timestamp = 0;
	time64_t clock = time64_now();
	if (0 == m_rtp_clock)
		m_rtp_clock = clock;
	static int64_t last_m_pos = 0;
	if(m_rtp_clock + 10 < clock)
	{
		size_t bytes;
		const uint8_t* ptr;
		if(0 == m_reader.GetNextFrame(m_pos, ptr, bytes))
		{
			m_timestamp = m_pos;
			rtp_payload_encode_input(m_rtppacker, ptr, bytes, m_timestamp * 90 /*kHz*/);
			SendRTCP();
			m_reader.FreeNextFrame();
			return 1;
		}
	}

	return 0;
}

int RtspCameraSource::Pause()
{
	m_status = 2;
	m_rtp_clock = 0;
	return 0;
}

int RtspCameraSource::Seek(int64_t pos)
{
	m_pos = pos;
	m_rtp_clock = 0;
	return 0;
}

int RtspCameraSource::SetSpeed(double speed)
{
	m_speed = speed;
	return 0;
}

int RtspCameraSource::GetDuration(int64_t& duration) const
{
	return m_reader.GetDuration(duration);
}

int RtspCameraSource::GetSDPMedia(std::string& sdp) const
{
    static const char* pattern =
        "m=video 0 RTP/AVP %d\n"
        "a=rtpmap:%d H265/90000\n"
        "a=fmtp:%d profile-level-id=%02X%02X%02X;"
    			 "packetization-mode=1;"
    			 "sprop-parameter-sets=";

    char base64[512] = {0};
    std::string parameters;

    const std::list<std::pair<const uint8_t*, size_t> >& sps = m_reader.GetParameterSets();
    std::list<std::pair<const uint8_t*, size_t> >::const_iterator it;
    for(it = sps.begin(); it != sps.end(); ++it)
    {
        if(parameters.empty())
        {
            snprintf(base64, sizeof(base64), pattern,
				RTP_PAYLOAD_H265, RTP_PAYLOAD_H265,RTP_PAYLOAD_H265,
				(unsigned int)(it->first[1]), (unsigned int)(it->first[2]), (unsigned int)(it->first[3]));
            sdp = base64;
        }
        else
        {
            parameters += ',';
        }

        size_t bytes = it->second;
        assert((bytes+2)/3*4 + bytes/57 + 1 < sizeof(base64));
        bytes = base64_encode(base64, it->first, bytes);
		base64[bytes] = '\0';
        assert(strlen(base64) > 0);
        parameters += base64;
    }

    sdp += parameters;
    sdp += '\n';
    return sps.empty() ? -1 : 0;
}

int RtspCameraSource::GetRTPInfo(const char* uri, char *rtpinfo, size_t bytes) const
{
	uint16_t seq;
	uint32_t timestamp;
	rtp_payload_encode_getinfo(m_rtppacker, &seq, &timestamp);

	// url=rtsp://video.example.com/twister/video;seq=12312232;rtptime=78712811
	snprintf(rtpinfo, bytes, "url=%s;seq=%hu;rtptime=%u", uri, seq, timestamp);
	return 0;
}

void RtspCameraSource::OnRTCPEvent(const struct rtcp_msg_t* msg)
{
	(void)msg;
}

void RtspCameraSource::OnRTCPEvent(void* param, const struct rtcp_msg_t* msg)
{
	RtspCameraSource *self = (RtspCameraSource *)param;
	self->OnRTCPEvent(msg);
}

int RtspCameraSource::SendRTCP()
{
	// make sure have sent RTP packet

	time64_t clock = time64_now();
	int interval = rtp_rtcp_interval(m_rtp);
	if(0 == m_rtcp_clock || m_rtcp_clock + interval < clock)
	{
		char rtcp[1024] = {0};
		size_t n = rtp_rtcp_report(m_rtp, rtcp, sizeof(rtcp));

		// send RTCP packet
		m_transport->Send(true, rtcp, n);

		m_rtcp_clock = clock;
	}

	return 0;
}

void* RtspCameraSource::RTPAlloc(void* param, int bytes)
{
	RtspCameraSource *self = (RtspCameraSource*)param;
	assert(bytes <= (int)sizeof(self->m_packet));
	return self->m_packet;
}

void RtspCameraSource::RTPFree(void* param, void *packet)
{
	RtspCameraSource *self = (RtspCameraSource*)param;
	assert(self->m_packet == packet);
}

int RtspCameraSource::RTPPacket(void* param, const void *packet, int bytes, uint32_t /*timestamp*/, int /*flags*/)
{
	RtspCameraSource *self = (RtspCameraSource*)param;
	assert(self->m_packet == packet);
	// const char* ptr = (const char*)packet;
	// for(int i=0;i<bytes;i++)
	// 	printf("%02x ",ptr[i]);
	// printf("RTPPacket over\n\n\n");
	// if(i++ > 4){
	// 	exit(-1);
	// }
	int r = self->m_transport->Send(false, packet, bytes);
	if (r != bytes)
		return -1;

	return rtp_onsend(self->m_rtp, packet, bytes/*, time*/);
}
