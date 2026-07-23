#pragma once

#include "luda/features/aimbot/auto_bone.hpp"
#include "luda/features/aimbot/auto_wall.hpp"

namespace reconrender {
    unsigned char* CfgByte(unsigned address);
}

namespace aimbot {
    typedef unsigned int u32;
    typedef unsigned char u8;
    typedef unsigned long long u64;

    struct Vec3 {
        float x, y, z;
    };
    struct Vec2 {
        float x, y;
    };

    static const u32 A_CG_Pointer = 0x82BBAE68;
    static const u32 A_EntitiesPointer = 0x82BBC554;
    static const u32 A_TeamCheck = 0x821CD948;
    static const u32 A_LocalClient = 0x82C70F4C;
    static const u32 A_ClientDObj = 0x82414578;
    static const u32 A_TagPos = 0x821D03F0;
    static const u32 A_SLGetString = 0x82533528;
    static const u32 A_LocationalTrace = 0x8225C568;
    static const u32 A_WeaponDef = 0x826BF988;
    static const u32 A_GetViewmodelWeapon = 0x826B40D8;
    static const u32 A_GetSpreadForWeapon = 0x826BB4E0;
    static const u32 A_SeedScramble = 0x826961B8;
    static const u32 A_RandomFloat = 0x82696250;
    static const u32 A_AdvanceSeed4 = 0x82697FC0;
    static const u32 A_BulletTrace = 0x8224CDA8;
    static const u32 A_IsDualWield = 0x826BDC48;
    static const u32 A_IsZombieSession = 0x82406578;
    static const u32 A_GetPlayerViewOrigin = 0x822544B0;

    static const u32 A_CalcEntityLerpPositions = 0x821CBC88;

    static const u32 CS_VPITCH = 0x108, CS_VYAW = 0x10C;

    static const u32 CS_MVPITCH = 0x2C8C, CS_MVYAW = 0x2C90, CS_MVROLL = 0x2C94;

    static const u32 CS_CMDRING = 0x42CA8, CS_CMDNUM = 0x44AA8, CMD_STRIDE = 0x3C;

    static const u32 CG_VIEWANG = 0x482A0;
    static const u32 CG_AXIS_FWD = 0x4D8C8, CG_AXIS_RIGHT = 0x4D8D4, CG_AXIS_UP = 0x4D8E0;

    static const int ENT_STRIDE = 0x374;
    static const u32 CG_CLIENTNUM = 0x230, CG_REFDEF = 0x4D888, CG_CLIENTINFO = 0x69A90;
    static const u32 CI_STRIDE = 0x808, CI_NAME = 0x0C;
    static const u32 RD_W = 0x00, RD_H = 0x04, RD_FOV = 0x10, RD_ORG = 0x30, RD_AXIS = 0x40;
    static const u32 E_ORIGIN = 0x2C, E_ALIVE = 0x36C;

    static const u32 U_SERVERTIME = 0x00, U_BUTTONS = 0x04, U_PITCH = 0x0C, U_YAW = 0x10;
    static const u32 BTN_ATTACK = 0x80000000u;

    static const u32 CFG_TYPE = 0x90B438B0;
    static const u32 CFG_HEAD = 0x90B438C2;
    static const u32 CFG_AIMTAG = 0x90B438CC;
    static const u32 CFG_AUTOSHOOT = 0x90B438B8;
    static const u32 CFG_AUTOWALL = 0x90B438C3;
    static const u32 CFG_AUTOBONE = 0x90B438C1;
    static const u32 CFG_RECOIL = 0x90B438BA;
    static const u32 CFG_SWAY = 0x90B438BD;
    static const u32 CFG_FLINCH = 0x90B438BC;
    static const u32 CFG_SPREAD = 0x90B438BB;
    static const u32 CFG_FASTRELOAD = 0x90B438BE;
    static const u32 CFG_POSITION_PREDICT = 0x90B43BD4;
    static const u32 CFG_POSITION_PREDICT_AMOUNT = 0x90B43BD8;
    static const u32 CFG_POSITION_CORRECTION = 0x90B43A70;
    static const u32 CFG_PING_PREDICT = 0x90B43A88;
    static const u32 CFG_PREDICTION_AMOUNT = 0x90B43A6C;
    static const u32 CFG_SHOWAA = 0x90B438E4;

    static const u32 E_YAW = 0x3C;

    static const u32 E_CLIENTNUM = 0x1D0, E_TYPE2 = 0x2A8, E_MODELPITCH_INV = 0x224, E_MODELPITCH = 0x90;
    static const u32 CFG_THIRDPERSON = 0x90B433A1;
    static const u32 CI_FAKEPITCH    = 0x4C4;
    static const float AA_PITCH_REF  = 20.0f;
    static const u32 CFG_WHITELIST   = 0x90B433CD;
    static const u32 CFG_PRIORITY    = 0x90B433DF;

    static const u32 AA_ENABLE      = 0x90B43895;
    static const u32 AA_YAW         = 0x90B438A0;
    static const u32 AA_PITCH       = 0x90B438A4;
    static const u32 AA_SPINYAW     = 0x90B438AC;
    static const u32 AA_SPINPIT     = 0x90B438A8;
    static const u32 MODE_SPECTATOR = 0x90B4328E;

    static const float AA_UP     = -70.0f;
    static const float AA_DOWN   = 70.0f;
    static const float AA_TRACKP = -40.0f;
    static const float AA_PSPIN  = 8.0f;
    static const float AA_YDYN   = 36.0f;
    static const float AA_CLAMP  = 70.0f;
    static const u32 CG_A2A      = 0x482A4;

