#pragma once

#include <cstddef>
#include <deque>

namespace maix::tracker
{
    /**
     * tracker.Object class
     * @maixpy maix.tracker.Object
     */
    class Object
    {
    public:
        /**
         * tracker.Object class constructor
         * @maixpy maix.tracker.Object.__init__
         * @maixcdk maix.tracker.Object.Object
         */
        Object(const int &x, const int &y, const int &w, const int &h, const int &class_id, const float &score)
            : x(x), y(y), w(w), h(h), class_id(class_id), score(score)
        {
        }

        /**
         * position x attribute.
         * @maixpy maix.tracker.Object.x
         */
        int x;

        /**
         * position y attribute.
         * @maixpy maix.tracker.Object.y
         */
        int y;

        /**
         * position rectangle width.
         * @maixpy maix.tracker.Object.w
         */
        int w;

        /**
         * position rectangle height.
         * @maixpy maix.tracker.Object.h
         */
        int h;

        /**
         * object class id, int type.
         * @maixpy maix.tracker.Object.class_id
         */
        int class_id;

        /**
         * object score(prob).
         * @maixpy maix.tracker.Object.score
         */
        float score;
    };

    /**
     * tracker.Track class
     * @maixpy maix.tracker.Track
     */
    class Track
    {
    public:
        /**
         * tracker.Track class constructor
         * @maixpy maix.tracker.Track.__init__
         * @maixcdk maix.tracker.Track.Track
         */
        Track(const size_t &id, const float &score, const bool &lost, const size_t &start_frame_id, const size_t &frame_id)
            : id(id), score(score), lost(lost), start_frame_id(start_frame_id), frame_id(frame_id)
        {
        }

        /**
         * tracker.Track class constructor
         * @maixcdk maix.tracker.Track.Track
         */
        Track()
            : id(0), score(0), lost(false), start_frame_id(0), frame_id(0)
        {
        }

        /**
         * track id.
         * @maixpy maix.tracker.Track.id
         */
        size_t id;

        /**
         * track score(prob).
         * @maixpy maix.tracker.Track.score
         */
        float score;

        /**
         * whether this track lost.
         * @maixpy maix.tracker.Track.lost
         */
        bool lost;

        /**
         * track start frame id.
         * @maixpy maix.tracker.Track.start_frame_id
         */
        size_t start_frame_id;

        /**
         * track current frame id.
         * @maixpy maix.tracker.Track.frame_id
         */
        size_t frame_id;

        /**
         * track position history, the last one is latest position.
         * @maixpy maix.tracker.Track.history
         */
        std::deque<tracker::Object> history;
    };
} // namespace maix::tracker
