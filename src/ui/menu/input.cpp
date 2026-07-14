#include "internal.hpp"

namespace reconrender {
    static u16 OpenComboMask() {
        static const u16 kMask[6] = {BTN_LEFT, BTN_X, BTN_RTHUMB, BTN_RIGHT, BTN_DOWN, BTN_UP};
        u32 idx = *Bp(0x90B43A64u);

        return (u16)(BTN_LT | kMask[idx < 6 ? idx : 5]);
    }
    static u16 g_prev = 0;
    static inline bool Hit(u16 now, u16 prev, u16 bit) { return (now & bit) && !(prev & bit); }

    void Input(u16 raw, u8 lt) {
        u16 b = raw;
        if (lt > 64) b |= BTN_LT;
        u16 prev = g_prev;
        u16 openCombo = OpenComboMask();

        if ((b & openCombo) == openCombo && (prev & openCombo) != openCombo) {
            g_open = !g_open;
            g_prev = b;
            return;
        }

        if (!g_open) {
            g_prev = b;
            return;
        }

        BuildPage(g_curPage);
        int page = g_curPage;
        int& sel = g_sel[page < PAGE_N ? page : 0];
        int n = g_optCount;

        if (Hit(b, prev, BTN_DOWN) && n) sel = (sel >= n - 1) ? 0 : sel + 1;
        if (Hit(b, prev, BTN_UP) && n) sel = (sel <= 0) ? n - 1 : sel - 1;

        if (Hit(b, prev, BTN_A) && n && sel < n) {
            Option& o = g_options[sel];
            if (o.kind == K_TOGGLE) {
                if (o.backByte) {
                    *o.backByte = !*o.backByte;
                    if (o.backByte >= Bp(0x90B433F1u) && o.backByte < Bp(0x90B43403u)) ApplyGodMode();
                }
            } else if (o.kind == K_ENUM_TOGGLE) {
                if (o.auxByte) *o.auxByte = !*o.auxByte;
            } else if (o.kind == K_SLIDER_TOGGLE) {
                if (o.auxByte) *o.auxByte = !*o.auxByte;
            } else if (o.kind == K_ENUM && o.action == ACT_APPLY_PRESET) {
                ApplyPresetConfig(o.backByte ? (int)*o.backByte : 0);
            } else if (o.kind == K_ACTION || o.kind == K_ACTION_PLAIN) {
                if (o.action >= ACT_SELPLAYER && o.action < ACT_SELPLAYER + 18) {
                    g_target = o.action - ACT_SELPLAYER;
                    if (RealEngineResident()) {
                        __try {
                            *(volatile u32*)(u64)0x90B443ECu = (u32)g_target;
                        } __except (EXCEPTION_EXECUTE_HANDLER) {
                        }
                    }
                    g_curPage = PAGE_PLAYERACT;
                    g_sel[PAGE_PLAYERACT] = 0;
                } else if (o.action >= 0 && o.action < PAGE_N) {
                    g_curPage = o.action;
                    g_sel[o.action] = 0;
                } else if (o.action == 0x1000) {
                    RunCmd("cmd sl;\r\n");
                } else if (o.action == 0x1001) {
                    char cmd[48];
                    int i = 0;
                    const char* prefix = "cmd mr ";
                    while (*prefix) cmd[i++] = *prefix++;
                    char id[16];
                    UtoA(*(volatile u32*)(u64)0x82C15758u, id);
                    for (int k = 0; id[k]; ++k) cmd[i++] = id[k];
                    const char* suffix = " 3 endround;\r\n";
                    while (*suffix) cmd[i++] = *suffix++;
                    cmd[i] = 0;
                    RunCmd(cmd);
                } else if (o.action == 0x1002) {
                    RunCmd("disconnect;\r\n");
                } else if (o.action == 0x1003) {
                    DetectMenu_Scan(true);
                } else if (o.action == 0x1004) {
                    if (ForceStartEligible()) {
                        *(volatile u32*)(u64)0x8227A50Cu = 0x60000000u;
                        *(volatile u32*)(u64)0x8227A4F4u = 0x60000000u;
                        GtFlush((void*)(u64)0x8227A50Cu);
                        GtFlush((void*)(u64)0x8227A4F4u);
                        RunCmd("xpartygo\n");
                        NotifyMsg("Started Game", 3000);
                    }
                } else if (o.action == 0x1021)
                    RunCmd("resetStats\n");
                else if (o.action == 0x1032)
                    SetMyGamertag(kFreezeGt);
                else if (o.action >= 0x1040 && o.action <= 0x104A)
                    SetMyGamertag(kPresetGt[o.action - 0x1040]);
                else if (o.action == ACT_PRIORITY_ADD || o.action == ACT_PRIORITY_REMOVE ||
                         o.action == ACT_WHITELIST_ADD || o.action == ACT_WHITELIST_REMOVE) {
                    u64 xuid = PlayerXuid(g_target);
                    bool priority = o.action == ACT_PRIORITY_ADD || o.action == ACT_PRIORITY_REMOVE;
                    bool enabled = o.action == ACT_PRIORITY_ADD || o.action == ACT_WHITELIST_ADD;
                    u32 setVa = priority ? 0x90B4A454u : 0x90B4A444u;
                    LocalIdSet& fallback = priority ? s_priorityIds : s_whitelistIds;
                    if (PersistentSetWrite(setVa, fallback, xuid, enabled)) {
                        *Bp((priority ? 0x90B433DFu : 0x90B433CDu) + (u32)g_target) = enabled ? 1 : 0;
                    }
                } else if (o.action == 0x1203) {
                    g_dbgLastAction = 0x1203;
                    __try {
                        u32 cg = PlayerMenuCg();
                        bool hostMode = PlayerMenuHostMode(cg);
                        const char* nm = hostMode ? HostSlotName(cg, g_target)
                                                  : (PlayerXuid(g_target) ? ClientTableName(g_target) : 0);
                        if (nm) {
                            SetMyGamertag(nm);
                            g_dbgLastResult = 1;
                        } else {
                            NotifyMsg("Failed to steal gamertag.", 3000);
                            g_dbgLastResult = 2;
                        }
                    } __except (EXCEPTION_EXECUTE_HANDLER) {
                        g_dbgLastResult = -1;
                    }
                } else if (o.action == 0x1200) {
                    g_dbgLastAction = 0x1200;
                    if (!RealEngineResident()) {
                        g_dbgLastResult = 2;
                    } else
                        __try {
                            const char* s =
                                ((const char*(__cdecl*)(const char*))(u64)0x90B2B188u)((const char*)(u64)0x90B03028u);
                            if (s) ((void(__cdecl*)(u64, const char*))(u64)0x90B09378u)((u64)g_target, s);
                            g_dbgLastResult = 1;
                        } __except (EXCEPTION_EXECUTE_HANDLER) {
                            g_dbgLastResult = -1;
                        }
                } else if (o.action == 0x1201) {
                    g_dbgLastAction = 0x1201;
                    if (!RealEngineResident()) {
                        g_dbgLastResult = 2;
                    } else
                        __try {
                            const char* s =
                                ((const char*(__cdecl*)(const char*))(u64)0x90B2B188u)((const char*)(u64)0x90B03030u);
                            if (s) ((void(__cdecl*)(u64, const char*))(u64)0x90B09378u)((u64)g_target, s);
                            g_dbgLastResult = 1;
                        } __except (EXCEPTION_EXECUTE_HANDLER) {
                            g_dbgLastResult = -1;
                        }
                } else if (o.action == 0x1202) {
                    g_dbgLastAction = 0x1202;
                    if (!RealEngineResident()) {
                        g_dbgLastResult = 2;
                    } else
                        __try {
                            typedef void(__cdecl * HostCmd_t)(u64, u8, u32, u32, u32);
                            HostCmd_t hc = (HostCmd_t)(u64)0x90B2B798u;
                            hc((u64)0x90B094A8u, 1, 100, (u32)g_target, 0);
                            hc((u64)0x90B094E8u, 1, 700, (u32)g_target, 0);
                            g_dbgLastResult = 1;
                        } __except (EXCEPTION_EXECUTE_HANDLER) {
                            g_dbgLastResult = -1;
                        }
                } else if (o.action == 0x1204) {
                    g_dbgLastAction = 0x1204;
                    if (!PlayerXuid(g_target)) {
                        g_dbgLastResult = 2;
                    } else
                        __try {
                            u32 entry = A_CLIENT_INFO + (u32)g_target * CLIENT_INFO_STRIDE;
                            u8* o1 = (u8*)(u64)(entry + 0xB4u);
                            u8* o2 = (u8*)(u64)(entry + 0xB5u);
                            u8* o3 = (u8*)(u64)(entry + 0xB6u);
                            u8* o4 = (u8*)(u64)(entry + 0xB7u);
                            const char* nm = ClientTableName(g_target);
                            if (o1 && o2 && o3 && o4 && nm) {
                                char gt[64];
                                int i = 0;
                                for (int k = 0; nm[k] && k < 15; ++k) gt[i++] = nm[k];
                                gt[i++] = '=';
                                char nb[12];
                                ItoA(*o1, nb);
                                for (int k = 0; nb[k]; ++k) gt[i++] = nb[k];
                                gt[i++] = '.';
                                ItoA(*o2, nb);
                                for (int k = 0; nb[k]; ++k) gt[i++] = nb[k];
                                gt[i++] = '.';
                                ItoA(*o3, nb);
                                for (int k = 0; nb[k]; ++k) gt[i++] = nb[k];
                                gt[i++] = '.';
                                ItoA(*o4, nb);
                                for (int k = 0; nb[k]; ++k) gt[i++] = nb[k];
                                gt[i] = 0;
                                SetMyGamertag(gt);
                            }
                            g_dbgLastResult = 1;
                        } __except (EXCEPTION_EXECUTE_HANDLER) {
                            g_dbgLastResult = -1;
                        }
                } else if (o.action == 0x1050) {
                    g_dbgLastAction = 0x1050;
                    __try {
                        g_dbgLastResult = BeginRenamePlayer(g_target) ? 1 : 2;
                    } __except (EXCEPTION_EXECUTE_HANDLER) {
                        g_dbgLastResult = -1;
                    }
                } else if (o.action == 0x1051) {
                    g_dbgLastAction = 0x1051;
                    __try {
                        int host = PlayerMenuHostIndex();
                        if (g_target == host) {
                            NotifyMsg("You cannot kick the host.", 3000);
                            g_dbgLastResult = 2;
                        } else {
                            u32 base = *(volatile u32*)(u64)0x83B50F40u;
                            if (!base) {
                                g_dbgLastResult = 2;
                            } else {
                                u64 addr = (u64)g_target * 0x4E100ull + (u64)base;
                                typedef void(__cdecl * Kick_t)(u64, const char*, u32);
                                ((Kick_t)(u64)0x8242D768u)(addr, "", 0u);
                                g_dbgLastResult = 1;
                            }
                        }
                    } __except (EXCEPTION_EXECUTE_HANDLER) {
                        g_dbgLastResult = -1;
                    }
                } else if (o.action == 0x1052) {
                    g_dbgLastAction = 0x1052;
                    __try {
                        SendCrashPlayerExact(g_target);
                        g_dbgLastResult = 1;
                    } __except (EXCEPTION_EXECUTE_HANDLER) {
                        g_dbgLastResult = -1;
                    }
                } else if (o.action == 0x1053) {
                    g_dbgLastAction = 0x1053;
                    __try {
                        char command[32];
                        int n = 0;
                        const char* prefix = "banClient \"";
                        while (*prefix) command[n++] = *prefix++;
                        char slot[12];
                        ItoA(g_target, slot);
                        for (int k = 0; slot[k]; ++k) command[n++] = slot[k];
                        command[n++] = '"';
                        command[n++] = '\n';
                        command[n] = 0;
                        RunCmd(command);
                        g_dbgLastResult = 1;
                    } __except (EXCEPTION_EXECUTE_HANDLER) {
                        g_dbgLastResult = -1;
                    }
                } else if (o.action == 0x1010) {
                    PersistentSetClear(0x90B4A454u, s_priorityIds);
                    for (int i = 0; i < 18; ++i) *Bp(0x90B433DFu + (u32)i) = 0;
                } else if (o.action == 0x1011) {
                    PersistentSetClear(0x90B4A444u, s_whitelistIds);
                    for (int i = 0; i < 18; ++i) *Bp(0x90B433CDu + (u32)i) = 0;
                }
            }
        }

        if (Hit(b, prev, BTN_B)) {
            if (TopTabIndex(page) < 0) {
                g_curPage = ParentPage(page);
                page = g_curPage;
            } else if (page != PAGE_MAIN) {
                g_curPage = PAGE_MAIN;
                page = PAGE_MAIN;
            }
        }
        {
            int ti = TopTabIndex(page);
            for (int p = page, guard = 0; ti < 0 && guard < 8; ++guard) {
                p = ParentPage(p);
                ti = TopTabIndex(p);
            }
            if (ti < 0) ti = 0;
            if (Hit(b, prev, BTN_RB)) {
                g_curPage = kTopTabs[(ti + 1) % 5];
                page = g_curPage;
            } else if (Hit(b, prev, BTN_LB)) {
                g_curPage = kTopTabs[(ti + 4) % 5];
                page = g_curPage;
            }
        }

        if (n && sel < n && !(b & BTN_LT)) {
            Option& o = g_options[sel];
            if (Hit(b, prev, BTN_RIGHT))
                Adjust(o, +1);
            else if (Hit(b, prev, BTN_LEFT))
                Adjust(o, -1);
        }
        g_prev = b;
    }
}  // namespace reconrender
