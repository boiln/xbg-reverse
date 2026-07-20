#include "luda/diagnostics/logger.hpp"

#include <ppcintrinsics.h>
#include <xtl.h>

#if XBG_LOG_ENABLED

extern "C" int __stdcall ExCreateThread(HANDLE*, unsigned long, unsigned long*, void*, unsigned long(__stdcall*)(void*),
                                        void*, unsigned long);
extern "C" void XapiThreadStartup(void);

namespace xbglog {
    static const u32 kSlotCount = 256;
    static const u32 kSlotMask = kSlotCount - 1;

    struct Slot {
        volatile LONG state;
        u32 sequence;
        u32 tick;
        u32 id;
        u32 a;
        u32 b;
        u32 c;
        u32 d;
    };

    static Slot s_slots[kSlotCount];
    static volatile LONG s_probe = -1;
    static volatile LONG s_sequence = 0;
    static volatile LONG s_dropped = 0;
    static volatile LONG s_epoch = 0;
    static volatile bool s_running = false;
    static volatile bool s_exited = true;
    static HANDLE s_thread = 0;

    static const char* Name(u32 id) {
        switch (id) {
            case EV_INSTANCE:
                return "INSTANCE";
            case EV_AW_BEGIN:
                return "AW_BEGIN";
            case EV_AW_INITIAL_PRE:
                return "AW_INITIAL_PRE";
            case EV_AW_INITIAL_POST:
                return "AW_INITIAL_POST";
            case EV_AW_RETRACE_PRE:
                return "AW_RETRACE_PRE";
            case EV_AW_RETRACE_POST:
                return "AW_RETRACE_POST";
            case EV_AW_WALL:
                return "AW_WALL";
            case EV_AW_ADVANCE_PRE:
                return "AW_ADVANCE_PRE";
            case EV_AW_ADVANCE_POST:
                return "AW_ADVANCE_POST";
            case EV_AW_FORWARD_PRE:
                return "AW_FORWARD_PRE";
            case EV_AW_FORWARD_POST:
                return "AW_FORWARD_POST";
            case EV_AW_REVERSE_BUILT:
                return "AW_REVERSE_BUILT";
            case EV_AW_REVERSE_ADV_PRE:
                return "AW_REVERSE_ADV_PRE";
            case EV_AW_REVERSE_ADV_POST:
                return "AW_REVERSE_ADV_POST";
            case EV_AW_REVERSE_PRE:
                return "AW_REVERSE_PRE";
            case EV_AW_REVERSE_POST:
                return "AW_REVERSE_POST";
            case EV_AW_BUDGET:
                return "AW_BUDGET";
            case EV_AW_RESULT:
                return "AW_RESULT";
        }

        return "UNKNOWN";
    }

    static char* PutText(char* p, const char* text) {
        while (*text) *p++ = *text++;

        return p;
    }

    static char* PutHex(char* p, u32 value) {
        static const char digits[] = "0123456789ABCDEF";
        for (int shift = 28; shift >= 0; shift -= 4) *p++ = digits[(value >> shift) & 0xF];

        return p;
    }

    static char* Format(char* p, const Slot& slot) {
        p = PutHex(p, slot.sequence);
        *p++ = ' ';
        p = PutHex(p, slot.tick);
        *p++ = ' ';
        p = PutText(p, Name(slot.id));
        p = PutText(p, " a=");
        p = PutHex(p, slot.a);
        p = PutText(p, " b=");
        p = PutHex(p, slot.b);
        p = PutText(p, " c=");
        p = PutHex(p, slot.c);
        p = PutText(p, " d=");
        p = PutHex(p, slot.d);
        *p++ = '\r';
        *p++ = '\n';

        return p;
    }

    static void ClearRing() {
        for (u32 i = 0; i < kSlotCount; ++i) s_slots[i].state = 0;
        s_dropped = 0;
    }

