#pragma once

#include "luda/features/aimbot.hpp"
#include "luda/features/esp.hpp"
#include "luda/ui/menu.hpp"

#include <ppcintrinsics.h>
#include <xtl.h>

extern "C" int __stdcall ExCreateThread(HANDLE*, unsigned long, unsigned long*, void*, unsigned long(__stdcall*)(void*),
                                        void*, unsigned long);
extern "C" void XapiThreadStartup(void);
extern "C" unsigned long XamGetCurrentTitleId(void);

namespace reconrender {
    typedef unsigned int u32;
    typedef unsigned short u16;
    typedef unsigned char u8;
    typedef unsigned long long u64;
    typedef signed char s8;
    typedef signed short s16;
    typedef int s32;
    typedef unsigned int ARGB;

    static const u32 BO2_TITLE_ID = 0x415608C3;
    static const u32 A_FindAsset = 0x822CAE50;
    static const u32 A_DrawText = 0x828B8BA0;
    static const u32 A_TextWidth = 0x828B6FD8;
    static const u32 A_StretchPic = 0x828B86C0;
    static const u32 A_Menu_PaintAll = 0x824744E0;
    static const u32 A_XInputGetState = 0x827D8A48;

    static const u32 A_CG_DObjGetWorldTagPos = 0x821D0390;
    static const u32 A_SL_GetString = 0x82533528;

    static const u32 A_BG_GetPlayerEyePosition = 0x82690C70;

    static const u32 A_CG_Player = 0x821D0D18;
    static const u32 A_CG_EntityEvent = 0x821D3D78;

    static const u32 A_BulletEmit = 0x82257DD8;

    static const u32 A_ChatCommandReceive = 0x8242D410;
    static const u32 A_NetChanValidate = 0x8241E438;

    static const u32 A_ServerCommand = 0x8222A3B8;

    static const u32 A_CmdArgv = 0x821BBBD8;
    static const u32 A_Atoi = 0x82916F08;

    static const u32 A_PartyGetActive = 0x8259B878;

    static const u32 A_PartyMemberAddressKey = 0x825B6F38;
    static const u32 A_CL_SendCmd = 0x8225EAA8;

    struct Vec4 {
        float x, y, z, w;
    };
    struct Vec3 {
        float x, y, z;
    };

    typedef int (*FindAsset_t)(unsigned, const char*, int);
    typedef void (*DrawText_t)(const char*, int, int, float, float, float, float, float, const Vec4*, int);
    typedef void (*Stretch_t)(float, float, float, float, float, float, float, float, const Vec4*, int);
    typedef int (*TextWidth_t)(int, const char*, int, int);

    static const u32 CFG_BASE = 0x90B42800;
    static const u32 CFG_SIZE = 0x1600;
    static const u32 VA_COL_MENU = 0x90B43934;
    static const u32 VA_COL_TEXT = 0x90B43944;
    static const u32 VA_BG_OPACITY = 0x90B43930;

    struct Layout {
        float scrW, scrH, ui, scale, emPx;
        float menuX, menuY, menuW, rowH, titleH, tabH, barH, pad, border, box;
        float hudMargin;
    };

    enum { K_ACTION = 0, K_TOGGLE, K_INFO, K_SLIDER, K_COLOR, K_ENUM, K_ACTION_PLAIN, K_ENUM_TOGGLE, K_SLIDER_TOGGLE };
    enum { FMT_0DEC = 0, FMT_1DEC, FMT_2DEC, FMT_MS, FMT_255 };
    struct Option {
        const char* label;
        int kind;
        u8* backByte;
        float* backFloat;
        const char** names;
        int names_n;
        float lo, hi, step;
        int fmt;
        int action;
        u8* auxByte;
    };
    static const int kMaxOptions = 72;

