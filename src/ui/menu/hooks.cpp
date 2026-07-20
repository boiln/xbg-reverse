#include "internal.hpp"

namespace reconrender {
#define DET_MAX 12
#define DET_WORDS 28

    static u32 s_stubs[DET_MAX][DET_WORDS];
    static int s_stubCount = 0;
    static bool s_rwx = false;
    static void Sync(void* p) {
        __dcbst(0, p);
        __sync();
        __emit(0x4C00012C);
    }
    static void PatchJump(u32* at, u32 dest, bool linked) {
        at[0] = 0x3C000000u | (dest >> 16);
        at[1] = 0x60000000u | (dest & 0xFFFF);
        at[2] = 0x7C0903A6u;
        at[3] = 0x4E800420u | (linked ? 1u : 0u);
        Sync(at);
        Sync(at + 2);
    }
    static u32 BranchTarget(u32 ins, u32* at) {
        u32 o = ins & 0x03FFFFFC;
        if (o & 0x02000000) o |= 0xFC000000u;

        return (u32)(u64)at + (s32)o;
    }
    struct Detour {
        u32* m_src;
        u32* m_stub;
        u32 m_orig[4];
        bool m_installed;
        int m_stubIdx;

        Detour() : m_src(0), m_stub(0), m_installed(false), m_stubIdx(-1) {}

        template <class Fn>
        Fn Original() const {
            return reinterpret_cast<Fn>(m_stub);
        }

        bool Install(u32 s, const void* dst) {
            if (m_installed || !s || !dst) return false;

            if (m_stubIdx < 0) {
                if (s_stubCount >= DET_MAX) return false;
                m_stubIdx = s_stubCount++;
            }

            if (!s_rwx) {
                DWORD o;
                VirtualProtect(s_stubs, sizeof(s_stubs), PAGE_EXECUTE_READWRITE, &o);
                s_rwx = true;
            }
            m_src = (u32*)s;
            m_stub = s_stubs[m_stubIdx];

            // installation saves the four instructions replaced by the detour.
            for (int i = 0; i < 4; ++i) m_orig[i] = m_src[i];

            int n = 0;
            for (int i = 0; i < 4; ++i) {
                u32 ins = m_src[i];
                if ((ins >> 26) == 18) {
                    // copied relative branches must be retargeted from the stub address.
                    bool lk = (ins & 1) != 0;
                    PatchJump(&m_stub[n], BranchTarget(ins, &m_src[i]), lk);
                    n += 4;
                    if (!lk) {
                        for (int k = 0; k < n; ++k) Sync(&m_stub[k]);
                        PatchJump(m_src, (u32)(u64)dst, false);
                        m_installed = true;
                        return true;
                    }
                } else
                    m_stub[n++] = ins;
            }
            PatchJump(&m_stub[n], (u32)(u64)&m_src[4], false);
            for (int i = 0; i < n + 4; ++i) Sync(&m_stub[i]);
            PatchJump(m_src, (u32)(u64)dst, false);
            m_installed = true;

            return true;
        }

        void Remove() {
            if (!m_installed) return;
            __try {
                // removal restores the four instructions saved during installation.
                for (int i = 0; i < 4; ++i) {
                    m_src[i] = m_orig[i];
                    Sync(&m_src[i]);
                }
            } __except (EXCEPTION_EXECUTE_HANDLER) {
            }
            m_installed = false;
        }
    };
    static Detour s_paint;
    static Detour s_xinput;
    static Detour s_clcmd;
    static Detour s_cgplayer;
    static Detour s_entityevent;
    static Detour s_bulletemit;
    static Detour s_tagworld;
    static Detour s_chatcommand;
    static Detour s_netchan;
    static Detour s_servercommand;
    static Detour s_uicolor;
    static Detour s_setcamo;
    static volatile bool s_running = false;
    static HANDLE s_bootThread = 0;
    static volatile bool s_bootExited = false;

    static void ForceUiRgb(u32 destination) {
        if (!destination) return;
        float* rgb = Fp(0x90B439B4u);
        for (int i = 0; i < 3; ++i) {
            int value = rgb[i] < 1.0f ? (int)(rgb[i] * 255.0f) : 255;
            if (value < 0) value = 0;
            *(volatile u8*)(u64)(destination + 1u + (u32)i) = (u8)value;
        }
    }

