#include "internal.hpp"

namespace reconrender {
    static void Push(const Option& o) {
        if (g_optCount < kMaxOptions) g_options[g_optCount++] = o;
    }
    void addToggle(const char* l, u32 va) {
        Option o = {l, K_TOGGLE, Bp(va), 0, 0, 0, 0, 0, 0, 0, 0};
        Push(o);
    }
    void addEnum(const char* l, const char** n, int c, u32 va) {
        Option o = {l, K_ENUM, Bp(va), 0, n, c, 0, 0, 0, 0, 0};
        Push(o);
    }
    void addEnumAction(const char* l, const char** n, int c, u32 va, int action) {
        Option o = {l, K_ENUM, Bp(va), 0, n, c, 0, 0, 0, 0, action};
        Push(o);
    }
    void addEnumToggle(const char* l, const char** n, int c, u32 enumVa, u32 enableVa) {
        Option o = {l, K_ENUM_TOGGLE, Bp(enumVa), 0, n, c, 0, 0, 0, 0, 0, Bp(enableVa)};
        Push(o);
    }
    void addSlider(const char* l, u32 va, float lo, float hi, float st, int fmt) {
        Option o = {l, K_SLIDER, 0, Fp(va), 0, 0, lo, hi, st, fmt, 0};
        Push(o);
    }
    void addSliderToggle(const char* l, u32 va, u32 enableVa, float lo, float hi, float st, int fmt) {
        Option o = {l, K_SLIDER_TOGGLE, 0, Fp(va), 0, 0, lo, hi, st, fmt, 0, Bp(enableVa)};
        Push(o);
    }
    static void addColor(const char* l, u32 va) {
        Option o = {l, K_COLOR, 0, Fp(va), 0, 0, 0, 0, 0, 0, 0};
        Push(o);
    }
    void addAction(const char* l, int act) {
        Option o = {l, K_ACTION, 0, 0, 0, 0, 0, 0, 0, 0, act};
        Push(o);
    }
    void addPlainAction(const char* l, int act) {
        Option o = {l, K_ACTION_PLAIN, 0, 0, 0, 0, 0, 0, 0, 0, act};
        Push(o);
    }
    void addInfo(const char* l) {
        Option o = {l, K_INFO, 0, 0, 0, 0, 0, 0, 0, 0, 0};
        Push(o);
    }

    void buildColorEditor(const char* title, u32 rgba, u32 rainbowVa, u32 enabledVa) {

        if (enabledVa) addToggle("Enabled", enabledVa);
        if (rainbowVa) addToggle("Rainbow", rainbowVa);

        addSlider("Red", rgba + 0, 0.0f, 1.0f, 0.003f, FMT_255);
        addSlider("Green", rgba + 4, 0.0f, 1.0f, 0.003f, FMT_255);
        addSlider("Blue", rgba + 8, 0.0f, 1.0f, 0.003f, FMT_255);
        addSlider("Alpha", rgba + 12, 0.0f, 1.0f, 0.003f, FMT_255);
        addColor(title, rgba);

    }

    bool ContainsStr(const char* hay, const char* needle) {

        for (const char* h = hay; *h; ++h) {
            const char* a = h;
            const char* b = needle;
            while (*a && *b && *a == *b) {
                ++a;
                ++b;
            }
            if (!*b) return true;
        }

        return false;

    }
    bool SameStr(const char* a, const char* b) {

        if (!a || !b) return false;

        while (*a && *b && *a == *b) {
            ++a;
            ++b;
        }

        return *a == *b;

    }

    static bool IsModderName(const char* n) {

        static const char* const kSig[] = {
            "XBOX360LSBEST",
            "^6J^5i^6g^5g^6y",
            "^1B^5at^1M^5an",
            "TCM",
            "^5C^3h^5e^3a^5t^3s",
            "XBL^5Pony",
            "^5J^1i^5g^1g^5y",
            "ELeGanCe",
            "Bossam",
            "Predator",
            "by: Matrix",
            "Serenity",
            "xePixTvx",
            "Project Iconic",
            "Conversion",
            "Zombieland",
            "Velocity",
            "Kamil_Modz",
            "RM|T Zombies Menu V2.6",
        };
        for (unsigned k = 0; k < sizeof(kSig) / sizeof(kSig[0]); ++k)
            if (ContainsStr(n, kSig[k])) return true;

        return false;

    }

    LocalIdSet s_priorityIds = {{0}, 0};
    LocalIdSet s_whitelistIds = {{0}, 0};
    LocalIdSet s_detectedIds = {{0}, 0};