    enum {
        PAGE_MAIN = 0,
        PAGE_AIMBOT = 1,
        PAGE_VISUAL = 2,
        PAGE_PLAYER = 3,
        PAGE_SETTINGS = 4,
        PAGE_RECOVERY = 5,
        PAGE_GAMERTAG = 6,
        PAGE_PRESETGT = 7,
        PAGE_PLAYERACT = 8,
        PAGE_HOST = 9,
        PAGE_COLOURS = 0xA,
        PAGE_ED_MENU = 0xB,
        PAGE_ED_SCROLL = 0xC,
        PAGE_ESPCOL = 0xD,
        PAGE_ED_ENEMY = 0xE,
        PAGE_ED_HITTABLE = 0xF,
        PAGE_ED_VISIBLE = 0x10,
        PAGE_ED_PRIO = 0x11,
        PAGE_ED_WL = 0x12,
        PAGE_ED_FRIENDLY = 0x13,
        PAGE_ED_SMOKE = 0x14,
        PAGE_ED_UI = 0x15,
        PAGE_UIELEM = 0x16,
        PAGE_ED_TEXT = 0x17,
        PAGE_THIRDP = 0x18,
        PAGE_ANTIAIM = 0x19,
        PAGE_ADVANCED = 0x1A,
        PAGE_CUSTOMGT = 0x1B,
        PAGE_PROFILER = 0x1C,
        PAGE_N = 0x1D
    };

    static const int ACT_SELPLAYER = 0x2200;
    static const int ACT_PRIORITY_ADD = 0x1210;
    static const int ACT_PRIORITY_REMOVE = 0x1211;
    static const int ACT_WHITELIST_ADD = 0x1212;
    static const int ACT_WHITELIST_REMOVE = 0x1213;
    static const int ACT_APPLY_PRESET = 0x1012;

    static const u32 A_CLIENT_INFO = 0x82CAC368u;
    static const u32 CLIENT_INFO_STRIDE = 0x148u;
    static const u32 A_CG_POINTER = 0x82BBAE68u;
    static const u32 A_CGS_POINTER = 0x82BBAE44u;
    static const u32 CG_CLIENTINFO = 0x69A90u;
    static const u32 CG_CLIENTINFO_STRIDE = 0x808u;

    struct LocalIdSet {
        u64 ids[18];
        int count;
    };

    extern FindAsset_t pFindAsset;
    extern DrawText_t pText;
    extern TextWidth_t pTextW;
    extern Stretch_t pStretch;
    extern volatile u16 s_thirdPersonHeadTag;
    extern u8 g_cfg[CFG_SIZE];
    extern ARGB kColAccent;
    extern ARGB kColText;
    extern const ARGB kColOff;
    extern ARGB kColBg;
    extern ARGB kColBar;
    extern ARGB kColBorder;
    extern int s_font;
    extern int s_white;
    extern Layout L;

    extern const char* kTitle;
    extern const char* kAimbotType[];
    extern const char* kLeaning[];
    extern const char* kDrawType[];
    extern const char* kDrawEnt[];
    extern const char* kSnaplines[];
    extern const char* kCrosshair[];
    extern const char* kCustomNotif[];
    extern const char* kPreset[];
    extern const char* kOpenBind[];
    extern const char* kGtColour[];
    extern const char* kAutoCrouch[];
    extern const char* kAntiYaw[];
    extern const char* kAntiPitch[];
    extern const char* kAimTag[];
    extern const char* kCamo[];
    extern const char* kPresetGt[];

    extern Option g_options[kMaxOptions];
    extern int g_optCount;
    extern int g_curPage;
    extern int g_sel[PAGE_N];
    extern bool g_open;
    extern bool g_cfgInit;
    extern int g_target;
    extern HANDLE s_keyboardThread;
    extern volatile bool s_keyboardStop;
    extern volatile int s_keyboardTarget;
    extern char s_savedRename[18][0x20];
    extern u8 s_savedRenameSet[18];
    extern volatile u32 g_dbgLastAction;
    extern volatile int g_dbgLastResult;
    extern LocalIdSet s_priorityIds;
    extern LocalIdSet s_whitelistIds;
    extern LocalIdSet s_detectedIds;
    extern const int kTopTabs[5];
    extern const char* const kTabLabels[5];
    extern const char kFreezeGt[];

