#pragma once

namespace xbg {
    typedef unsigned int u32;
    typedef unsigned long long u64;

    extern unsigned char* g_arena;
    extern unsigned char* g_tramp;
    extern volatile unsigned int g_bootStage;
    extern volatile unsigned int g_hookMask;

    // allocates and hydrates the reconstructed arena and trampoline storage.
    void ArenaInit();
    // releases the arena and trampoline allocations.
    void ArenaFree();
    // resolves a hashed import seed from the reconstructed arena.
    unsigned long long* SeedNode(unsigned int hash);

    // maps a virtual address into the reconstructed arena when applicable.
    inline unsigned char* At(u32 va) {
        if (va >= 0x90B00000u && va < 0x90C10000u) return g_arena + (va - 0x90B00000u);

        return reinterpret_cast<unsigned char*>(static_cast<u64>(va));
    }
}
