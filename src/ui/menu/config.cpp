#include "internal.hpp"

namespace reconrender {
    void ConfigDefaults() {

        for (u32 i = 0; i < CFG_SIZE; ++i) g_cfg[i] = 0;
        *Fp(0x90B433A4) = 80.0f;
        *Bp(0x90B438CC) = 2;

        *Bp(0x90B433A1) = 1;
        *Bp(0x90B433AA) = 1;
        *Bp(0x90B433AB) = 1;
        *Bp(0x90B433B4) = 1;
        *Bp(0x90B433B5) = 1;
        *Bp(0x90B433C4) = 2;
        *Bp(0x90B433B0) = 1;
        *Bp(0x90B433C0) = 1;
        *Bp(0x90B433C8) = 1;
        *Bp(0x90B433A8) = 1;
        *Bp(0x90B43394) = 1;
        *Bp(0x90B438BA) = 1;
        *Bp(0x90B438BD) = 1;
        *Bp(0x90B438BB) = 1;
        *Bp(0x90B438BC) = 1;
        *Bp(0x90B438BE) = 1;
        *Bp(0x90B438B9) = 1;
        *Bp(0x90B438C0) = 1;
        *Bp(0x90B438A0) = 3;
        *Fp(0x90B438AC) = 5.0f;
        *Bp(0x90B438E4) = 1;

        *Fp(0x90B438EC) = 120.0f;
        *Fp(0x90B438F0) = 65.0f;
        *Fp(0x90B438E8) = 0.0f;
        *Fp(0x90B43930) = 0.7f;
        SetRGBA(VA_COL_MENU, 0.741f, 0.537f, 1.0f, 1.0f);
        SetRGBA(VA_COL_TEXT, 1.0f, 1.0f, 1.0f, 1.0f);
        SetRGBA(0x90B43954, 0.741f, 0.537f, 1.0f, 1.0f);
        SetRGBA(0x90B439C4, 0.741f, 0.537f, 1.0f, 1.0f);
        SetRGBA(0x90B439F4, 0.741f, 0.537f, 1.0f, 1.0f);
        SetRGBA(0x90B43A24, 1.0f, 1.0f, 1.0f, 1.0f);
        SetRGBA(0x90B439B4, 0.741f, 0.537f, 1.0f, 1.0f);

    }

    struct FloatDefault {
        u32 va;
        float value;
    };
    static void ApplyPostInitColours() {

        static const FloatDefault k[] = {
            {0x90B43930u, 0.7f},   {0x90B43934u, 0.741f}, {0x90B43924u, 0.0f},   {0x90B43928u, 0.0f},
            {0x90B4392Cu, 0.0f},   {0x90B43938u, 0.537f}, {0x90B4393Cu, 1.0f},   {0x90B43940u, 1.0f},
            {0x90B43954u, 0.741f}, {0x90B43950u, 1.0f},   {0x90B43958u, 0.537f}, {0x90B43960u, 1.0f},
            {0x90B43904u, 1.0f},   {0x90B43944u, 1.0f},   {0x90B43948u, 1.0f},   {0x90B4394Cu, 1.0f},
            {0x90B4395Cu, 1.0f},   {0x90B43908u, 1.0f},   {0x90B4390Cu, 1.0f},   {0x90B43910u, 1.0f},
            {0x90B43918u, 0.537f}, {0x90B4391Cu, 1.0f},   {0x90B43920u, 1.0f},   {0x90B43914u, 0.741f},
            {0x90B439F4u, 0.2f},   {0x90B439F8u, 1.0f},   {0x90B439FCu, 0.2f},   {0x90B43A00u, 1.0f},
            {0x90B439B4u, 0.741f}, {0x90B439B8u, 0.537f}, {0x90B439BCu, 1.0f},   {0x90B439C0u, 1.0f},
            {0x90B439C4u, 0.741f}, {0x90B439C8u, 0.537f}, {0x90B439CCu, 1.0f},   {0x90B439D0u, 1.0f},
            {0x90B43A14u, 0.365f}, {0x90B43A18u, 0.664f}, {0x90B43A20u, 1.0f},   {0x90B43A1Cu, 0.85f},
            {0x90B43A24u, 1.0f},   {0x90B43A34u, 1.0f},   {0x90B43A28u, 1.0f},   {0x90B43A38u, 1.0f},
            {0x90B43A2Cu, 1.0f},   {0x90B43A30u, 1.0f},   {0x90B43A44u, 1.0f},   {0x90B439A8u, 1.0f},
            {0x90B439ACu, 1.0f},   {0x90B439B0u, 0.6f},   {0x90B43A48u, 1.0f},   {0x90B43A4Cu, 1.0f},
            {0x90B43A50u, 1.0f},   {0x90B43998u, 0.3f},   {0x90B4399Cu, 0.3f},   {0x90B43974u, 0.0f},
            {0x90B439A0u, 0.6f},   {0x90B439A4u, 1.0f},   {0x90B43A40u, 1.0f},   {0x90B439D4u, 0.741f},
            {0x90B439D8u, 0.537f}, {0x90B439DCu, 1.0f},   {0x90B439E0u, 1.0f},   {0x90B4396Cu, 1.0f},
            {0x90B43970u, 1.0f},   {0x90B43A3Cu, 1.0f},   {0x90B43964u, 1.0f},   {0x90B43968u, 1.0f},
            {0x90B43978u, 0.0f},   {0x90B43A08u, 0.664f}, {0x90B43A0Cu, 0.85f},  {0x90B43A10u, 1.0f},
            {0x90B43980u, 1.0f},   {0x90B43A04u, 0.365f}, {0x90B43984u, 0.5f},   {0x90B439E4u, 0.8f},
            {0x90B439E8u, 0.0f},   {0x90B439ECu, 1.0f},   {0x90B4397Cu, 0.0f},   {0x90B43988u, 0.5f},
            {0x90B4398Cu, 0.5f},   {0x90B43990u, 0.7f},   {0x90B43994u, 0.3f},   {0x90B439F0u, 1.0f},
            {0x90B43A54u, 0.365f}, {0x90B43A58u, 0.664f}, {0x90B43A5Cu, 0.85f},  {0x90B43A60u, 1.0f}};
        for (u32 i = 0; i < sizeof(k) / sizeof(k[0]); ++i) *Fp(k[i].va) = k[i].value;

    }

