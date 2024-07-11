#ifndef _rtsp_camera_reader_h_
#define _rtsp_camera_reader_h_

#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <list>
#include "ctypedef.h"
#include "pthread.h"

class RtspCameraReader
{
public:
	RtspCameraReader(const char* file);
	~RtspCameraReader();

	bool IsOpened() const;

public:
    const std::list<std::pair<const uint8_t*, size_t> > GetParameterSets() const { return m_sps; }
	int GetDuration(int64_t& duration) const { duration = m_duration; return 0; }
    int GetNextFrame(int64_t &dts, const uint8_t* &ptr, size_t &bytes);
	int FreeNextFrame();
	int PushNextFrame(int64_t time, const uint8_t* nalu, size_t bytes);
	int SetPspFromFrame(const uint8_t* nalu, size_t bytes);
	int Seek(int64_t &dts);

private:
	int Init();

private:
	struct vframe_t
	{
		const uint8_t* nalu;
		int64_t time;
		long bytes;
		bool idr; // IDR frame

		bool operator < (const struct vframe_t &v) const
		{
			return time < v.time;
		}
	};
    typedef std::list<vframe_t> vframes_t;
    vframes_t m_videos;
	vframes_t::iterator m_vit;
	pthread_mutex_t m_lock;

    std::list<std::pair<const uint8_t*, size_t> > m_sps;
	int64_t m_duration;

    uint8_t *m_ptr;
    size_t m_capacity;
	uint8_t m_first_pop;
	uint64_t m_first_time;
	uint8_t m_sps_nalu[512];
};

#endif /* !_rtsp_camera_reader_h_ */