    typedef void(__cdecl* UiColourFn)(u32, u16, u32, u32, u32, u32);
    static void __cdecl hkUiColour(u32 assetRecord, u16 param2, u32 param3, u32 param4, u32 colour, u32 param6) {
        const char* name = 0;
        __try {
            if (assetRecord) name = *(const char**)(u64)assetRecord;
            if (name && (SameStr(name, "ui_holotable_grid") || SameStr(name, "ui_holotable_grid2"))) return;

            if (name && *Bp(0x90B438FAu)) {
                bool apply = (SameStr(name, "ui_globe") && *Bp(0x90B438FCu)) ||
                             (ContainsStr(name, "lui_") && *Bp(0x90B43900u)) ||
                             (ContainsStr(name, "hud") && *Bp(0x90B43901u)) ||
                             (ContainsStr(name, "minimap") && *Bp(0x90B43902u)) || ContainsStr(name, "controller") ||
                             (SameStr(name, "menu_mp_soldiers") && *Bp(0x90B438FDu)) || ContainsStr(name, "score_") ||
                             (SameStr(name, "ui_holotable_grid3") && *Bp(0x90B438FEu));
                if (apply) ForceUiRgb(colour);
            }
        } __except (EXCEPTION_EXECUTE_HANDLER) {
        }
        UiColourFn original = s_uicolor.Original<UiColourFn>();
        if (original) original(assetRecord, param2, param3, param4, colour, param6);
    }

    typedef void(__cdecl* SetCamoFn)(u32, u32, float, u32, u32, u32, u32);
    static void __cdecl hkSetCamo(u32 param1, u32 param2, float param3, u32 param4, u32 camo, u32 param6, u32 param7) {
        if (*Bp(0x90B438D3u) && esp::InGame()) camo = (u32)*Bp(0x90B438DCu);
        SetCamoFn original = s_setcamo.Original<SetCamoFn>();
        if (original) original(param1, param2, param3, param4, camo, param6, param7);
    }

    static void AsciiLower(char* text) {
        if (!text) return;
        for (; *text; ++text)
            if (*text >= 'A' && *text <= 'Z') *text = (char)(*text + ('a' - 'A'));
    }

    static void ProcessChatCommand(u32 client, char* message) {
        if (!message || (!*Bp(0x90B4339Du) && !*Bp(0x90B4339Cu))) return;

        u32 base = *(volatile u32*)(u64)0x83B50F40u;

        if (!base || client < base) {
            // clearing rejects an unmappable sender before the original handler runs.
            message[0] = 0;
            return;
        }

        u32 sender = (client - base) / 0x4E100u;

        if (sender > 17) {
            message[0] = 0;
            return;
        }
        if ((int)sender == esp::LocalClientIdx()) return;

        u32 len = 0;
        while (message[len] && len <= 0x688u) ++len;

        if (len > 0x688u) {
            // clearing prevents the original handler from consuming an oversized command.
            message[0] = 0;
            return;
        }

        char lower[0x689];
        for (u32 i = 0; i <= len; ++i) lower[i] = message[i];

        AsciiLower(lower);

        const char* name = esp::PlayerName((int)sender);
        if (!name || !name[0]) name = "Unknown";

        if (*Bp(0x90B4339Du) && StrHas(lower, "userinfo")) {
            NotifyPlayer("Kick All From: ", name, 3000);
            message[0] = 0;
            return;
        }
        if (!*Bp(0x90B4339Cu)) return;

        if (StrHas(lower, "endround") || StrHas(lower, "killserverpc")) {
            NotifyPlayer("End Game From: ", name, 3000);
            message[0] = 0;
            return;
        }

        if (StrHas(lower, "sl")) {
            if (!*Bp(0x90B4328Eu)) NotifyPlayer("Server Crash From: ", name, 3000);
            if (StrHas(message, "_SUNSETCRASH")) NotifyPlayerFull("Detected ", name, " as an RME user.", 3000);
            message[0] = 0;
            return;
        }

        if (StrHas(lower, "callvote")) {
            NotifyPlayer("Callvote Exploit From: ", name, 3000);
            message[0] = 0;
            return;
        }
        if (!StrHas(lower, "userinfo") || !StrHas(lower, "xuid")) return;

        char xuid[17];
        U64ToHex(PlayerXuid((int)sender), xuid);

        if (PlayerXuid((int)sender) && StrHas(lower, xuid)) return;
        NotifyPlayer("Kick All From: ", name, 3000);
        message[0] = 0;
    }