    void ApplyPresetConfig(int mode) {

        if ((u32)mode >= 6u) return;
        *Bp(0x90B438E0u) = (u8)mode;
        *Bp(0x90B4389Cu) = 0;

        if (mode == 0 || mode == 1) {
            *Bp(0x90B438BDu) = 1;
            *Bp(0x90B438BFu) = 1;
            *Bp(0x90B438B0u) = mode == 1 ? 2 : 1;
            *Bp(0x90B438CCu) = 0;
            *Bp(0x90B438BAu) = 1;
            *Bp(0x90B438B9u) = 1;
            *Bp(0x90B438BBu) = 1;
            *Bp(0x90B438BCu) = 1;
            *Bp(0x90B438C1u) = 1;
            *Bp(0x90B438C3u) = 1;
            *Bp(0x90B438BEu) = 1;
            *Bp(0x90B438B8u) = 1;
            *Bp(0x90B438C0u) = 1;
            *Bp(0x90B43895u) = 1;
            *Bp(0x90B438A0u) = 4;
            *Bp(0x90B433AAu) = 1;
            *Bp(0x90B433C4u) = 1;
            *Bp(0x90B433AEu) = 1;
            *Bp(0x90B438A4u) = 6;
            *Bp(0x90B433ABu) = 1;
            *Bp(0x90B433B4u) = 1;
            *Bp(0x90B433B6u) = 1;
            *Bp(0x90B433B0u) = 2;
            *Bp(0x90B433B8u) = 1;
            *Bp(0x90B433C1u) = 1;
            *Bp(0x90B433ACu) = 1;
            *Bp(0x90B433B5u) = 1;
            *Bp(0x90B433B7u) = 1;
            *Bp(0x90B433BCu) = 1;
            *Bp(0x90B433C0u) = 1;
            *Bp(0x90B433C8u) = 1;
            *Bp(0x90B433ADu) = 1;
            *Bp(0x90B43898u) = 3;
            if (mode == 0) NotifyMsg("Make sure to prioritize players when HvHing!", 3000);
        } else if (mode == 2) {
            *Bp(0x90B438B0u) = 2;
            *Bp(0x90B438BBu) = 1;
            *Bp(0x90B438BDu) = 1;
            *Bp(0x90B438BFu) = 1;
            *Bp(0x90B438C0u) = 1;
            *Bp(0x90B438C1u) = 1;
            *Bp(0x90B438A0u) = 3;
            *Bp(0x90B438C3u) = 1;
            *Bp(0x90B438BEu) = 1;
            *Bp(0x90B438CCu) = 0;
            *Bp(0x90B438A4u) = 5;
            *Bp(0x90B433AAu) = 1;
            *Bp(0x90B433ACu) = 1;
            *Bp(0x90B433B6u) = 0;
            *Fp(0x90B438A8u) = 16.0f;
            *Fp(0x90B438ACu) = 40.0f;
            *Bp(0x90B438BAu) = 1;
            *Bp(0x90B438BCu) = 1;
            *Bp(0x90B438B8u) = 1;
            *Bp(0x90B438B9u) = 1;
            *Bp(0x90B43895u) = 1;
            *Bp(0x90B433ABu) = 1;
            *Bp(0x90B433C4u) = 1;
            *Bp(0x90B433B4u) = 1;
            *Bp(0x90B433B5u) = 1;
            *Bp(0x90B433AEu) = 1;
            *Bp(0x90B433B7u) = 0;
            *Bp(0x90B433B0u) = 2;
            *Bp(0x90B433B8u) = 1;
            *Bp(0x90B433BCu) = 0;
            *Bp(0x90B433C0u) = 1;
            *Bp(0x90B433C1u) = 1;
            *Bp(0x90B433C8u) = 1;
            *Bp(0x90B433ADu) = 1;
            *Bp(0x90B43898u) = 3;
        } else if (mode == 3) {
            *Bp(0x90B438BAu) = 1;
            *Bp(0x90B438C1u) = 1;
            *Fp(0x90B43A70u) = 2.4f;
            *Bp(0x90B438C3u) = 1;
            *Bp(0x90B438B8u) = 1;
            *Bp(0x90B438C0u) = 1;
            *Bp(0x90B438A0u) = 4;
            *Bp(0x90B438A4u) = 6;
            *Fp(0x90B43A6Cu) = 0.0f;
            *Fp(0x90B43930u) = 0.86f;
            *Bp(0x90B438B0u) = 1;
            SetRGBA(0x90B43934u, 0.564f, 0.0f, 1.0f, 1.0f);
            *Bp(0x90B438BDu) = 1;
            SetRGBA(0x90B43954u, 0.564f, 0.0f, 1.0f, 1.0f);
            *Bp(0x90B438BFu) = 1;
            *Bp(0x90B438B9u) = 1;
            *Fp(0x90B43A14u) = 1.0f;
            *Bp(0x90B433AAu) = 1;
            *Fp(0x90B43A18u) = 0.0f;
            *Bp(0x90B433ABu) = 1;
            *Fp(0x90B43A1Cu) = 1.0f;
            *Bp(0x90B438CCu) = 0x21;
            *Fp(0x90B43A24u) = 1.0f;
            *Bp(0x90B438BEu) = 1;
            *Fp(0x90B439C4u) = 0.564f;
            *Fp(0x90B439C8u) = 0.0f;
            *Fp(0x90B439CCu) = 1.0f;
            *Fp(0x90B43A34u) = 0.564f;
            *Fp(0x90B43A38u) = 0.0f;
            *Fp(0x90B43A3Cu) = 1.0f;
            *Fp(0x90B43944u) = 1.0f;
            *Fp(0x90B43948u) = 1.0f;
            *Fp(0x90B4394Cu) = 1.0f;
            *Fp(0x90B439D4u) = 0.564f;
            *Fp(0x90B439D8u) = 0.0f;
            *Fp(0x90B439DCu) = 1.0f;
            *Bp(0x90B433ACu) = 1;
            *Fp(0x90B433A4u) = 90.0f;
            *Bp(0x90B433C4u) = 3;
            *Bp(0x90B433B4u) = 1;
            *Bp(0x90B433B5u) = 1;
            *Bp(0x90B433AEu) = 1;
            *Bp(0x90B433B6u) = 0;
            *Bp(0x90B433B7u) = 0;
            *Bp(0x90B433B0u) = 2;
            *Bp(0x90B433B8u) = 1;
            *Bp(0x90B433BCu) = 0;
            *Bp(0x90B433C0u) = 1;
            *Bp(0x90B433C1u) = 1;
            *Bp(0x90B433C8u) = 1;
            *Bp(0x90B433ADu) = 1;
            *Bp(0x90B43BD4u) = 1;
            *Bp(0x90B43A88u) = 0;
            *Bp(0x90B43898u) = 3;
            *Bp(0x90B438D0u) = 1;
            *Bp(0x90B438F8u) = 1;
            NotifyMsg("Make sure to prioritize players when HvHing!", 3000);
        } else if (mode == 4) {
            *Bp(0x90B4339Cu) = 1;
            *Bp(0x90B4339Fu) = 1;
            *Bp(0x90B4339Du) = 1;
            *Fp(0x90B439D8u) = 0.0f;
            *Fp(0x90B43A6Cu) = 0.5f;
            *Fp(0x90B43A70u) = 0.2f;
            *Fp(0x90B43938u) = 0.588f;
            *Fp(0x90B4393Cu) = 0.0f;
            *Fp(0x90B43958u) = 0.588f;
            *Bp(0x90B438B0u) = 2;
            *Fp(0x90B4395Cu) = 0.0f;
            *Bp(0x90B438BDu) = 0;
            *Bp(0x90B438BCu) = 0;
            *Bp(0x90B438BFu) = 0;
            *Bp(0x90B438C1u) = 0;
            *Bp(0x90B438CCu) = 6;
            *Bp(0x90B438B8u) = 0;
            *Fp(0x90B439D4u) = 0.564f;
            *Bp(0x90B438B4u) = 2;
            *Fp(0x90B439DCu) = 1.0f;
            *Bp(0x90B438B9u) = 0;
            *Fp(0x90B43944u) = 1.0f;
            *Fp(0x90B43948u) = 1.0f;
            *Bp(0x90B433A1u) = 0;
            *Fp(0x90B4394Cu) = 1.0f;
            *Bp(0x90B433A2u) = 0;
            *Fp(0x90B43934u) = 1.0f;
            *Bp(0x90B438BAu) = 0;
            *Fp(0x90B43930u) = 0.247f;
            *Bp(0x90B438BBu) = 1;
            *Fp(0x90B43954u) = 1.0f;
            *Bp(0x90B438C3u) = 0;
            *Fp(0x90B439C4u) = 0.741f;
            *Bp(0x90B438BEu) = 0;
            *Fp(0x90B439C8u) = 0.537f;
            *Bp(0x90B438C0u) = 0;
            *Bp(0x90B43895u) = 0;
            *Bp(0x90B43898u) = 0;
            *Bp(0x90B433AAu) = 1;
            *Bp(0x90B433ABu) = 1;
            *Bp(0x90B43395u) = 1;
            *Fp(0x90B439CCu) = 1.0f;
            *Fp(0x90B43A14u) = 0.364f;
            *Fp(0x90B43A24u) = 1.0f;
            *Fp(0x90B43A28u) = 1.0f;
            *Fp(0x90B43A2Cu) = 1.0f;
            *Fp(0x90B43A54u) = 0.364f;
            *Fp(0x90B43A44u) = 1.0f;
            *Fp(0x90B43A48u) = 1.0f;
            *Fp(0x90B43A4Cu) = 1.0f;
            *Bp(0x90B438A0u) = 0;
            *Fp(0x90B43A18u) = 0.662f;
            *Bp(0x90B438A4u) = 0;
            *Fp(0x90B43A1Cu) = 0.85f;
            *Bp(0x90B433ACu) = 1;
            *Fp(0x90B43A58u) = 0.662f;
            *Bp(0x90B433C4u) = 3;
            *Fp(0x90B43A5Cu) = 0.85f;
            *Bp(0x90B433B4u) = 1;
            *Fp(0x90B433A4u) = 95.0f;
            *Bp(0x90B433B5u) = 0;
            *Fp(0x90B43A80u) = -125.0f;
            *Bp(0x90B433AEu) = 0;
            *Fp(0x90B43A84u) = -70.0f;
            *Bp(0x90B433B6u) = 0;
            *Bp(0x90B433B7u) = 1;
            *Bp(0x90B433B8u) = 0;
            *Bp(0x90B433BCu) = 0;
            *Bp(0x90B433B0u) = 0;
            *Bp(0x90B433C0u) = 1;
            *Bp(0x90B433C1u) = 1;
            *Bp(0x90B433C8u) = 1;
            *Bp(0x90B433ADu) = 0;
            *Bp(0x90B438D1u) = 1;
            *Bp(0x90B438D0u) = 0;
            *Bp(0x90B433A8u) = 1;
            *Bp(0x90B433A9u) = 1;
            *Bp(0x90B43394u) = 1;
            *Bp(0x90B438F8u) = 0;
            *Bp(0x90B438F7u) = 0;
            *Bp(0x90B438F5u) = 0;
            *Bp(0x90B438F6u) = 0;
            *Bp(0x90B438F4u) = 0;
        } else {
            for (u32 i = 0; i < 0x860u; ++i) *Bp(0x90B43380u + i) = 0;
            *(u32*)Bp(0x90B43380u) = 0xFB000240u;
            *Bp(0x90B4339Fu) = 1;
            *Fp(0x90B433A4u) = 65.0f;
            *Fp(0x90B438ECu) = 120.0f;
            *Fp(0x90B438E8u) = 0.0f;
            *Fp(0x90B438F0u) = 65.0f;
            *Bp(0x90B433A8u) = 1;
            *Bp(0x90B433A9u) = 1;
            *Bp(0x90B43394u) = 1;
            *Bp(0x90B438B0u) = 0;
            *Bp(0x90B438C3u) = 1;
            *Bp(0x90B438BBu) = 1;
            *Bp(0x90B438B8u) = 1;
            *Bp(0x90B433ABu) = 0;
            *Bp(0x90B433ACu) = 0;
            *Bp(0x90B433AEu) = 0;
            *Bp(0x90B433B4u) = 0;
            *Bp(0x90B433B0u) = 2;
            *Bp(0x90B433B5u) = 0;
            *Bp(0x90B433C4u) = 2;
            ApplyPostInitColours();
        }
        NotifyMsg("Configuration Loaded!", 3000);

    }

