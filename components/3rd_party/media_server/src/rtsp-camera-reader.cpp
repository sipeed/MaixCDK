#include "rtsp-camera-reader.h"
#include <assert.h>
#include <string.h>
#include <algorithm>
#include <pthread.h>
#include <list>
#include "time64.h"

#define H265_NAL(v)	 ((v>> 1) & 0x3f)

enum { NAL_IDR_W_RADL = 19, NAL_IDR_N_LP= 20, NAL_VPS = 32, NAL_SPS = 33, NAL_PPS = 34, NAL_SEI = 39};

RtspCameraReader::RtspCameraReader(const char* file)
:m_ptr(NULL), m_capacity(0), m_first_time(0)
{
	pthread_mutex_init(&m_lock, NULL);
	pthread_mutex_unlock(&m_lock);
	m_vit = m_videos.begin();
	m_first_pop = 1;
}

RtspCameraReader::~RtspCameraReader()
{
	if (m_ptr)
	{
		assert(m_capacity > 0);
		free(m_ptr);
	}

	pthread_mutex_destroy(&m_lock);
}

bool RtspCameraReader::IsOpened() const
{
	return true;
}

int RtspCameraReader::Seek(int64_t &dts)
{
	return 0;
}

static inline const uint8_t* search_start_code(const uint8_t* ptr, const uint8_t* end)
{
    for(const uint8_t *p = ptr; p + 3 < end; p++)
    {
        if(0x00 == p[0] && 0x00 == p[1] && (0x01 == p[2] || (0x00==p[2] && 0x01==p[3])))
            return p;
    }
	return end;
}

static inline int h265_nal_type(const unsigned char* ptr)
{
    int i = 2;
    assert(0x00 == ptr[0] && 0x00 == ptr[1]);
    if(0x00 == ptr[2])
        ++i;
    assert(0x01 == ptr[i]);
    return H265_NAL(ptr[i+1]);
}

int RtspCameraReader::SetPspFromFrame(const uint8_t* nalu, size_t bytes)
{
	if(m_sps.size() == 0) {
		int nal_unit_type = h265_nal_type(nalu);
		if (0 > nal_unit_type) {
			return -1;
		}

		const uint8_t *nalu2 = (uint8_t *)nalu;
		const uint8_t* end = nalu2 + bytes;
		const uint8_t* p = nalu2;
		while (p < end)
		{
			const unsigned char* pn = search_start_code(p + 4, end);
			size_t size = pn - nalu2;

			int nal_unit_type = h265_nal_type(p);
			if (nal_unit_type < 0) {
				return -1;
			}

			if(NAL_SPS == nal_unit_type)
			{
				memcpy(m_sps_nalu, nalu2, size);
				size_t n = 0x01 == nalu2[2] ? 3 : 4;
				std::pair<const uint8_t*, size_t> pr;
				pr.first = m_sps_nalu + n;
				pr.second = size;
				m_sps.push_back(pr);
				break;
			}

			nalu2 = pn;
			p = pn;
		}

		PushNextFrame(0, nalu, bytes);
	}
	return 0;
}

int RtspCameraReader::GetNextFrame(int64_t &dts, const uint8_t* &ptr, size_t &bytes)
{
	pthread_mutex_lock(&m_lock);
	vframes_t::iterator frame;

	if (m_first_pop) {
		while (1) {
			frame = m_videos.begin();
			if(frame == m_videos.end()) {
				pthread_mutex_unlock(&m_lock);
				return -1;
			}

			int nal_unit_type = h265_nal_type(frame->nalu);
			if (0 > nal_unit_type) {
				m_videos.pop_front();
				continue;
			}

			if(NAL_VPS == nal_unit_type || NAL_SPS == nal_unit_type || NAL_PPS == nal_unit_type)
			{
				m_first_pop = 0;
				break;
			} else {
				m_videos.pop_front();
				continue;
			}
		}
	} else {
		frame = m_videos.begin();
		if(frame == m_videos.end()) {
			pthread_mutex_unlock(&m_lock);
			return -1;
		}
	}

	ptr = frame->nalu;
	dts = frame->time;
	bytes = frame->bytes;

	pthread_mutex_unlock(&m_lock);
	return 0;
}