    typedef void(__cdecl* ChatCommandFn)(u32, char*, void*);
    static void __cdecl hkChatCommand(u32 client, char* message, void* extra) {
        __try {
            ProcessChatCommand(client, message);
        } __except (EXCEPTION_EXECUTE_HANDLER) {
        }
        ChatCommandFn orig = s_chatcommand.Original<ChatCommandFn>();
        if (orig) orig(client, message, extra);
    }

    static const char* ResolveNetSenderName(u32 context) {
        if (!context) return "";
        typedef u32(__cdecl * PartyGetActiveFn)();
        typedef u32(__cdecl * PartyMemberAddressKeyFn)(u32, int);
        PartyGetActiveFn getActive = (PartyGetActiveFn)(u64)A_PartyGetActive;
        PartyMemberAddressKeyFn memberKey = (PartyMemberAddressKeyFn)(u64)A_PartyMemberAddressKey;

        __try {
            if (*(volatile u32*)(u64)(context + 0x18u) == 2u) {
                if (!esp::InGame()) return (const char*)(u64)0x841E1B30u;
                u32 cg = *(volatile u32*)(u64)A_CG_POINTER;
                if (cg) {
                    int local = *(volatile int*)(u64)cg;
                    const char* name =
                        (const char*)(u64)(cg + (u32)local * CG_CLIENTINFO_STRIDE + CG_CLIENTINFO + 0xCu);
                    return name;
                }
            }

            u32 party = getActive();
            if (!party) return "";
            u32 key = *(volatile u32*)(u64)(context + 0x10u);
            for (int i = 0; i < 18; ++i) {
                if (memberKey(party, i) != key) continue;
                const char* name = ClientTableName(i);
                return name ? name : "";
            }
        } __except (EXCEPTION_EXECUTE_HANDLER) {
        }

        return "";
    }

    typedef void(__cdecl* NetValidateFn)(u32, u32);
    static void __cdecl hkNetValidate(u32 context, u32 message) {
        bool drop = false;
        bool rme = false;
        __try {
            if (*Bp(0x90B4339Fu) && message) {
                volatile u8* record = *(volatile u8**)(u64)(message + 8u);
                if (record && ((*(volatile u32*)record & 0xC0000000u) != 0)) {
                    s8 fieldA = *(volatile s8*)(record + 4);
                    s16 fieldB = *(volatile s16*)(record + 6);
                    u16 fieldC = *(volatile u16*)(record + 8);
                    if (fieldA > 0x7F || fieldB > 0x7FFF || fieldB < 0) drop = true;
                    if (!drop && fieldC > 0x7FFFu) {
                        drop = true;
                        rme = true;
                    }
                }
            }
        } __except (EXCEPTION_EXECUTE_HANDLER) {
            drop = false;
        }

        if (drop) {
            NotifyPlayer(rme ? "Prevented RME Exploit From: " : "Prevented Server Exploit From: ",
                         ResolveNetSenderName(context), 3000);
            return;
        }
        NetValidateFn orig = s_netchan.Original<NetValidateFn>();
        if (orig) orig(context, message);
    }

    static bool ServerCommandAllowed(u32 localClientNum) {
        typedef const char*(__cdecl * CmdArgvFn)(int);
        typedef int(__cdecl * AtoiFn)(const char*);
        CmdArgvFn cmdArgv = (CmdArgvFn)(u64)A_CmdArgv;
        AtoiFn atoiFn = (AtoiFn)(u64)A_Atoi;

        const char* command = cmdArgv(0);
        if (!command || (u8)command[0] != 0x69u) return true;

        const char* arg1Text = cmdArgv(1);
        u32 arg1 = (u32)atoiFn(arg1Text);
        const char* arg2Text = cmdArgv(2);
        atoiFn(arg2Text);

        u32 cg = *(volatile u32*)(u64)0x82BBAE68u;
        u32 candidate = (localClientNum * 0x21DC0u + arg1 + 0x1A223u) * 4u + cg;

        if (candidate >= 0xC0000000u && candidate < 0xC0010000u) {
            NotifyMsg("Host Freeze Prevented", 3000);
            return false;
        }
        if (!*Bp(0x90B4339Fu)) return true;

        return arg1 < 0xBu;
    }

