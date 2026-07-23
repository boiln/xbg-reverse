#include "internal.hpp"

namespace reconrender {
    void ApplyMainConfig() {
        if (RealEngineResident()) {
            *(volatile u8*)(u64)0x90B4339Cu = *Bp(0x90B4339Cu);
            *(volatile u8*)(u64)0x90B4339Du = *Bp(0x90B4339Du);
            *(volatile u8*)(u64)0x90B4339Fu = *Bp(0x90B4339Fu);
        }

        void* cg = *(void**)(u64)0x82BBAE68u;
        if (!cg) return;
        ResolveThirdPersonHeadTag();
        u32 base = (u32)(u64)cg;
        if (*(volatile u32*)(u64)base != *(volatile u32*)(u64)(base + 0x48248u)) return;
        *(volatile u8*)(u64)(base + 0x4809Cu) = (u8)(*Bp(0x90B433A1) ? 1 : 0);

        if (u32 rp = *(volatile u32*)(u64)0x82BBC584u) {
            float r = *Fp(0x90B438EC);

            *(volatile u32*)(u64)(rp + 0x18u) = *(u32*)&r;
        }

        if (u32 ap = *(volatile u32*)(u64)0x82BC27A8u) {
            float a = *Fp(0x90B438E8);

            *(volatile u32*)(u64)(ap + 0x18u) = *(u32*)&a;
        }

        *(volatile float*)(u64)0x83C59ED8u = *Fp(0x90B433A4);

        volatile u32* laserInstruction = (volatile u32*)(u64)0x82255E1Cu;

        if ((*laserInstruction & 0xFFFFFF00u) == 0x2B0B0000u) {
            *(volatile u8*)(u64)0x82255E1Fu = *Bp(0x90B433A2u);
            __dcbst(0, (void*)(u64)0x82255E1Cu);
            __sync();
            __emit(0x4C00012C);
        }
    }