int RtspCameraReader::PushNextFrame(int64_t time, const uint8_t* nalu, size_t bytes)
{
	pthread_mutex_lock(&m_lock);
	int nal_unit_type = h265_nal_type(nalu);
	if (0 > nal_unit_type) {
		pthread_mutex_unlock(&m_lock);
		return -1;
	}

	if(m_sps.size() == 0) {
		const uint8_t *nalu2 = (uint8_t *)nalu;
		const uint8_t* end = nalu2 + bytes;
		const uint8_t* p = nalu2;
		while (p < end)
		{
			const unsigned char* pn = search_start_code(p + 4, end);
			size_t size = pn - nalu2;

			int nal_unit_type = h265_nal_type(p);
			if (nal_unit_type < 0) {
				return -1;
			}

			if(NAL_SPS == nal_unit_type)
			{
				memcpy(m_sps_nalu, nalu2, size);
				size_t n = 0x01 == nalu2[2] ? 3 : 4;
				std::pair<const uint8_t*, size_t> pr;
				pr.first = m_sps_nalu + n;
				pr.second = size;
				m_sps.push_back(pr);
				break;
			}

			nalu2 = pn;
			p = pn;
		}
		m_first_time = time;
	}

	vframe_t frame;
	frame.nalu = (const uint8_t*)malloc(bytes);
	memcpy((uint8_t *)frame.nalu, nalu, bytes);
	frame.bytes = bytes;
	frame.time = (time == 0) ? 0 : time - m_first_time;
	frame.idr = (NAL_IDR_N_LP == nal_unit_type || NAL_IDR_W_RADL == nal_unit_type); // IDR-frame
	m_videos.push_back(frame);
	pthread_mutex_unlock(&m_lock);

	return 0;
}

int RtspCameraReader::FreeNextFrame()
{
	pthread_mutex_lock(&m_lock);
	vframes_t::iterator frame = m_videos.begin();
	if(frame == m_videos.end()) {
		pthread_mutex_unlock(&m_lock);
		return -1;
	}

	if (frame->nalu) {
		free((uint8_t *)frame->nalu);
	}
	m_videos.pop_front();
	pthread_mutex_unlock(&m_lock);

	return 0;
}

int RtspCameraReader::Init()
{
#if 0
    size_t count = 0;
    bool vpsspspps = true;

	const uint8_t* end = m_ptr + m_capacity;
    const uint8_t* nalu = search_start_code(m_ptr, end);
	const uint8_t* p = nalu;

	while (p < end)
	{
        const unsigned char* pn = search_start_code(p + 4, end);
		size_t bytes = pn - nalu;

        int nal_unit_type = h265_nal_type(p);
		assert(0 <= nal_unit_type);

        if(NAL_VPS == nal_unit_type || NAL_SPS == nal_unit_type || NAL_PPS == nal_unit_type)
        {
            if(vpsspspps)
            {
                size_t n = 0x01 == p[2] ? 3 : 4;
				std::pair<const uint8_t*, size_t> pr;
				pr.first = p + n;
				pr.second = bytes;
				m_sps.push_back(pr);
            }
        }

        {
            if(m_sps.size() > 0) vpsspspps = false; // don't need more vps/sps/pps

			vframe_t frame;
			frame.nalu = nalu;
			frame.bytes = bytes;
			frame.time = 40 * count++;
			frame.idr = (NAL_IDR_N_LP == nal_unit_type || NAL_IDR_W_RADL == nal_unit_type); // IDR-frame
			m_videos.push_back(frame);
			nalu = pn;
        }

        p = pn;
    }

    m_duration = 40 * count;
    return 0;
#else
    // size_t count = 0;

	// const uint8_t* end = m_ptr + m_capacity;
    // const uint8_t* nalu = search_start_code(m_ptr, end);
	// const uint8_t* p = nalu;

	// while (p < end)
	// {
    //     const unsigned char* pn = search_start_code(p + 4, end);
	// 	size_t bytes = pn - nalu;

	// 	PushNextFrame(40 * count++, nalu, bytes);

	// 	nalu = pn;
    //     p = pn;
    // }

    // m_duration = 40 * count;
    return 0;
#endif
}