    typedef void(__cdecl* ServerCommandFn)(u32);
    static void __cdecl hkServerCommand(u32 localClientNum) {
        bool allowed = true;
        __try {
            allowed = ServerCommandAllowed(localClientNum);
        } __except (EXCEPTION_EXECUTE_HANDLER) {
            allowed = true;
        }
        if (!allowed) return;
        ServerCommandFn orig = s_servercommand.Original<ServerCommandFn>();
        if (orig) orig(localClientNum);
    }

    typedef void (*PaintFn)(int, int);
    static void __cdecl hkPaint(int r3, int r4) {
        PaintFn orig = s_paint.Original<PaintFn>();
        if (orig) orig(r3, r4);
        __try {
            Frame();
        } __except (EXCEPTION_EXECUTE_HANDLER) {
        }
    }
    typedef unsigned long(__cdecl* XInputFn)(unsigned long, XINPUT_STATE*);
    static unsigned long __cdecl hkXInput(unsigned long idx, XINPUT_STATE* st) {
        XInputFn orig = s_xinput.Original<XInputFn>();
        unsigned long r = orig ? orig(idx, st) : (unsigned long)ERROR_DEVICE_NOT_CONNECTED;

        if (r == ERROR_SUCCESS && st && idx == 0) {
            __try {
                Input(st->Gamepad.wButtons, st->Gamepad.bLeftTrigger);

                if (!g_open) aimbot::ApplyInputSynth(&st->Gamepad.wButtons, &st->Gamepad.bRightTrigger);
            } __except (EXCEPTION_EXECUTE_HANDLER) {
            }
            if (g_open) {
                st->Gamepad.wButtons &= (unsigned short)~0x330F;
            }
        }

        return r;
    }

    typedef void(__cdecl* SendCmdFn)(int, int, void*);
    static void __cdecl hkSendCmd(int clientNum, int serverTime, void* cmd) {
        if (clientNum == 0) {
            __try {
                aimbot::InjectCmd();
            } __except (EXCEPTION_EXECUTE_HANDLER) {
            }
        }
        SendCmdFn orig = s_clcmd.Original<SendCmdFn>();
        if (orig) orig(clientNum, serverTime, cmd);
    }

    typedef void(__cdecl* CGPlayerFn)(int, void*);
    static void __cdecl hkCGPlayer(int localClientNum, void* centity) {
        CGPlayerFn orig = s_cgplayer.Original<CGPlayerFn>();
        if (orig) orig(localClientNum, centity);

        __try {
            aimbot::ApplyFakeModel(localClientNum, centity);
        } __except (EXCEPTION_EXECUTE_HANDLER) {
        }
    }

    typedef void(__cdecl* EntityEventFn)(int, void*, unsigned, void*);
    static void __cdecl hkEntityEvent(int localClientNum, void* centity, unsigned event, void* eventData) {
        __try {
            aimbot::OnEntityEvent(centity, event);
        } __except (EXCEPTION_EXECUTE_HANDLER) {
        }
        EntityEventFn orig = s_entityevent.Original<EntityEventFn>();
        if (orig) orig(localClientNum, centity, event, eventData);
    }

    typedef void(__cdecl* BulletEmitFn)(void*, unsigned, void*, void*, void*, const void*, void*, void*, u32, u32, u32,
                                        u32, u32);
    static void __cdecl hkBulletEmit(void* a1, unsigned owner, void* a3, void* a4, void* a5, const void* endpoint,
                                     void* a7, void* a8, u32 a9, u32 a10, u32 a11, u32 a12, u32 a13) {
        __try {
            esp::OnBulletEmit(owner, endpoint);
        } __except (EXCEPTION_EXECUTE_HANDLER) {
        }
        BulletEmitFn orig = s_bulletemit.Original<BulletEmitFn>();
        if (orig) orig(a1, owner, a3, a4, a5, endpoint, a7, a8, a9, a10, a11, a12, a13);
    }

