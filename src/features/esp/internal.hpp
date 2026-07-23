#pragma once

#include "luda/features/aimbot.hpp"
#include "luda/features/esp.hpp"

namespace reconrender {
    unsigned char* CfgByte(unsigned address);
    float* CfgFloat(unsigned address);
}

namespace esp {
    typedef unsigned int u32;
    typedef unsigned char u8;
    typedef unsigned long long u64;
    typedef unsigned int ARGB;

    struct Vec2 {
        float x, y;
    };
    struct Vec3 {
        float x, y, z;
    };

    static const u32 A_CG_Pointer      = 0x82BBAE68;
    static const u32 A_CGS_Pointer     = 0x82BBAE44;
    static const u32 A_EntitiesPointer = 0x82BBC554;
    static const u32 A_ScreenPlacement = 0x82CBC168;
    static const u32 A_FindAsset       = 0x822CAE50;
    static const u32 A_DrawText        = 0x828B8BA0;
    static const u32 A_TextWidth       = 0x828B6FD8;
    static const u32 A_StretchPic      = 0x828B86C0;
    static const u32 A_RotPic          = 0x821C7F58;
    static const u32 A_ClientDObj      = 0x82414578;
    static const u32 A_TagPos          = 0x821D03F0;
    static const u32 A_TeamCheck       = 0x821CD948;
    static const u32 A_SLGetString     = 0x82533528;
    static const u32 A_WeaponDef       = 0x826BF988;
    static const u32 A_WeaponDef2      = 0x826BF970;
    static const u32 A_GetNumWeapons   = 0x826BF960;
    static const u32 A_LocalizeString  = 0x82442F68;
    static const u32 A_ModelIndex      = 0x826C06E8;
    static const u32 A_CalcMuzzle      = 0x822546F8;
    static const u32 A_DObjBounds      = 0x824CBFE0;
    static const u32 A_DrawTracer3D    = 0x822F1E30;
    static const u32 A_IsZombieSession = 0x82406578;

    static const int ENT_STRIDE = 0x374;
    static const u32 CG_ORIGIN = 0xB8, CG_CLIENTNUM = 0x230, CG_REFDEF = 0x4D888, CG_CLIENTINFO = 0x69A90;
    static const u32 CI_STRIDE = 0x808, CI_NAME = 0x0C, CI_TEAM = 0x34;

    static const u32 RD_W = 0x00, RD_H = 0x04, RD_FOV = 0x1C, RD_ORG = 0x30, RD_AXIS = 0x40;

    static const u32 E_ORIGIN = 0x2C, E_AIMING = 0x155, E_ANGLES = 0x38, E_WEAPIDX = 0x1B0, E_TYPE = 0x2A8,
    E_WEAPNUM = 0x2B7, E_ALIVE = 0x36C, E_FLAGS = 0x154;

    typedef int(* FindAsset_t)(unsigned, const char*, int);
    typedef void(* DrawText_t)(const char*, int, int, float, float, float, float, float,
        const void*, int);
    typedef void(* Stretch_t)(float, float, float, float, float, float, float, float, const void*,
        int);
    typedef void(* RotPic_t)(int, float, float, float, float, float, const void*, int);
    typedef int(* ClientDObj_t)(int, int);
    typedef short(* SLGet_t)(const char*, int);
    typedef int(* TagPos_t)(void*, int, int, Vec3*);
    typedef void*(* WeaponDef_t)(int);
    typedef int(* GetNumWeapons_t)(void);
    typedef const char*(* LocalizeString_t)(const char*);
    typedef int(* ModelIndex_t)(const char*);
    typedef int(* CalcMuzzle_t)(int, void*, u32, void*, int, int, int, void*, Vec3*, void*, void*,
        void*, void*, void*, void*);
    typedef void(* DObjBounds_t)(void*, Vec3*, Vec3*);
    typedef void(* DrawTracer3D_t)(const void*);
    typedef char(* SessionPredicate_t)(void);
    typedef int(* TextWidth_t)(int, const char*, int, int);
    typedef char(* TeamCheck_t)(int, void*);

