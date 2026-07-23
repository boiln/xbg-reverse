#include "internal.hpp"

#include <math.h>
#include <ppcintrinsics.h>
#include <xtl.h>

namespace esp {
    void DrawWorldItems(void* cg, char* base) {

        int mode     = CB(V_ENTITIES);
        int scavMode = CB(V_SCAVENGER);
        if (!mode && !scavMode) return;
        int scavengerIndex    = pModelIndex("scavenger_item_mp");
        int scavengerMaterial = pFind(0x06, "hud_scavenger_pickup", 0);
        ARGB enemy            = Pack(CFp(C_ENEMY), 0xFFFF3030);
        ARGB friendly         = Pack(CFp(C_FRIENDLY), 0xFF30FF30);
        ARGB iconColor        = Pack(CFp(C_LABEL_TEXT), 0xFFFFFFFF);

        for (int i = 18; i < 1023; ++i) {
            char* e = base + i * ENT_STRIDE;
            if ((RB(e, E_ALIVE) & 0x40) == 0) continue;
            if (((u32)RI(e, 0x1D4) & 0x40000u) != 0) continue;
            if (RI(e, 0x1D0) == 0) continue;
            int entityClass = RI(e, 0x2B4);
            bool scavenger  = entityClass == scavengerIndex;

            if (scavenger && scavMode) {
                Vec3 scavOrigin = RV3(e, E_ORIGIN);
                Vec2 scavScreen;

                if (W2S(cg, scavOrigin, &scavScreen)) {
                    if ((scavMode == 1 || scavMode == 3) && scavengerMaterial) {
                        float white[4];

                        Vec4Of(iconColor, white);
                        pStretch(scavScreen.x - 20.0f, scavScreen.y, 40.0f, 20.0f, 0.0f, 0.0f, 1.0f, 1.0f, white,
                            scavengerMaterial);
                    }

                    if (scavMode == 2 || scavMode == 3) Box3D(base, cg, i, e, enemy);
                }
            }

            if (!mode || scavenger) continue;
            if ((u32)entityClass > 0xA4u) continue;
            if (RI(e, E_WEAPIDX) == scavengerIndex) continue;
            Vec3 o = RV3(e, E_ORIGIN);
            if (!(o.x > -1e6f && o.x < 1e6f && o.y > -1e6f && o.y < 1e6f && o.z > -1e6f && o.z < 1e6f)) continue;
            Vec2 sc;
            if (!W2S(cg, o, &sc)) continue;
            bool teammate = pTeam(0, e) != 0;
            ARGB col      = teammate ? friendly : enemy;

            if (mode == 1 || mode == 3) {
                int table = *(int*)(0x845CA998u + ((u32)entityClass & 0xFFu)* 4u);
                int def   = table ? *(int*)(table + 8) : 0;
                int icon  = def ? *(int*)(def + 0x660) : 0;

                if (icon) {
                    float w = 28.0f;
                    float h = 28.0f;

                    if (*(short*)(e + E_TYPE)== 3) {
                        w = 60.0f;
                        h = 30.0f;
                    } else {
                        void* state = pWDef(1);
                        int style   = state ? RI(state, 0x3B0) : 0;

                        if (style == 1) {
                            w = 56.0f;
                            h = 28.0f;
                        } else if (style == 2) {
                            w = 56.0f;
                            h = 14.0f;
                        }
                    }

                    float rgba[4];

                    Vec4Of(iconColor, rgba);
                    pStretch(sc.x, sc.y, w, h, 0.0f, 0.0f, 1.0f, 1.0f, rgba, icon);
                }
            }

            if ((mode == 2 || mode == 3) && (*(short*)(e + E_TYPE)!= 3 || !teammate)) Box3D(base, cg, i, e, col);
        }

    }

    int LocalPing() {

        void* cg = CG();
        if (!cg) return -1;

        __try {
            u32 a = *(u32*)((char*)cg + 0x84);
            u32 b = *(u32*)((char*)cg + 0x24084);
            int p = (int)((a + b)>> 1);

            return (p < 0 || p > 999) ? 0 : p;
        } __except(EXCEPTION_EXECUTE_HANDLER) {
            return -1;
        }

    }

    const char* PlayerName(int idx) {

        void* cg = CG();
        if (!cg || idx < 0 || idx >= 18) return 0;
        const char* n = (const char*)cg + CG_CLIENTINFO + idx * CI_STRIDE + CI_NAME;

        return n[0] ? n : 0;

    }
    bool InGame() { return CG() != 0; }

    void SyncSessionMode() { *reconrender::CfgByte(0x90B4328E) = pIsZombieSession() ? 1 : 0; }

    int LocalClientIdx() {

        void* cg = CG();
        if (!cg) return -1;
        int i = *(int*)((char*)cg + CG_CLIENTNUM);

        return (i >= 0 && i < 18) ? i : -1;

    }

    bool PlayerEligible(void* cg, char* base, int idx) {

        if (!cg || !base || idx < 0) return false;

        void** cgsPointer = (void**)(u64)A_CGS_Pointer;
        void* cgs         = cgsPointer ? *cgsPointer : 0;
        if (!cgs) return false;
        if ((u32)idx > (u32)RI(cgs, 0x150)) return false;
        char* ent = base + idx * ENT_STRIDE;
        if (idx == RI(cg, 0x48248) && ((u32)RI(cg, 0x481A4) & 0x4000u) != 0) return true;
        short type = *(short*)(ent + E_TYPE);
        u32 alive  = (u32)RI(ent, E_ALIVE);
        if (type != 1) return false;
        if (*(char*)(ent + E_ALIVE)== 0 || (alive & 0x40000000u) == 0) return false;
        if (((u32)RI(ent, 0x1D4) & 0x40000u) != 0) return false;
        if (*(char*)((char*)cg + 0x69B1F + idx * CI_STRIDE)!= 0) return false;

        return idx == (int)*(signed char*)(ent + 0x2C3);

    }

    bool ZombieEntityEligible(char* ent, int idx) {

        if (!ent || idx < 0 || idx > 0x400) return false;
        if (*(short*)(ent + E_TYPE)!= 0x10) return false;

        u32 flags = (u32)RI(ent, 0x1D4);
        if ((flags & 0x20u) != 0 || (flags & 0x40000u) != 0) return false;
        u32 state = (u32)RI(ent, E_ALIVE);
        if ((state & 0xF00000u) == 0x500000u) return (state & 0x40000000u) != 0;
        u32 secondary = (u32)RI(ent, 0x1D8);
        if ((flags & 0x800u) == 0 && secondary != 0 && (secondary & 2u) != 0) return (state & 0x40000000u) != 0;
        if (flags == 0 || RI(ent, 0xE0) == 0) return false;

        return (state & 0x40000000u) != 0;

    }

    bool ZombieEntityIsEnemy(char* ent) {
        if (!ent || *(short*)(ent + E_TYPE)!= 0x10) return false;
        if (pTeam(0, ent) != 0) return false;

        return RI(ent, 0x22C) == 2;
    }
}