    void RunCmd(const char* cmd) { ((void(__cdecl*)(int, const char*))(u64)0x824015E0u)(0, cmd); }

    bool RealEngineResident() {

        const void* base = (const void*)(u64)0x90B00000u;
        MEMORY_BASIC_INFORMATION mbi;
        if (!VirtualQuery(base, &mbi, sizeof(mbi))) return false;
        if (mbi.State != MEM_COMMIT) return false;
        if (mbi.Protect & (PAGE_NOACCESS | PAGE_GUARD)) return false;
        __try {
            return *(volatile const u32*)base == 0x4D5A9000u;
        } __except (EXCEPTION_EXECUTE_HANDLER) {
            return false;
        }

    }

    static inline void* ResolveImportAddr(u32 hash) {

        u32 key = hash;
        u32* node = ((u32 * (__cdecl*)(u32*))(u64)0x90B0A590u)(&key);
        if (!node) return 0;

        return (void*)(u64)(node[0] ^ node[1]);

    }

    const char kFreezeGt[] = {(char)0x5E, (char)0x48, (char)0x7F, (char)0x7F, (char)0x01, 0};
    void GtFlush(void* q) {
        __dcbst(0, q);
        __sync();
        __emit(0x4C00012C);
    }
    void SetMyGamertag(const char* s) {

        char* b1 = (char*)(u64)0x82C55D60u;
        char* b2 = (char*)(u64)0x841E1B30u;
        int i = 0;
        for (; s[i] && i < 0x1F; ++i) {
            b1[i] = s[i];
            b2[i] = s[i];
        }
        b1[i] = 0;
        b2[i] = 0;
        static const unsigned char pa[0x20] = {
            0x7C, 0x83, 0x23, 0x78, 0x3D, 0x60, 0x82, 0xC5, 0x38, 0x8B, 0x5D,
            0x60, 0x3D, 0x60, 0x82, 0x4A, 0x39, 0x6B, 0xDC, 0xA0, 0x38, 0xA0,
            0x00, 0x20, 0x7D, 0x69, 0x03, 0xA6, 0x4E, 0x80, 0x04, 0x20};
        static const unsigned char pb[0x10] = {
            0x3D, 0x60, 0x82, 0xC5, 0x39, 0x6B, 0x5D, 0x00,
            0x7D, 0x69, 0x03, 0xA6, 0x4E, 0x80, 0x04, 0x20};
        unsigned char* A = (unsigned char*)(u64)0x82C55D00u;
        unsigned char* B = (unsigned char*)(u64)0x8293D724u;
        for (int j = 0; j < 0x20; ++j) A[j] = pa[j];
        for (int j = 0; j < 0x10; ++j) B[j] = pb[j];
        *(volatile unsigned char*)(u64)0x8259B6A7u = 0x00;
        *(volatile unsigned char*)(u64)0x822D1110u = 0x40;
        for (u32 a = 0x82C55D00u; a < 0x82C55D20u; a += 8) GtFlush((void*)(u64)a);
        for (u32 a = 0x8293D724u; a < 0x8293D734u; a += 8) GtFlush((void*)(u64)a);
        GtFlush((void*)(u64)0x8259B6A0u);
        GtFlush((void*)(u64)0x822D1110u);

    }

    void ResolveThirdPersonHeadTag() {

        if (s_thirdPersonHeadTag) return;
        typedef short(__cdecl * SLGetStringFn)(const char*, int);

        SLGetStringFn getString = (SLGetStringFn)(u64)A_SL_GetString;

        short tag = getString("j_head", 0);

        if (tag > 0) s_thirdPersonHeadTag = (u16)tag;

    }
}