    static const float DEG2SHORT   = 182.044449f;
    static const float PITCH_CLAMP = 89.0f;
    static const u32 CFG_FOV       = 0x90B433A4;
    static const float RAD2DEG     = 57.2957795f;

    static const u32 CFG_LEAN      = 0x90B438B4;
    static const u32 CFG_AUTOBURST = 0x90B438C0;
    static const u32 CFG_RAPID     = 0x90B438B9;
    static const u32 AC_CROUCH     = 0x90B43898;

    inline u8 CB(u32 va) { return *reconrender::CfgByte(va); }
    inline float CF(u32 va) { return *(float*)reconrender::CfgByte(va); }
    inline float RF(void* p, u32 o) { return *(float*)((char*)p + o); }
    inline int RI(void* p, u32 o) { return *(int*)((char*)p + o); }
    inline u8 RB(void* p, u32 o) { return *(u8*)((char*)p + o); }
    inline Vec3 RV3(void* p, u32 o) {
        Vec3 v = { RF(p, o), RF(p, o + 4), RF(p, o + 8) };

        return v;
    }

    typedef char(* TeamCheck_t)(int, void*);
    typedef void*(* WeaponDef_t)(int);
    typedef int(* GetViewmodelWeapon_t)(void*);
    typedef void(* GetSpreadForWeapon_t)(void*, u32, float*, float*);
    typedef void(* SeedFn_t)(u32*);
    typedef float(* RandomFloat_t)(u32*);
    typedef int(* BulletTrace_t)(int, void*, void*, void*, u32, u32);
    typedef char(* WeaponPredicate_t)(u32);
    typedef char(* SessionPredicate_t)();
    typedef void(* GetPlayerViewOrigin_t)(int, void*, Vec3*);
    typedef void(* CalcEntityLerpPositions_t)(void*, void*, int);
    typedef int(* ClientDObj_t)(int, int);
    typedef short(* SLGet_t)(const char*, int);
    typedef int(* TagPos_t)(void*, int, int, Vec3*);

    extern TeamCheck_t pTeam;
    extern WeaponDef_t pWeaponDef;
    extern GetViewmodelWeapon_t pGetViewmodelWeapon;
    extern GetSpreadForWeapon_t pGetSpreadForWeapon;
    extern SeedFn_t pSeedScramble;
    extern RandomFloat_t pRandomFloat;
    extern SeedFn_t pAdvanceSeed4;
    extern BulletTrace_t pBulletTrace;
    extern WeaponPredicate_t pIsDualWield;
    extern SessionPredicate_t pIsZombieSession;
    extern GetPlayerViewOrigin_t pGetPlayerViewOrigin;
    extern CalcEntityLerpPositions_t pCalcEntityLerpPositions;
    extern ClientDObj_t pDObj;
    extern SLGet_t pSL;
    extern TagPos_t pTag;

    struct BoneEvalContext {
        void* cg;
        char* entities;
        int localIdx;
        int targetIdx;
        Vec3 eye;
        bool allowPenetration;
        bool lastDirect;
    };

    extern volatile unsigned char s_espHittable[18];
    extern volatile unsigned char s_espVisible[18];
    extern volatile float s_viewYaw;
    extern volatile float s_viewPitch;
    extern const int kBoneCount;
    extern autobone::SelectState s_boneSelectState[18];
    extern volatile int s_autoInput;
    extern volatile int s_autoCmd;
    extern volatile bool s_hasTarget;
    extern volatile bool s_aaHasTarget;
    extern volatile bool s_targetVisible;
    extern volatile float s_aimYaw;
    extern volatile float s_aimPitch;
    extern volatile float s_aaAimYaw;
    extern volatile float s_aaAimPitch;
    extern float s_spinYaw;
    extern float s_spinPitch;
    extern int s_aaTogYaw;
    extern int s_aaTogPit;
    extern u32 s_aaRand;
    extern volatile bool s_aaActive;
    extern volatile float s_aaFakeYaw;
    extern volatile float s_aaFakePitch;
    extern volatile float s_aaFakeYawDelta;
    extern volatile int s_call;
    extern volatile int s_reach;
    extern volatile int s_cand;
    extern volatile int s_cost;
    extern volatile int s_wrote;
    extern volatile int s_cmdIn;
    extern volatile int s_myAim;
    extern volatile int s_path;
    extern volatile int s_pathMode;
    extern volatile int s_pathHasTarget;
    extern volatile unsigned s_csVal;
    extern volatile int s_csNibble;
    extern bool s_slitherOn;
    extern bool s_slitherHittable;

    void ApplyAimbotPatches(void* cg);
    float AimFovPixels();
    void* CG();
    char* ClientState();
    void ViewRef(float* pitch, float* yaw);
    char* Entities();
    bool WorldToScreen(void* cg, const Vec3& world, Vec2* out);
    bool TagPos(char* base, int index, const char* tag, Vec3* out);
    void ApplyAimPrediction(void* cg, char* entities, int targetIndex, Vec3* point);
    bool DirectBulletTrace(void* cg, char* entities, int localIndex, int targetIndex,
        const Vec3& start, const Vec3& end);
    bool ResolveDirectBone(void* context, u8 selector, autobone::Vec3* position);
    bool TryBoneSelector(void* context, u8 selector, autobone::Vec3* position, float* bestDamage);
    float NormDeg(float angle);
    int ANGLE2SHORT(float degrees);
    void AngleVectors(float pitch, float yaw, float roll, float* forward, float* right, float* up);
    void SteerView(void* cg, float pitch, float yaw);
    u32 AaRand();
}
