#include "luda/core/arena.hpp"

#include <xtl.h>

extern "C" {
extern const unsigned int g_xbgDumpSize;
extern const unsigned int g_xbgDumpChunkSize;
extern const unsigned int g_xbgDumpChunkCount;
extern const char* const g_xbgDumpChunks[];
}

namespace xbg {
    unsigned char* g_arena = 0;
    volatile unsigned int g_bootStage = 0;
    unsigned char* g_tramp = 0;
    volatile unsigned int g_hookMask = 0xFFFFFFFF;

    void ArenaInit() {

        if (g_arena) return;

        g_arena = (unsigned char*)VirtualAlloc(0, 0x110000, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
        if (!g_arena) return;
        memset(g_arena, 0, 0x110000);

        g_tramp = (unsigned char*)VirtualAlloc(0, 0x10000, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
        unsigned int off = 0;
        for (unsigned int i = 0; i < g_xbgDumpChunkCount; ++i) {
            unsigned int len = g_xbgDumpChunkSize;
            if (off + len > g_xbgDumpSize) len = g_xbgDumpSize - off;
            memcpy(g_arena + off, g_xbgDumpChunks[i], len);
            off += len;
        }

    }

    void ArenaFree() {

        if (g_tramp) {
            VirtualFree(g_tramp, 0, MEM_RELEASE);
            g_tramp = 0;
        }

        if (g_arena) {
            VirtualFree(g_arena, 0, MEM_RELEASE);
            g_arena = 0;
        }

    }
}
