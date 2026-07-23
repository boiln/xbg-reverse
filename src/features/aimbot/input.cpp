#include "internal.hpp"

#include <math.h>
#include <ppcintrinsics.h>
#include <xtl.h>

namespace aimbot {
    static u32 s_synthTick   = 0;
    static int s_prevHealth  = 100;
    bool s_slitherOn         = false;
    static bool s_slitherKey = false;
    bool s_slitherHittable   = false;

    void ApplyInputSynth(unsigned short* btn, unsigned char* rtrig) {

        static const unsigned short XB_B  = 0x2000;
        static const unsigned short XB_LB = 0x0100;
        static const unsigned short XB_RB = 0x0200;

        ++s_synthTick;

        s_autoInput = 0;

        if (rtrig && CB(CFG_AUTOSHOOT) && s_hasTarget) {
            *rtrig      = 0x23;
            s_autoInput = 1;
        }

        int ac        = CB(AC_CROUCH);
        bool dpadLeft = (*btn & 0x0004) != 0;

        if (ac == 3 && dpadLeft && !s_slitherKey) s_slitherOn = !s_slitherOn;
        s_slitherKey = dpadLeft;
        if (ac != 3) s_slitherOn = false;
        s_slitherHittable = ((*btn >> 6) & 1) != 0;

        if (ac == 3 && s_slitherOn && !s_slitherHittable) {
            if ((s_synthTick % 9) == 0) *btn |= XB_B;
        } else if (ac == 2 && s_hasTarget) {
            *btn |= XB_B;
        } else if (ac == 1) {
            void* cg = CG();

            if (cg) {
                int hp = RI(cg, 0x482E0);

                if (hp >= 0 && hp < 1000 && hp < s_prevHealth) *btn |= XB_B;
                if (hp >= 0 && hp < 1000) s_prevHealth = hp;
            }
        }

    }

    static u32 s_reloadTime         = 0;
    static bool s_reloadActive      = false;
    static int s_burstCount         = 0;
    static bool s_burstWindow       = false;
    static bool s_burstActionActive = false;

    typedef void(__cdecl* WeaponActionFn)(int, int, int);

    void OnEntityEvent(void* centity, u32 event) {

        void* cg = CG();
        char* cs = ClientState();
        if (!cg || !cs || !centity) return;

        int owner = RI(cg, 0x00);
        if (*(short*)((char*)centity + 0x2C2)!= owner) return;

        WeaponActionFn action = (WeaponActionFn)0x82256AC0;

        if (CB(CFG_FASTRELOAD)) {
            if (event == 0x17) {
                action(0, 1, 0);
                s_reloadTime   = *(volatile u32*)0x801B0EF0;
                s_reloadActive = true;
            } else if (event == 0x1D) {
                u32 now = *(volatile u32*)0x801B0EF0;

                s_reloadActive = s_reloadActive && (u32)(now - s_reloadTime) < 0x1F5;
                if (s_reloadActive) action(0, 0, 0);
                s_reloadActive = false;
            }
        } else {
            s_reloadActive = false;
        }

        if (!CB(CFG_AUTOBURST)) {
            s_burstCount        = 0;
            s_burstWindow       = false;
            s_burstActionActive = false;

            return;
        }

        if (event == 0x20) {
            if (!pIsZombieSession() && RB(cg, 0x48263) == 0x2C) {
                u32 cmdNum = *(u32*)(cs + CS_CMDNUM);
                char* cur  = cs + CS_CMDRING + (cmdNum & 0x7F) * CMD_STRIDE;
                char* next = cs + CS_CMDRING + ((cmdNum + 1) & 0x7F) * CMD_STRIDE;

                *(u32*)(cur + U_BUTTONS)  |= 0x04000000;
                *(u32*)(next + U_BUTTONS) &= ~0x04000000u;

                if (s_burstCount < 3) {
                    ++s_burstCount;

                    return;
                }

                s_burstCount  = 0;
                s_burstWindow = true;
            }

            char* entities = Entities();
            if (!entities) return;

            u8 weapon       = RB(entities + owner * ENT_STRIDE, 0x1B0);
            void* def       = pWeaponDef(weapon);
            int weaponClass = def ? RI(def, 0x30) : -1;

            if ((weaponClass >= 2 && weaponClass <= 5) || weaponClass == 8) {
                *(u32*)((char*)cg + 0x696A8)= RI(cg, 0x4808C);
                action(0, 1, 0);
                s_burstActionActive = true;
            }

            return;
        }

        if (event == 0x1B && s_burstWindow) {
            s_burstWindow = false;
            if (!s_burstActionActive) return;
            s_burstActionActive = false;
            action(0, 0, 0);

            return;
        }

        if (event == 0x1D && s_burstActionActive) {
            s_burstActionActive = false;
            action(0, 0, 0);
        }

    }
}