    typedef int(__cdecl* TagWorldFn)(void*, void*, u32, void*, Vec3*);
    typedef void(__cdecl* GetPlayerEyeFn)(void*, Vec3*, u32);
    static int __cdecl hkTagWorld(void* centity, void* dobj, u32 tag, void* axis, Vec3* out) {
        TagWorldFn orig = s_tagworld.Original<TagWorldFn>();
        int result = orig ? orig(centity, dobj, tag, axis, out) : 0;
        u16 headTag = s_thirdPersonHeadTag;
        if (!out || !headTag || tag != (u32)headTag || !*Bp(0x90B433A1)) return result;

        void* cg = *(void**)(u64)0x82BBAE68u;
        if (!cg) return result;
        u32 base = (u32)(u64)cg;
        if (*(volatile u32*)(u64)base != *(volatile u32*)(u64)(base + 0x48248u)) return result;

        __try {
            void* playerState = (void*)(u64)(base + 0x480A8u);
            GetPlayerEyeFn getEye = (GetPlayerEyeFn)(u64)A_BG_GetPlayerEyePosition;
            getEye(playerState, out, *(volatile u32*)playerState);
            out->z = *(volatile float*)(u64)(base + 0x480D8u) + *Fp(0x90B438F0);
        } __except (EXCEPTION_EXECUTE_HANDLER) {
        }

        return result;
    }

    static bool TagWorldTargetIsClean() {
        volatile u32* p = (volatile u32*)(u64)A_CG_DObjGetWorldTagPos;

        return p[0] == 0x7D8802A6u && p[1] == 0x48745BF9u && p[2] == 0x9421FF90u && p[3] == 0x7CDD3378u;
    }

    static bool BulletEmitTargetIsClean() {
        volatile u32* p = (volatile u32*)(u64)A_BulletEmit;

        return p[0] == 0x7D8802A6u && p[1] == 0x486BE18Du && p[2] == 0x9421FF10u && p[3] == 0x3D6082AAu;
    }

    static bool ChatTargetIsClean() {
        volatile u32* p = (volatile u32*)(u64)A_ChatCommandReceive;

        return p[0] == 0x7D8802A6u && p[1] == 0x484E8B75u && p[2] == 0x9421FF80u && p[3] == 0x7C7D1B78u;
    }

    static bool NetTargetIsClean() {
        volatile u32* p = (volatile u32*)(u64)A_NetChanValidate;

        return p[0] == 0x3C009080u && p[1] == 0x6000B0A0u && p[2] == 0x7C0903A6u && p[3] == 0x4E800420u;
    }

    static bool ServerCommandTargetIsClean() {
        volatile u32* p = (volatile u32*)(u64)A_ServerCommand;

        return p[0] == 0x3C00809Du && p[1] == 0x60000900u && p[2] == 0x7C0903A6u && p[3] == 0x4E800420u;
    }

    static bool UiColourTargetIsClean() {
        volatile u32* p = (volatile u32*)(u64)0x828BA040u;
        bool stock = p[0] == 0x7D8802A6u && p[1] == 0x4805BF25u && p[2] == 0x9421FF40u && p[3] == 0x7C7F1B78u;
        bool resident = (p[0] & 0xFFFF0000u) == 0x3C000000u && (p[1] & 0xFFFF0000u) == 0x60000000u &&
                        p[2] == 0x7C0903A6u && p[3] == 0x4E800420u && (p[0] & 0xFFFFu) >= 0x9000u &&
                        (p[0] & 0xFFFFu) < 0x9200u;

        return stock || resident;
    }

    static bool SetCamoTargetIsClean() {
        volatile u32* p = (volatile u32*)(u64)0x82245B48u;
        bool stock = p[0] == 0x7D8802A6u && p[1] == 0x486D0405u && p[2] == 0xDBA1FF50u && p[3] == 0xDBC1FF58u;
        bool resident = (p[0] & 0xFFFF0000u) == 0x3C000000u && (p[1] & 0xFFFF0000u) == 0x60000000u &&
                        p[2] == 0x7C0903A6u && p[3] == 0x4E800420u && (p[0] & 0xFFFFu) >= 0x9000u &&
                        (p[0] & 0xFFFFu) < 0x9200u;

        return stock || resident;
    }

