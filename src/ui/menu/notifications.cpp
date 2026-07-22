#include "internal.hpp"

namespace reconrender {
    static const char* s_detectMsg = 0;
    static u32 s_detectUntil = 0;
    static char s_notifyStorage[128];
    static void DispatchNotification(int ms) {

        s_detectMsg = 0;
        if (!*Bp(0x90B43394u)) return;

        if (*Bp(0x90B43398u) == 1) {
            *(volatile u32*)(u64)0x82281280u = 0x60000000u;
            GtFlush((void*)(u64)0x82281280u);

            typedef void(__cdecl * ToastFn)(int, const char*, const char*, const char*, int);

            ((ToastFn)(u64)0x82454800u)(0, "code_warning_fps", "luda v1.0.0", s_notifyStorage, ms);
            return;
        }

        s_detectMsg = s_notifyStorage;
        s_detectUntil = GetTickCount() + (u32)ms;

    }

    void NotifyMsg(const char* msg, int ms) {

        int i = 0;

        if (msg)
            for (; msg[i] && i < (int)sizeof(s_notifyStorage) - 1; ++i) s_notifyStorage[i] = msg[i];

        s_notifyStorage[i] = 0;
        DispatchNotification(ms);

    }

    void NotifyPlayerFull(const char* prefix, const char* name, const char* suffix, int ms) {

        int i = 0;

        if (prefix)
            for (; *prefix && i < (int)sizeof(s_notifyStorage) - 1; ++prefix) s_notifyStorage[i++] = *prefix;

        if (name)
            for (; *name && i < (int)sizeof(s_notifyStorage) - 1; ++name) s_notifyStorage[i++] = *name;

        if (suffix)
            for (; *suffix && i < (int)sizeof(s_notifyStorage) - 1; ++suffix) s_notifyStorage[i++] = *suffix;

        s_notifyStorage[i] = 0;
        DispatchNotification(ms);

    }

    void NotifyPlayer(const char* prefix, const char* name, int ms) { NotifyPlayerFull(prefix, name, 0, ms); }

    bool StrHas(const char* h, const char* n) {

        if (!h || !n) return false;
        for (const char* pp = h; *pp; ++pp) {
            const char* a = pp;
            const char* b = n;
            while (*a && *b && *a == *b) {
                ++a;
                ++b;
            }
            if (!*b) return true;
        }

        return false;

    }

    bool DetectMenu_Scan(bool showNoMenu) {

        if (!(*(void**)(u64)0x82BBAE68u)) {
            NotifyMsg("This must be used while In-Game!", 3000);
            return false;
        }

        typedef const char* (*GetName_t)(int);

        GetName_t GetPlayerName = (GetName_t)(u64)0x822628D8u;

        const char* found = 0;
        for (int i = 0; i <= 0xAF5 && !found; ++i) {
            const char* nm = GetPlayerName(i);
            if (!nm) continue;
            found = DetectMenuName(nm);
        }

        if (found) {
            NotifyMsg(found, 3000);
            return true;
        }
        if (showNoMenu) NotifyMsg("No Menu Detected", 3000);

        return false;

    }

    static bool s_autoDetectDone = false;
    static u8 s_autoDetectTry = 0;
    static void RememberDetectedHost() {

        typedef u32(__cdecl * PartyGetActiveFn)();
        typedef int(__cdecl * PartyHostIndexFn)(u32);

        PartyGetActiveFn getActive = (PartyGetActiveFn)(u64)A_PartyGetActive;
        PartyHostIndexFn hostIndex = (PartyHostIndexFn)(u64)0x825B6BA0u;

        u32 party = getActive();
        if (!party) return;

        int host = hostIndex(party);
        if (host < 0 || host >= 18) return;

        u64 xuid = PlayerXuid(host);
        if (!xuid) return;

        for (int i = 0; i < s_detectedIds.count; ++i)
            if (s_detectedIds.ids[i] == xuid) return;
        if (s_detectedIds.count < 18) s_detectedIds.ids[s_detectedIds.count++] = xuid;

    }

    static void AutoDetect_Tick() {

        bool eligible = esp::InGame() && *Bp(0x90B43BDCu);
        __try {
            if (*(volatile u32*)(u64)0x83B50F40u != 0) eligible = false;
        } __except (EXCEPTION_EXECUTE_HANDLER) {
            eligible = false;
        }

        if (!eligible) {
            s_autoDetectDone = false;
            s_autoDetectTry = 0;
            return;
        }

        if (s_autoDetectDone) return;
        if (s_autoDetectTry >= 6) return;

        bool found = DetectMenu_Scan(s_autoDetectTry >= 5);

        if (found) {
            RememberDetectedHost();
            s_autoDetectDone = true;
        }

        ++s_autoDetectTry;
        if (s_autoDetectTry >= 6) s_autoDetectDone = true;

    }

    static void NotifyDraw() {

        if (!*Bp(0x90B43394u) || !s_detectMsg || GetTickCount() >= s_detectUntil) return;

        char buf[96];
        int i = 0;

        for (const char* m = s_detectMsg; *m && i < 95; ++m) buf[i++] = *m;
        buf[i] = 0;

        if (*Bp(0x90B43398u) != 0) return;

        DrawTextS(buf, 60.0f, 60.0f, kColAccent);

    }

    void Frame() {

        ComputeLayout();
        if (!s_font) return;

        if (!g_cfgInit) {
            ConfigDefaults();
            esp::Init();
            g_cfgInit = true;
        }

        bool worldFrame = BeginWorldFrame();

        if (worldFrame) TickFps();
        __try {
            esp::SyncSessionMode();
        } __except (EXCEPTION_EXECUTE_HANDLER) {
        }

        __try {
            UpdatePalette();
        } __except (EXCEPTION_EXECUTE_HANDLER) {
        }

        if (worldFrame) {
            __try {
                esp::Render();
            } __except (EXCEPTION_EXECUTE_HANDLER) {
            }
            __try {
                aimbot::Frame();
            } __except (EXCEPTION_EXECUTE_HANDLER) {
            }
        }

        __try {
            ApplyMainConfig();
        } __except (EXCEPTION_EXECUTE_HANDLER) {
        }

        __try {
            AutoDetect_Tick();
        } __except (EXCEPTION_EXECUTE_HANDLER) {
        }

        __try {
            ApplySettingsConfig();
        } __except (EXCEPTION_EXECUTE_HANDLER) {
        }

        __try {
            ApplyGodMode();
        } __except (EXCEPTION_EXECUTE_HANDLER) {
        }

        if (g_open) {
            __try {
                RenderMenu();
                DrawLegend();
            } __except (EXCEPTION_EXECUTE_HANDLER) {
            }
        }
        __try {
            WatermarkHUD();
        } __except (EXCEPTION_EXECUTE_HANDLER) {
        }

        __try {
            NotifyDraw();
        } __except (EXCEPTION_EXECUTE_HANDLER) {
        }

    }
}
