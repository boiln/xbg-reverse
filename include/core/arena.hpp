#pragma once

namespace xbg {
    typedef unsigned int u32;
    typedef unsigned long long u64;

    extern unsigned char* g_arena;
    extern unsigned char* g_tramp;
    extern volatile unsigned int g_bootStage;
    extern volatile unsigned int g_hookMask;

    void ArenaInit();

    void ArenaFree();

    unsigned long long* SeedNode(unsigned int hash);

    inline unsigned char* At(u32 va) {
        if (va >= 0x90B00000u && va < 0x90C10000u) return g_arena + (va - 0x90B00000u);

        return reinterpret_cast<unsigned char*>(static_cast<u64>(va));
    }
}