    static bool TitleUp() { return XamGetCurrentTitleId() == BO2_TITLE_ID; }

    static unsigned long __stdcall BootThunk(void*) {
        s_bootExited = false;

        while (s_running) {
            while (s_running && !TitleUp()) Sleep(250);
            if (!s_running) break;
            for (int i = 0; i < 125 && s_running && TitleUp(); ++i) Sleep(16);
            if (!s_running || !TitleUp()) continue;
            s_paint.Install(A_Menu_PaintAll, (const void*)hkPaint);
            s_xinput.Install(A_XInputGetState, (const void*)hkXInput);
            s_clcmd.Install(A_CL_SendCmd, (const void*)hkSendCmd);
            s_cgplayer.Install(A_CG_Player, (const void*)hkCGPlayer);
            s_entityevent.Install(A_CG_EntityEvent, (const void*)hkEntityEvent);
            if (BulletEmitTargetIsClean()) s_bulletemit.Install(A_BulletEmit, (const void*)hkBulletEmit);
            if (TagWorldTargetIsClean()) s_tagworld.Install(A_CG_DObjGetWorldTagPos, (const void*)hkTagWorld);
            if (ChatTargetIsClean()) s_chatcommand.Install(A_ChatCommandReceive, (const void*)hkChatCommand);
            if (NetTargetIsClean()) s_netchan.Install(A_NetChanValidate, (const void*)hkNetValidate);
            if (ServerCommandTargetIsClean()) s_servercommand.Install(A_ServerCommand, (const void*)hkServerCommand);
            if (UiColourTargetIsClean()) s_uicolor.Install(0x828BA040u, (const void*)hkUiColour);
            if (SetCamoTargetIsClean()) s_setcamo.Install(0x82245B48u, (const void*)hkSetCamo);
            while (s_running && TitleUp()) Sleep(16);
            s_setcamo.Remove();
            s_uicolor.Remove();
            s_servercommand.Remove();
            s_netchan.Remove();
            s_chatcommand.Remove();
            s_tagworld.Remove();
            s_entityevent.Remove();
            s_bulletemit.Remove();
            s_cgplayer.Remove();
            s_clcmd.Remove();
            s_xinput.Remove();
            s_paint.Remove();
        }
        s_bootExited = true;

        return 0;
    }
    void Start() {
        if (s_running) return;
        s_running = true;
        s_bootExited = false;
        unsigned long t = 0;
        ExCreateThread(&s_bootThread, 0x20000, &t, (void*)XapiThreadStartup, BootThunk, 0, 0x2);
    }

    void Stop() {
        if (!s_running && !s_bootThread && !s_keyboardThread) return;
        s_running = false;
        s_keyboardStop = true;

        if (s_keyboardThread) {
            __try {
                ((void(__cdecl*)(u32))(u64)0x816F2E60u)(0);
            } __except (EXCEPTION_EXECUTE_HANDLER) {
            }
            WaitForSingleObject(s_keyboardThread, 4000);
            CloseHandle(s_keyboardThread);
            s_keyboardThread = 0;
        }
        __try {
            esp::Shutdown();
        } __except (EXCEPTION_EXECUTE_HANDLER) {
        }
        __try {
            s_setcamo.Remove();
            s_uicolor.Remove();
            s_servercommand.Remove();
            s_netchan.Remove();
            s_chatcommand.Remove();
            s_bulletemit.Remove();
            s_tagworld.Remove();
            s_entityevent.Remove();
            s_cgplayer.Remove();
            s_clcmd.Remove();
            s_xinput.Remove();
            s_paint.Remove();
        } __except (EXCEPTION_EXECUTE_HANDLER) {
        }
        Sleep(140);

        if (s_bootThread) {
            WaitForSingleObject(s_bootThread, 4000);
            CloseHandle(s_bootThread);
            s_bootThread = 0;
        }
        for (int i = 0; i < 400 && !s_bootExited; ++i) Sleep(5);
        Sleep(40);
    }
}