    void ApplySettingsConfig() {
        u32 vsync = *(volatile u32*)(u64)0x84916368u;

        if (vsync) *(volatile u8*)(u64)(vsync + 0x18u) = *Bp(0x90B438D1u) ? 0 : 1;

        u32 forceA = *(volatile u32*)(u64)0x82CB745Cu;
        u32 forceB = *(volatile u32*)(u64)0x82CB8244u;

        if (forceA && forceB) {
            *(volatile u8*)(u64)(forceA + 0x18u) = *Bp(0x90B42A40u) ? 0 : 1;
            *(volatile u8*)(u64)(forceB + 0x18u) = *Bp(0x90B42A40u) ? 1 : 0;
        }

        u32 fixedOff = *(volatile u32*)(u64)0x83B6F49Cu;
        u32 fixedOn = *(volatile u32*)(u64)0x841DDFCCu;

        if (fixedOff && fixedOn) {
            *(volatile u8*)(u64)(fixedOff + 0x18u) = 0;
            *(volatile u8*)(u64)(fixedOn + 0x18u) = 1;
        }

        *(volatile u32*)(u64)0x84449014u = *Bp(0x90B42A40u) ? 0x70000000u : (*Bp(0x90B42A35u) ? 0u : 0x539u);

        if (*Bp(0x90B42A35u) && *Bp(0x90B42A40u)) {
            NotifyMsg("Disabling force host!", 3000);
            *Bp(0x90B42A40u) = 0;
        }

        static u32 dlcOriginal[8];
        static bool dlcCached = false;
        static int dlcLast = -1;
        int disableDlc = *Bp(0x90B42A4Cu) ? 1 : 0;

        if (!dlcCached) {
            for (int i = 0; i < 8; ++i) dlcOriginal[i] = *(volatile u32*)(u64)(0x83880EF8u + (u32)i * 0x4Cu);
            dlcCached = true;
        }

        if (disableDlc != dlcLast) {
            for (int i = 0; i < 8; ++i) {
                volatile u32* word = (volatile u32*)(u64)(0x83880EF8u + (u32)i * 0x4Cu);

                if ((*word & 0x0FFFFFFFu) != 4u) *word = disableDlc ? 0x10000000u : dlcOriginal[i];
            }

            dlcLast = disableDlc;
        }

        if (*Bp(0x90B438F8u)) {
            *(volatile float*)(u64)0x83C28854u = *Fp(0x90B439D4u);
            *(volatile float*)(u64)0x83C28858u = *Fp(0x90B439D8u);
            *(volatile float*)(u64)0x83C2885Cu = *Fp(0x90B439DCu);
            *(volatile float*)(u64)0x83C56038u = *Fp(0x90B439D4u);
            *(volatile float*)(u64)0x83C5603Cu = *Fp(0x90B439D8u);
            *(volatile float*)(u64)0x83C56040u = *Fp(0x90B439DCu);
        } else {
            *(volatile u32*)(u64)0x83C28854u = 0x80000000u;
            *(volatile u32*)(u64)0x83C28858u = 0x80000000u;
            *(volatile u32*)(u64)0x83C2885Cu = 0x80000000u;
            *(volatile u32*)(u64)0x83C56038u = 0x3F800000u;
            *(volatile u32*)(u64)0x83C5603Cu = 0x3F800000u;
            *(volatile u32*)(u64)0x83C56040u = 0x3F800000u;
        }

        if (*Bp(0x90B438D0u) && esp::InGame()) {
            *(volatile u32*)(u64)0x82FFC2FCu = 0x64C0u;
            *(volatile u32*)(u64)0x82FF9B3Cu = 0x22C2u;
            *(volatile u32*)(u64)0x8308C12Cu = 0x40C0u;
        }

        static u32 unlockTick = 0;
        u32 now = GetTickCount();

        if (now - unlockTick >= 1600u) {
            struct Patch32 {
                u32 address;
                u32 value;
            };

            static const Patch32 fixed[] = {
                {0x82004B5Cu, 0x00000000u}, {0x822781D8u, 0x60000000u}, {0x825BAB6Cu, 0x60000000u},
                {0x8270F148u, 0x60000000u}, {0x823C93E8u, 0x4E800020u}, {0x82903A10u, 0x4E800020u},
                {0x822D2D68u, 0x4E800020u}, {0x825D1BA8u, 0x4E800020u}, {0x82717D48u, 0x91370000u},
                {0x826A5FBCu, 0x3B40FFEFu}, {0x8219F3CCu, 0x60000000u}, {0x821A1C04u, 0x60000000u},
                {0x821DE58Cu, 0x60000000u}, {0x821DFC80u, 0x60000000u}, {0x821DFD64u, 0x60000000u}};

            for (u32 i = 0; i < sizeof(fixed) / sizeof(fixed[0]); ++i) {
                *(volatile u32*)(u64)fixed[i].address = fixed[i].value;
                GtFlush((void*)(u64)fixed[i].address);
            }

            volatile u64* patchA = (volatile u64*)(u64)0x82413340u;
            volatile u64* patchB = (volatile u64*)(u64)0x826A3A90u;

            if (*Bp(0x90B438D2u)) {
                *patchA = 0x386000014E800020ull;
                *patchB = 0x386000004E800020ull;
            } else {
                *patchA = 0x7D8802A69181FFF8ull;
                *patchB = 0x7D8802A6482724F9ull;
            }

            GtFlush((void*)(u64)0x82413340u);
            GtFlush((void*)(u64)0x82413344u);
            GtFlush((void*)(u64)0x826A3A90u);
            GtFlush((void*)(u64)0x826A3A94u);
            unlockTick = now;
        }
    }