    u64 PlayerXuid(int slot) {

        if (slot < 0 || slot >= 18) return 0;
        __try {
            return *(volatile u64*)(u64)(A_CLIENT_INFO + (u32)slot * CLIENT_INFO_STRIDE + 0x38u);
        } __except (EXCEPTION_EXECUTE_HANDLER) {
            return 0;
        }

    }

    const char* ClientTableName(int slot) {

        if (slot < 0 || slot >= 18) return 0;
        const char* name = (const char*)(u64)(A_CLIENT_INFO + (u32)slot * CLIENT_INFO_STRIDE + 0x40u);
        __try {
            return name[0] ? name : 0;
        } __except (EXCEPTION_EXECUTE_HANDLER) {
            return 0;
        }

    }

    u32 PlayerMenuCg() {

        __try {
            return *(volatile u32*)(u64)A_CG_POINTER;
        } __except (EXCEPTION_EXECUTE_HANDLER) {
            return 0;
        }

    }

    bool PlayerMenuIsHost() {

        __try {
            if (*(volatile u32*)(u64)0x82C6FDD0u != 10u) return false;

            u32 session = *(volatile u32*)(u64)0x82C6FDC4u;
            if (!session) return false;
            if (*(volatile u8*)(u64)(session + 0x18u) == 0) return false;

            typedef u8(__cdecl * HostBlockFn)();

            if (((HostBlockFn)(u64)0x827504D0u)() != 0) return false;

            return (*(volatile u32*)(u64)0x82C6FDC8u & 0x30u) != 0;
        } __except (EXCEPTION_EXECUTE_HANDLER) {
            return false;
        }

    }

    bool PlayerMenuHostMode(u32 cg) {

        if (!PlayerMenuIsHost() || !cg) return false;
        __try {
            return *(volatile u32*)(u64)(cg + 0x4808Cu) != 0;
        } __except (EXCEPTION_EXECUTE_HANDLER) {
            return false;
        }

    }

    int PlayerMenuHostIndex() {

        typedef u32(__cdecl * PartyGetActiveFn)();
        typedef int(__cdecl * PartyHostIndexFn)(u32);

        __try {
            u32 party = ((PartyGetActiveFn)(u64)A_PartyGetActive)();
            if (!party) return -1;

            int slot = ((PartyHostIndexFn)(u64)0x825B6BA0u)(party);

            return slot >= 0 && slot < 18 ? slot : -1;
        } __except (EXCEPTION_EXECUTE_HANDLER) {
            return -1;
        }

    }

    static bool HostSlotOccupied(u32 cg, int slot) {

        if (!cg || slot < 0 || slot >= 18) return false;
        __try {
            u32 entry = cg + CG_CLIENTINFO + (u32)slot * CG_CLIENTINFO_STRIDE;
            return *(volatile u32*)(u64)entry != 0;
        } __except (EXCEPTION_EXECUTE_HANDLER) {
            return false;
        }

    }

    const char* HostSlotName(u32 cg, int slot) {
        if (!HostSlotOccupied(cg, slot)) return 0;

        return (const char*)(u64)(cg + CG_CLIENTINFO + (u32)slot * CG_CLIENTINFO_STRIDE + 0x0Cu);
    }

    static bool ValidXenonPtr(u32 p) { return p >= 0x80000000u && p < 0xC0000000u; }

    bool DetectedModderXuid(u64 xuid) {

        if (RealEngineResident()) {
            __try {
                const u64* it = *(const u64**)(u64)0x90B4A6D0u;
                const u64* end = *(const u64**)(u64)0x90B4A6D4u;

                u32 beginVa = (u32)(u64)it;
                u32 endVa = (u32)(u64)end;

                if (beginVa == endVa) return false;

                if (ValidXenonPtr(beginVa) && ValidXenonPtr(endVa) && endVa >= beginVa &&
                    endVa - beginVa <= 256u * sizeof(u64)) {
                    for (; it != end; ++it)
                        if (*it == xuid) return true;
                    return false;
                }
            } __except (EXCEPTION_EXECUTE_HANDLER) {
            }
        }

        if (xuid)
            for (int i = 0; i < s_detectedIds.count; ++i)
                if (s_detectedIds.ids[i] == xuid) return true;

        return false;

    }

    static bool LocalSetContains(const LocalIdSet& set, u64 id) {
        for (int i = 0; i < set.count; ++i)
            if (set.ids[i] == id) return true;

        return false;
    }