    enum {
        BTN_UP = 0x0001,
        BTN_DOWN = 0x0002,
        BTN_LEFT = 0x0004,
        BTN_RIGHT = 0x0008,
        BTN_RTHUMB = 0x0080,
        BTN_LB = 0x0100,
        BTN_RB = 0x0200,
        BTN_LT = 0x0400,
        BTN_A = 0x1000,
        BTN_B = 0x2000,
        BTN_X = 0x4000
    };

    inline u8* Bp(u32 va) { return &g_cfg[va - CFG_BASE]; }
    inline float* Fp(u32 va) { return (float*)&g_cfg[va - CFG_BASE]; }

    Vec4 Col(ARGB color);
    ARGB PackRGBA(const float* rgba, ARGB fallback);
    void UpdatePalette();
    void ComputeLayout();
    void DrawTextSc(const char* text, float x, float y, float scale, ARGB color);
    void DrawTextS(const char* text, float x, float y, ARGB color);
    void DrawRect(float x, float y, float width, float height, ARGB color);
    float TextWSc(const char* text, float scale);
    float TextW(const char* text);
    float VisTextW(const char* text);
    float HudW(const char* text);
    void DrawRight(const char* text, float right, float y, ARGB color);
    float TextY(float rowTop, float rowHeight);
    void DrawBorder(float x, float y, float width, float height, float thickness, ARGB color);

    void addToggle(const char* label, u32 address);
    void addEnum(const char* label, const char** names, int count, u32 address);
    void addEnumAction(const char* label, const char** names, int count, u32 address, int action);
    void addEnumToggle(const char* label, const char** names, int count, u32 enumAddress, u32 enableAddress);
    void addSlider(const char* label, u32 address, float low, float high, float step, int format);
    void addSliderToggle(const char* label, u32 valueAddress, u32 enableAddress, float low, float high, float step,
                         int format);
    void addAction(const char* label, int action);
    void addPlainAction(const char* label, int action);
    void addInfo(const char* label);
    void buildColorEditor(const char* title, u32 colorAddress, u32 rainbowAddress, u32 pulseAddress);
    bool ContainsStr(const char* haystack, const char* needle);
    bool SameStr(const char* left, const char* right);
    u64 PlayerXuid(int slot);
    const char* ClientTableName(int slot);
    u32 PlayerMenuCg();
    bool PlayerMenuIsHost();
    bool PlayerMenuHostMode(u32 cg);
    int PlayerMenuHostIndex();
    const char* HostSlotName(u32 cg, int slot);
    bool DetectedModderXuid(u64 xuid);
    bool PersistentSetContains(u32 setAddress, LocalIdSet& fallback, u64 id);
    bool PersistentSetWrite(u32 setAddress, LocalIdSet& fallback, u64 id, bool enabled);
    void PersistentSetClear(u32 setAddress, LocalIdSet& fallback);
    bool EngineClientArrayReady();
    bool ForceStartEligible();

    void BuildPage(int page);
    int TopTabIndex(int page);
    int ParentPage(int page);
    void ItoA(int value, char* output);
    void UtoA(u32 value, char* output);
    void U64ToHex(u64 value, char* output);
    void Adjust(Option& option, int direction);
    void SetRGBA(u32 address, float r, float g, float b, float a);
    void RenderMenu();
    void TickFps();
    bool BeginWorldFrame();
    void WatermarkHUD();
    void DrawLegend();

    void ConfigDefaults();
    void GtFlush(void* address);
    bool RealEngineResident();
    void RunCmd(const char* command);
    void SetMyGamertag(const char* name);
    void ApplyPresetConfig(int mode);
    void ResolveThirdPersonHeadTag();

    void ApplyMainConfig();
    void ApplySettingsConfig();
    void ApplyGodMode();
    bool BeginRenamePlayer(int target);
    bool StrHas(const char* haystack, const char* needle);
    const char* DetectMenuName(const char* name);
    void SendCrashPlayerExact(int target);

    void NotifyMsg(const char* message, int milliseconds);
    void NotifyPlayer(const char* prefix, const char* name, int milliseconds);
    void NotifyPlayerFull(const char* prefix, const char* name, const char* suffix, int milliseconds);
    bool DetectMenu_Scan(bool showNoMenu);
    void Frame();
    void Input(u16 buttons, u8 leftTrigger);
}