    void ApplyGodMode() {
        if (*(volatile u32*)(u64)0x83B50F40u == 0) return;
        if (!PlayerMenuIsHost()) return;

        u32 cg = PlayerMenuCg();
        u32 cgs = 0;

        __try {
            cgs = *(volatile u32*)(u64)A_CGS_POINTER;
        } __except (EXCEPTION_EXECUTE_HANDLER) {
            return;
        }

        if (!cg || !cgs) return;

        typedef u64(__cdecl * ClientConnectedFn)(u32);

        ClientConnectedFn residentConnected = RealEngineResident() ? (ClientConnectedFn)(u64)0x90B19068u : 0;
        int local = *(volatile int*)(u64)cg;
        bool localConnected = false;

        if (residentConnected)
            localConnected = residentConnected((u32)local) != 0;
        else
            localConnected = local >= 0 && local < 18 && esp::PlayerName(local) != 0;

        if (!localConnected) return;

        int count = *(volatile int*)(u64)(cgs + 0x150u);
        if (count <= 0) return;
        if (count > 18) count = 18;

        for (int i = 0; i < count; ++i) {
            bool connected = residentConnected ? residentConnected((u32)i) != 0 : esp::PlayerName(i) != 0;
            if (!connected) continue;

            volatile u8* pb = (volatile u8*)(0x83551A2Bull + (u64)i * 0x57F8ull);
            u8 god = *Bp(0x90B433F1 + (u32)i);
            u8 v = *pb;

            if (!god) {
                if (v & 5) *pb = (u8)(v & 0xFE);
            } else if (v & 4)
                *pb = (u8)(v | 1);
        }
    }

    static bool WriteRenamedPlayer(int target, const char* name) {
        if (target < 0 || target >= 18 || !name) return false;
        if (*(volatile u32*)(u64)0x83B50F40u == 0) return false;
        if (!PlayerMenuIsHost()) return false;

        volatile char* serverName = (volatile char*)(u64)(0x83556F44u + (u32)target * 0x57F8u);
        volatile char* clientName = (volatile char*)(u64)(A_CLIENT_INFO + (u32)target * CLIENT_INFO_STRIDE + 0x40u);

        for (int i = 0; i < 0x20; ++i) {
            serverName[i] = name[i];
            clientName[i] = name[i];
            s_savedRename[target][i] = name[i];
        }

        s_savedRenameSet[target] = 1;

        return true;
    }

    static unsigned long __stdcall RenameKeyboardWorker(void*) {
        typedef u32(__cdecl * ShowKeyboardFn)(u32, u32, const WCHAR*, const WCHAR*, const WCHAR*, WCHAR*, u32,
                                              volatile u32*);
        typedef void(__cdecl * CancelKeyboardFn)(u32);

        static const WCHAR kEmpty[] = L"";
        static const WCHAR kTitle[] = L"luda v1.0.0";
        static const WCHAR kDescription[] = L"Enter a custom Gamertag. (32 character Max)\nCredit: Gamer7112";
        WCHAR result[0x20] = {0};
        volatile u32 status[6] = {0};
        u32 rc = ((ShowKeyboardFn)(u64)0x816C1F38u)(0, 0, kEmpty, kTitle, kDescription, result, 0x20, status);

        if (rc != 0x3E5u) {
            NotifyMsg("Keyboard UI failed to open, try again.", 3000);

            return 0;
        }

        while (status[0] == 0x3E5u) {
            if (s_keyboardStop) {
                ((CancelKeyboardFn)(u64)0x816F2E60u)(0);

                return 0;
            }

            Sleep(100);
        }

        if (status[4] != 0 || s_keyboardStop) return 0;

        char narrow[0x20] = {0};

        for (int i = 0; i < 0x1F && result[i] != 0; ++i) narrow[i] = (char)(result[i] & 0xFF);
        __try {
            WriteRenamedPlayer((int)s_keyboardTarget, narrow);
        } __except (EXCEPTION_EXECUTE_HANDLER) {
        }

        return 0;
    }

    bool BeginRenamePlayer(int target) {
        if (s_keyboardThread) {
            if (WaitForSingleObject(s_keyboardThread, 0) != WAIT_OBJECT_0) return false;

            CloseHandle(s_keyboardThread);
            s_keyboardThread = 0;
        }

        s_keyboardTarget = target;
        s_keyboardStop = false;

        unsigned long tid = 0;
        int rc = ExCreateThread(&s_keyboardThread, 0, &tid, 0, RenameKeyboardWorker, 0, 0x04000426u);
        if (rc >= 0) return true;
        s_keyboardThread = 0;
        NotifyMsg("Keyboard UI failed to open, try again.", 3000);

        return false;
    }