    enum {
        V_RADAR = 0x90B433AA,
        V_TRACERS = 0x90B438BF,
        V_ENEMY = 0x90B433AB,
        V_FRIENDLY = 0x90B433AC,
        V_NAMES = 0x90B433B4,
        V_DIST = 0x90B433B5,
        V_SKELETON = 0x90B433AE,
        V_POINTERS = 0x90B433AD,
        V_WEAPNAMES = 0x90B433B6,
        V_TYPE = 0x90B433C4,
        V_ENTITIES = 0x90B433BC,
        V_SCAVENGER = 0x90B433B8,
        V_SNAPLINES = 0x90B433B0,
        V_HEALTHBAR = 0x90B433C0,
        V_CROSSHAIR = 0x90B433C8,
        V_DISABLECROSS = 0x90B433C1,
        C_ENEMY = 0x90B439C4,
        C_FRIENDLY = 0x90B439F4,
        C_TEXT = 0x90B43944,
        C_BOX = 0x90B43924,
        C_LABEL_TEXT = 0x90B43964,
        C_ACCENT = 0x90B43934
    };

    struct TracerRecord {
        Vec3 start, end;
        u32 born;
        float rgba[4];
    };

    extern FindAsset_t pFind;
    extern DrawText_t pText;
    extern TextWidth_t pTextW;
    extern Stretch_t pStretch;
    extern RotPic_t pRot;
    extern ClientDObj_t pDObj;
    extern SLGet_t pSL;
    extern TagPos_t pTag;
    extern TeamCheck_t pTeam;
    extern WeaponDef_t pWDef;
    extern WeaponDef_t pWDef2;
    extern GetNumWeapons_t pGetNumWeapons;
    extern LocalizeString_t pLocalizeString;
    extern ModelIndex_t pModelIndex;
    extern CalcMuzzle_t pCalcMuzzle;
    extern DObjBounds_t pDObjBounds;
    extern DrawTracer3D_t pDrawTracer3D;
    extern SessionPredicate_t pIsZombieSession;

    extern int s_font;
    extern int s_white;
    extern int s_radarState;
    extern int s_crossState;
    extern bool s_radarOwn;
    extern bool s_crossOwn;
    extern ARGB s_accent;
    static const float kScale = 0.5f;
    extern TracerRecord s_tracers[19];
    extern int s_tracerCount;

    inline u8 CB(u32 va) { return *reconrender::CfgByte(va); }
    inline float* CFp(u32 va) { return reconrender::CfgFloat(va); }
    inline float RF(void* p, u32 o) { return *(float*)((char*)p + o); }
    inline int RI(void* p, u32 o) { return *(int*)((char*)p + o); }
    inline u8 RB(void* p, u32 o) { return *(u8*)((char*)p + o); }
    inline Vec3 RV3(void* p, u32 o) {
        Vec3 v = { RF(p, o), RF(p, o + 4), RF(p, o + 8) };

        return v;
    }

    void* CG();
    char* Entities();
    char* Entity(void* base, int index);
    void Vec4Of(ARGB color, float* channels);
    ARGB Pack(const float* rgba, ARGB fallback);
    bool W2S(void* cg, const Vec3& world, Vec2* output);
    float TextWSc(const char* text, float scale);
    int FontH();
    float TextW(const char* text);
    void DrawText(const char* text, float x, float y, ARGB color);
    void DrawRect(float x, float y, float width, float height, ARGB color);
    void DrawLine(float x0, float y0, float x1, float y1, ARGB color);
    int TracerChannel(float value);
    bool TagPos(void* base, int index, const char* tag, Vec3* output);
    bool PlayerEligible(void* cg, char* base, int index);
    void DrawTracerList(void* cg);
    void BoxedLabel(float centerX, float baselineY, const char* text, ARGB border);
    void Crosshair(void* cg);
    void LocalHealthBar(void* cg);
    void DisableNativeCrosshair(int enabled);
    void Box3D(void* base, void* cg, int index, char* entity, ARGB color);
    void DrawPointer(void* cg, char* entity, ARGB color);
    void DrawPlayer(void* base, void* cg, int index, char* entity, ARGB color,
        const Vec3& localOrigin);
    void ConstantRadar(bool enabled);
    void DrawWorldItems(void* cg, char* base);
    bool ZombieEntityEligible(char* entity, int index);
    bool ZombieEntityIsEnemy(char* entity);
}