    static void LocalSetWrite(LocalIdSet& set, u64 id, bool enabled) {

        int at = -1;
        for (int i = 0; i < set.count; ++i)
            if (set.ids[i] == id) {
                at = i;
                break;
            }

        if (enabled) {
            if (at < 0 && set.count < 18) set.ids[set.count++] = id;
            return;
        }
        if (at < 0) return;
        for (int i = at; i + 1 < set.count; ++i) set.ids[i] = set.ids[i + 1];
        set.ids[--set.count] = 0;

    }

    bool PersistentSetContains(u32 setVa, LocalIdSet& fallback, u64 id) {

        if (!id) return false;

        if (RealEngineResident()) {
            __try {
                u32 head = *(volatile u32*)(u64)(setVa + 4u);
                if (!ValidXenonPtr(head)) return LocalSetContains(fallback, id);

                u32 found[2] = {0, 0};

                typedef void(__cdecl * Find_t)(u32*, u32, const u64*);

                ((Find_t)(u64)0x90B23DC8u)(found, setVa, &id);

                return found[0] != head;
            } __except (EXCEPTION_EXECUTE_HANDLER) {
            }
        }

        return LocalSetContains(fallback, id);

    }

    bool PersistentSetWrite(u32 setVa, LocalIdSet& fallback, u64 id, bool enabled) {

        if (!id) return false;

        if (RealEngineResident()) {
            __try {
                u32 head = *(volatile u32*)(u64)(setVa + 4u);
                if (!ValidXenonPtr(head)) {
                    // the local set preserves mutations when the resident tree is unavailable.
                    LocalSetWrite(fallback, id, enabled);
                    return true;
                }
                if (enabled) {
                    typedef void*(__cdecl * MakeNode_t)(u32, const u64*);
                    typedef void*(__cdecl * Insert_t)(u32*, u32, void*);

                    void* node = ((MakeNode_t)(u64)0x90B243D8u)(setVa, &id);
                    if (!node) return false;

                    u32 result[2] = {0, 0};

                    ((Insert_t)(u64)0x90B23F70u)(result, setVa, node);
                } else {
                    typedef u64(__cdecl * Erase_t)(u32, const u64*);

                    ((Erase_t)(u64)0x90B23D58u)(setVa, &id);
                }
                return true;
            } __except (EXCEPTION_EXECUTE_HANDLER) {
                return false;
            }
        }
        LocalSetWrite(fallback, id, enabled);

        return true;

    }

    void PersistentSetClear(u32 setVa, LocalIdSet& fallback) {

        if (RealEngineResident()) {
            __try {
                u32 head = *(volatile u32*)(u64)(setVa + 4u);
                if (ValidXenonPtr(head)) {
                    typedef void(__cdecl * DestroyTree_t)(u32, u32);

                    u32 root = *(volatile u32*)(u64)(head + 4u);

                    ((DestroyTree_t)(u64)0x90B09138u)(setVa, root);
                    // resetting every sentinel link leaves the resident tree empty and reusable.
                    *(volatile u32*)(u64)(head + 0u) = head;
                    *(volatile u32*)(u64)(head + 4u) = head;
                    *(volatile u32*)(u64)(head + 8u) = head;
                    *(volatile u32*)(u64)(setVa + 8u) = 0;
                }
            } __except (EXCEPTION_EXECUTE_HANDLER) {
            }
        }
        for (int i = 0; i < 18; ++i) fallback.ids[i] = 0;
        fallback.count = 0;

    }

    bool EngineClientArrayReady() {

        __try {
            return *(volatile u32*)(u64)0x83B50F40u != 0;
        } __except (EXCEPTION_EXECUTE_HANDLER) {
            return false;
        }

    }

    bool ForceStartEligible() {

        if (esp::InGame()) return false;

        typedef u32(__cdecl * PartyGetActiveFn)();
        typedef int(__cdecl * PartyHostIndexFn)(u32);
        typedef int(__cdecl * PartyLocalIndexFn)(int, u32);

        PartyGetActiveFn getActive = (PartyGetActiveFn)(u64)A_PartyGetActive;
        PartyHostIndexFn hostIndex = (PartyHostIndexFn)(u64)0x825B6BA0u;
        PartyLocalIndexFn localIndex = (PartyLocalIndexFn)(u64)0x825B7100u;

        __try {
            u32 party = getActive();
            if (!party) return false;

            int host = hostIndex(party);
            int local = localIndex(0, party);

            return host >= 0 && host == local;
        } __except (EXCEPTION_EXECUTE_HANDLER) {
        }

        return false;

    }
}