    static const u8 kCrashPayload0[] = {0x3B, 0x20, 0x22, 0x5E, 0x48, 0xA4, 0xA8, 0x54, 0x17, 0xC6, 0x22, 0x00};
    static const u8 kCrashPayload1[] = {0x37, 0x20, 0x33, 0x30, 0x20, 0x39, 0x30, 0x00};
    static const u8 kCrashPayload2[] = {
        0x3B, 0x20, 0x22, 0x5E, 0x48, 0xFB, 0x43, 0xFB, 0x31, 0x65, 0xAC, 0x2B, 0x76, 0x2B, 0x44, 0x2B, 0x43,
        0x65, 0xAC, 0x54, 0x67, 0x64, 0x77, 0x64, 0xE5, 0x2B, 0xA6, 0x38, 0xD6, 0x65, 0x2D, 0x65, 0x2D, 0xA6,
        0x65, 0x2B, 0x66, 0x2B, 0x54, 0x2B, 0x43, 0x65, 0x2D, 0x4F, 0xEA, 0x2B, 0x54, 0x2B, 0x53, 0x65, 0x2B,
        0x4F, 0x78, 0x4F, 0x76, 0x64, 0xE5, 0xA6, 0x66, 0x66, 0xD6, 0x65, 0x2D, 0x65, 0xA6, 0xA6, 0x65, 0xA6,
        0x65, 0x64, 0xE7, 0x5F, 0xE0, 0x65, 0x65, 0x65, 0x2B, 0x64, 0xE7, 0x38, 0xE5, 0x65, 0x65, 0x65, 0xA6,
        0x65, 0xBF, 0x65, 0xBF, 0x65, 0x65, 0x65, 0xA6, 0x65, 0x66, 0x65, 0x66, 0x65, 0xA6, 0x65, 0x65, 0x65,
        0x2D, 0x65, 0x2B, 0x65, 0x65, 0x65, 0x65, 0x65, 0xA6, 0x65, 0xA6, 0x65, 0x65, 0x65, 0x65, 0x65, 0x65,
        0x65, 0x66, 0x65, 0xA6, 0x65, 0x65, 0x65, 0x65, 0x65, 0x65, 0x65, 0xA6, 0x65, 0xA6, 0x22, 0x00};
    static const u8 kCrashPayload3[] = {0x5C, 0x20, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x00};
    static const u8 kCrashPayload4[] = {0x68, 0x20, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x00};

    void SendCrashPlayerExact(int target) {
        typedef void(__cdecl * ServerCommandFn)(u32, u32, const char*);

        ServerCommandFn send = (ServerCommandFn)(u64)0x8242FB70u;

        send((u32)target, 0, (const char*)kCrashPayload0);
        send((u32)target, 0, (const char*)kCrashPayload1);
        send((u32)target, 0, (const char*)kCrashPayload2);

        u32 cg = *(volatile u32*)(u64)A_CG_POINTER;
        u32 encoded = ((0u - cg - 0x7D66CF38u) >> 2) - 0x1A223u;
        char dynamicPayload[64];
        char number[16];
        int out = 0;

        dynamicPayload[out++] = 'i';
        dynamicPayload[out++] = ' ';
        ItoA((s32)encoded, number);
        for (int i = 0; number[i]; ++i) dynamicPayload[out++] = number[i];
        dynamicPayload[out++] = ' ';
        ItoA((s32)0x82336E30u, number);
        for (int i = 0; number[i]; ++i) dynamicPayload[out++] = number[i];
        dynamicPayload[out] = 0;
        send((u32)target, 1, dynamicPayload);

        send((u32)target, 0, (const char*)kCrashPayload3);
        send((u32)target, 0, (const char*)kCrashPayload4);
        NotifyMsg("Crashed Player.", 3000);
    }