    static unsigned long __stdcall Writer(void*) {
        s_exited = false;
        HANDLE file = INVALID_HANDLE_VALUE;
        LONG openedEpoch = 0;
        u32 scan = 0;
        u32 lastFlush = GetTickCount();
        char batch[8192];

        while (s_running) {
            const LONG wantedEpoch = s_epoch;
            if (wantedEpoch != openedEpoch) {
                if (file != INVALID_HANDLE_VALUE) {
                    FlushFileBuffers(file);
                    CloseHandle(file);
                }
                ClearRing();
                file = CreateFileA("Hdd:\\xbg.log", GENERIC_WRITE, FILE_SHARE_READ, 0, CREATE_ALWAYS,
                                   FILE_ATTRIBUTE_NORMAL, 0);
                openedEpoch = wantedEpoch;
                lastFlush = GetTickCount();
                if (file != INVALID_HANDLE_VALUE) {
                    static const char header[] = "xbg.log async trace - seq tick event a b c d\r\n";
                    DWORD wrote = 0;
                    WriteFile(file, header, sizeof(header) - 1, &wrote, 0);

                    FlushFileBuffers(file);
                }
            }

            char* out = batch;
            for (u32 checked = 0; checked < kSlotCount; ++checked) {
                Slot& slot = s_slots[scan++ & kSlotMask];
                if (slot.state != 2) continue;
                out = Format(out, slot);
                __lwsync();
                InterlockedExchange(&slot.state, 0);
                if ((u32)(out - batch) > sizeof(batch) - 128) break;
            }

            if (out != batch && file != INVALID_HANDLE_VALUE) {
                DWORD wrote = 0;
                WriteFile(file, batch, (DWORD)(out - batch), &wrote, 0);

                FlushFileBuffers(file);
                lastFlush = GetTickCount();
            }

            const u32 now = GetTickCount();
            if (file != INVALID_HANDLE_VALUE && now - lastFlush >= 50) {
                FlushFileBuffers(file);
                lastFlush = now;
            }
            if (out == batch) Sleep(2);
        }

        if (file != INVALID_HANDLE_VALUE) {
            char* out = batch;
            for (u32 i = 0; i < kSlotCount; ++i) {
                Slot& slot = s_slots[i];
                if (slot.state != 2) continue;
                out = Format(out, slot);
                slot.state = 0;
                if ((u32)(out - batch) > sizeof(batch) - 128) {
                    DWORD wrote = 0;
                    WriteFile(file, batch, (DWORD)(out - batch), &wrote, 0);
                    out = batch;
                }
            }
            if (out != batch) {
                DWORD wrote = 0;
                WriteFile(file, batch, (DWORD)(out - batch), &wrote, 0);
            }
            FlushFileBuffers(file);
            CloseHandle(file);
        }
        s_exited = true;

        return 0;
    }

    void Event(EventId id, u32 a, u32 b, u32 c, u32 d) {
        if (!s_running || s_epoch == 0) return;

        const LONG first = InterlockedIncrement(&s_probe);
        for (u32 attempt = 0; attempt < 4; ++attempt) {
            Slot& slot = s_slots[(first + attempt) & kSlotMask];
            if (InterlockedCompareExchange(&slot.state, 1, 0) != 0) continue;
            slot.sequence = (u32)InterlockedIncrement(&s_sequence);
            slot.tick = GetTickCount();
            slot.id = (u32)id;
            slot.a = a;
            slot.b = b;
            slot.c = c;
            slot.d = d;
            __lwsync();
            InterlockedExchange(&slot.state, 2);
            return;
        }
        InterlockedIncrement(&s_dropped);
    }

    void Start() {
        if (s_running) return;
        ClearRing();
        s_epoch = 0;
        s_running = true;
        s_exited = false;
        unsigned long tid = 0;

        if (ExCreateThread(&s_thread, 0x10000, &tid, (void*)XapiThreadStartup, Writer, 0, 0x2) < 0) {
            s_thread = 0;
            s_running = false;
            s_exited = true;
        }
    }

    void BeginGameInstance() {
        if (!s_running) return;
        InterlockedIncrement(&s_epoch);
    }

    void Stop() {
        if (!s_running && !s_thread) return;
        s_running = false;

        if (s_thread) {
            WaitForSingleObject(s_thread, 4000);
            CloseHandle(s_thread);
            s_thread = 0;
        }
        for (int i = 0; i < 400 && !s_exited; ++i) Sleep(5);
    }
}

#endif
