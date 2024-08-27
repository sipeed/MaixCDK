#pragma once

#include "maix_tracker.hpp"
#include <vector>

namespace maix::tracker
{
    /**
     * tracker.ByteTracker class
     * @maixpy maix.tracker.ByteTracker
     */
    class ByteTracker
    {
    public:
        /**
         * tracker.ByteTracker class constructor
         * @param max_lost_buff_num the frames for keep lost tracks.
         * * @param track_thresh tracking confidence threshold.
         * * @param high_thresh threshold to add to new track.
         * * @param match_thresh matching threshold for tracking, e.g. one object in two frame iou < match_thresh we think they are the same obj.
         * * @param max_history max tack's position history length.
         * @maixpy maix.tracker.ByteTracker.__init__
         * @maixcdk maix.tracker.ByteTracker.ByteTracker
         */
        ByteTracker(const int &max_lost_buff_num = 60,
                    const float &track_thresh = 0.5,
                    const float &high_thresh = 0.6,
                    const float &match_thresh = 0.8,
                    const int &max_history = 20);
        ~ByteTracker();

        /**
         * update tracks according to current detected objects.
         * @maixpy maix.tracker.ByteTracker.update
         */
        std::vector<tracker::Track> update(const std::vector<tracker::Object> &objs);

    private:
        void *_data;
    };
} // namespace maix::tracker