    const char* DetectMenuName(const char* name) {
        if (StrHas(name, "XBOX360LSBEST")) return "Detected Jiggy Menu V4.2 (Infection)";

        if (StrHas(name, "^6J^5i^6g^5g^6y")) {
            if (StrHas(name, "^5v^64^5.^62")) return "Detected Jiggy Menu V4.2";
            if (StrHas(name, "^5v^64^5.^65")) return "Detected Jiggy Menu V4.5";
            if (StrHas(name, "^5v^64^5.^63")) return "Detected Jiggy Menu V4.3";
            if (StrHas(name, "^5v^64^5.^64")) return "Detected Jiggy Menu V4.4";
            if (StrHas(name, "^5v^64^5")) return "Detected Jiggy Menu V4";
            if (StrHas(name, "^5v^65")) return "Detected Jiggy Menu V5";
            if (StrHas(name, "^5v^63^5.^69")) return "Detected Jiggy Menu V3.9";
            if (StrHas(name, "^5v^63^5.^68")) return "Detected Jiggy Menu V3.8";
            if (StrHas(name, "^5v^62^5.^62")) return "Detected Jiggy Menu V2.2";
            if (StrHas(name, "^5v^61^5.^66")) return "Detected Jiggy Menu V1.6";

            return "Detected Jiggy Menu";
        }

        if (StrHas(name, "^1B^5at^1M^5an")) {
            if (StrHas(name, "^1V^517")) return "Detected Batman V17";
            if (StrHas(name, "^1V^516")) return "Detected Batman V16";
            if (StrHas(name, "^1V^515")) return "Detected Batman V15";
            if (StrHas(name, "^1V^514")) return "Detected Batman V14";
            if (StrHas(name, "^1V^513")) return "Detected Batman V13";
            if (StrHas(name, "^1V^512")) return "Detected Batman V12";
            if (StrHas(name, "^1V^57")) return "Detected Batman V7";
            if (StrHas(name, "^1V^56^1.^54")) return "Detected Batman V6.4";
            if (StrHas(name, "^1V6")) return "Detected Batman V6";
            if (StrHas(name, "^1V5")) return "Detected Batman V5";
            if (StrHas(name, "^6V2")) return "Detected Batman V2";

            return "Detected Batman";
        }

        if (StrHas(name, "TCM")) {
            if (StrHas(name, "V15")) return "Detected TCM V15";
            if (StrHas(name, "V13")) return "Detected TCM V13";
            if (StrHas(name, "V14")) return "Detected TCM V14";

            return "Detected TCM";
        }

        if (StrHas(name, "^5C^3h^5e^3a^5t^3s")) return "Detected Simple Aimbot Menu (GSC Infection V1)";
        if (StrHas(name, "XBL^5Pony")) return "Detected XBL Pony";
        if (StrHas(name, "^5J^1i^5g^1g^5y")) return "Detected Jiggy Menu";
        if (StrHas(name, "ELeGanCe")) return "Detected Elegance";
        if (StrHas(name, "Bossam")) return "Detected Bossam";
        if (StrHas(name, "Predator")) return "Detected Predator";
        if (StrHas(name, "by: Matrix")) return "Detected Matrix";
        if (StrHas(name, "Serenity")) return "Detected Serenity";
        if (StrHas(name, "xePixTvx")) return "8xePixTvx's 'Unreleased Menu'";
        if (StrHas(name, "Project Iconic")) return "Detected Project Iconic";
        if (StrHas(name, "Conversion")) return "Detected Conversion V1";
        if (StrHas(name, "Zombieland")) return "Detected Zombieland";
        if (StrHas(name, "Velocity")) return "Detected Velocity";
        if (StrHas(name, "Kamil_Modz")) return "Detected Kamil Modz V10.2";
        if (StrHas(name, "RM|T Zombies Menu V2.6")) return "Detected RM|T Zombies Menu V2.6";

        return 0;
    }
}
