#include "maix_bytetrack.hpp"
#include "ByteTrack/BYTETracker.h"
#include "maix_basic.hpp"

namespace maix::tracker
{
    ByteTracker::ByteTracker(const int& max_lost_buff_num,
                const float& track_thresh,
                const float& high_thresh,
                const float& match_thresh,
                const int& max_history)
    {
        _data = new byte_track::BYTETracker(max_lost_buff_num, track_thresh, high_thresh, match_thresh, max_history);
        if(!_data)
            throw err::Exception(err::ERR_NO_MEM);
    }

    ByteTracker::~ByteTracker()
    {
        delete (byte_track::BYTETracker*)_data;
    }

    std::vector<tracker::Track> ByteTracker::update(const std::vector<tracker::Object> &objs)
    {
        byte_track::BYTETracker *bytetracker = (byte_track::BYTETracker*)_data;
        std::vector<tracker::Track> res;
        std::vector<byte_track::Object> objs2;
        for(const auto &obj : objs)
        {
            byte_track::Rect<float> rect(obj.x, obj.y, obj.w, obj.h);
            objs2.push_back(byte_track::Object(rect, obj.class_id, obj.score));
        }
        const auto &res0 = bytetracker->update(objs2);
        for (const auto &r : res0)
        {
            res.push_back(tracker::Track(r->getTrackId(), r->getScore(), r->getLost(), r->getStartFrameId(), r->getFrameId()));
            tracker::Track &track = res.back();
            const std::deque<byte_track::Object> &history = r->get_rect_history();
            for(const auto &i : history)
            {
                track.history.push_back(tracker::Object(i.rect.x(), i.rect.y(), i.rect.width(), i.rect.height(), i.label, i.prob));
            }
        }
        return res;
    }
} // namespace maix::tracker



