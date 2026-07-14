#include "internal.hpp"

#include <math.h>
#include <ppcintrinsics.h>
#include <xtl.h>

namespace esp {
    void Init() {
        s_font = 0;
        s_white = 0;
        s_tracerCount = 0;
    }

    void Shutdown() {
        ConstantRadar(false);
        DisableNativeCrosshair(false);
    }

    void Render() {
        void* cg = CG();

        if (!cg) {
            s_tracerCount = 0;
            return;
        }

        char* rd0 = (char*)cg + CG_REFDEF;

        if (RI(rd0, RD_W) < 1 || RI(rd0, RD_H) < 1) {
            s_tracerCount = 0;
            return;
        }

        bool zombieSession = CB(0x90B4328E) != 0;

        DrawTracerList(cg);
        if (!s_font) s_font = pFind(0x15, "fonts/720/normalFont", 0xFFFFFFFF);
        if (!s_white) s_white = pFind(0x06, "white", 0xFFFFFFFF);
        if (!s_font || !s_white) return;

        s_accent = Pack(CFp(C_ACCENT), 0xFFBD89FF);
        ConstantRadar(!zombieSession && CB(V_RADAR) != 0);
        DisableNativeCrosshair(CB(V_DISABLECROSS) != 0);
        Crosshair(cg);
        if (CB(V_HEALTHBAR)) LocalHealthBar(cg);

        char* base = Entities();
        int localIdx = RI(cg, CG_CLIENTNUM);
        Vec3 localOrg = {0.0f, 0.0f, 0.0f};

        if (base && localIdx >= 0 && localIdx < 18) localOrg = RV3(base + localIdx * ENT_STRIDE, E_ORIGIN);
        if (base && !zombieSession) DrawWorldItems(cg, base);
        bool drawE = CB(V_ENEMY) != 0, drawF = CB(V_FRIENDLY) != 0;
        if ((!drawE && !drawF) || !base) return;

        ARGB colEnemy = Pack(CFp(C_ENEMY), 0xFFFF3030), colFriend = Pack(CFp(C_FRIENDLY), 0xFF30FF30);

        if (zombieSession && drawF) {
            void** cgsPointer = (void**)(u64)A_CGS_Pointer;
            void* cgs = cgsPointer ? *cgsPointer : 0;
            int maxClients = cgs ? RI(cgs, 0x150) : 0;
            for (int i = 0; i < maxClients && i < 0x401; ++i) {
                if (i == localIdx || !PlayerEligible(cg, base, i)) continue;
                DrawPlayer(base, cg, i, Entity(base, i), colFriend, localOrg);
            }
        }
        int scanCount = zombieSession ? 0x401 : 18;
        for (int i = 0; i < scanCount; ++i) {
            if (i == localIdx) continue;
            char* ent = Entity(base, i);
            if (!ent) continue;
            if (zombieSession) {
                if (!ZombieEntityEligible(ent, i)) continue;
                bool enemy = ZombieEntityIsEnemy(ent);
                if (enemy && drawE) {
                    DrawPlayer(base, cg, i, ent, colEnemy, localOrg);
                    if (CB(V_POINTERS) && CB(0x90B4328D) == 0) DrawPointer(cg, ent, colEnemy);
                }
                if (!enemy && drawF) {
                    DrawPlayer(base, cg, i, ent, colFriend, localOrg);
                    if (CB(V_POINTERS) && CB(0x90B4328D) == 0) DrawPointer(cg, ent, colFriend);
                }
                continue;
            }
            if (!PlayerEligible(cg, base, i)) continue;
            char* ci = (char*)cg + CG_CLIENTINFO + i * CI_STRIDE;
            if (RI(ci, 0) == 0 && RB(ci, CI_NAME) == 0) continue;

            bool enemy = pTeam(0, ent) == 0;
            if (enemy && !drawE) continue;
            if (!enemy && !drawF) continue;

            ARGB col = enemy ? colEnemy : colFriend;
            if (enemy && CB(0x90B433DF + i)) col = Pack(CFp(0x90B43A54), 0xFF5EA8D9);
            if (enemy && CB(0x90B433CD + i)) col = Pack(CFp(0x90B43A44), 0xFFFFFFFF);
            if (enemy && CB(0x90B438B0) && aimbot::EspHittable(i)) col = Pack(CFp(0x90B43A14), 0xFFFF8000);
            if (enemy && CB(0x90B438B0) && aimbot::EspVisible(i)) col = Pack(CFp(0x90B43A24), 0xFFFFFF00);
            DrawPlayer(base, cg, i, ent, col, localOrg);
            if (CB(V_POINTERS) && CB(0x90B4328D) == 0) DrawPointer(cg, ent, col);
        }
    }
}  // namespace esp
