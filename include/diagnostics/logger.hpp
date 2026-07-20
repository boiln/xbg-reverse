#pragma once

#define XBG_LOG_ENABLED 0

namespace xbglog {
    typedef unsigned int u32;

    enum EventId {
        EV_INSTANCE = 1,
        EV_AW_BEGIN,
        EV_AW_INITIAL_PRE,
        EV_AW_INITIAL_POST,
        EV_AW_RETRACE_PRE,
        EV_AW_RETRACE_POST,
        EV_AW_WALL,
        EV_AW_ADVANCE_PRE,
        EV_AW_ADVANCE_POST,
        EV_AW_FORWARD_PRE,
        EV_AW_FORWARD_POST,
        EV_AW_REVERSE_BUILT,
        EV_AW_REVERSE_ADV_PRE,
        EV_AW_REVERSE_ADV_POST,
        EV_AW_REVERSE_PRE,
        EV_AW_REVERSE_POST,
        EV_AW_BUDGET,
        EV_AW_RESULT
    };

#if XBG_LOG_ENABLED
    // starts the asynchronous logger and owns its writer thread until stop.
    void Start();
    // starts a fresh game log epoch and clears any pending records.
    void BeginGameInstance();
    // drains pending records and joins the writer thread.
    void Stop();
    // queues one diagnostic event with four caller-defined fields.
    void Event(EventId id, u32 a, u32 b, u32 c, u32 d);
#else
    inline void Start() {}
    inline void BeginGameInstance() {}
    inline void Stop() {}
    inline void Event(EventId, u32, u32, u32, u32) {}
#endif
}
