#include "internal.hpp"

namespace reconrender {
    FindAsset_t pFindAsset = (FindAsset_t)A_FindAsset;
    DrawText_t pText = (DrawText_t)A_DrawText;
    TextWidth_t pTextW = (TextWidth_t)A_TextWidth;
    Stretch_t pStretch = (Stretch_t)A_StretchPic;
    volatile u16 s_thirdPersonHeadTag = 0;

    u8 g_cfg[CFG_SIZE];
    u8* CfgByte(u32 va) { return Bp(va); }
    float* CfgFloat(u32 va) { return Fp(va); }

    ARGB kColAccent = 0xFFBD89FF;
    ARGB kColText = 0xFFFFFFFF;
    const ARGB kColOff = 0xFF4D4D4D;
    ARGB kColBg = 0xC8151515;
    ARGB kColBar = 0xE0101010;
    ARGB kColBorder = 0xFFBD89FF;

    Vec4 Col(ARGB a) {
        Vec4 c = {((a >> 16) & 0xFF) / 255.0f, ((a >> 8) & 0xFF) / 255.0f, (a & 0xFF) / 255.0f,
                  ((a >> 24) & 0xFF) / 255.0f};

        return c;
    }
    ARGB PackRGBA(const float* rgba, ARGB fallback) {
        if (!rgba) return fallback;

        int r = (int)(rgba[0] * 255.0f);
        int g = (int)(rgba[1] * 255.0f);
        int b = (int)(rgba[2] * 255.0f);
        int a = (int)(rgba[3] * 255.0f);

        if (r < 0) r = 0;
        if (r > 255) r = 255;
        if (g < 0) g = 0;
        if (g > 255) g = 255;
        if (b < 0) b = 0;
        if (b > 255) b = 255;
        if (a < 0) a = 0;
        if (a > 255) a = 255;

        return ((ARGB)a << 24) | ((ARGB)r << 16) | ((ARGB)g << 8) | (ARGB)b;
    }
    void UpdatePalette() {
        kColAccent = PackRGBA(Fp(VA_COL_MENU), 0xFFBD89FFu);
        kColBorder = kColAccent;
        kColText = PackRGBA(Fp(VA_COL_TEXT), 0xFFFFFFFFu);
        int alpha = (int)(*Fp(VA_BG_OPACITY) * 255.0f);
        if (alpha < 0) alpha = 0;
        if (alpha > 255) alpha = 255;
        kColBg = ((ARGB)alpha << 24) | 0x00151515u;
    }

    int s_font = 0;
    int s_white = 0;
    Layout L;

    void ComputeLayout() {
        if (!s_font) s_font = pFindAsset(0x15, "fonts/720/normalFont", 0xFFFFFFFF);
        if (!s_white) s_white = pFindAsset(0x06, "white", 0xFFFFFFFF);

        float w = 1280.0f;
        float h = 720.0f;

        L.scrW = w;
        L.scrH = h;
        L.ui = 1.0f;

        L.emPx = 22.0f;
        L.scale = 0.62f;
        L.rowH = 28.0f;
        L.titleH = 38.0f;
        L.tabH = 32.0f;
        L.barH = 3.0f;
        L.pad = 12.0f;
        L.border = 2.0f;
        L.box = 17.0f;
        L.menuW = 520.0f;
        L.menuX = (w - L.menuW) * 0.5f + *Fp(0x90B43A80);
        L.menuY = 90.0f + *Fp(0x90B43A84);
        L.hudMargin = 44.0f;
    }

    void DrawTextSc(const char* s, float x, float y, float sc, ARGB col) {
        if (!s || !s_font) return;
        Vec4 c = Col(col);
        pText(s, 0x7FFFFFFF, s_font, x, y, sc, sc, 0.0f, &c, 4);
    }
    void DrawTextS(const char* s, float x, float y, ARGB col) { DrawTextSc(s, x, y, L.scale, col); }
    void DrawRect(float x, float y, float w, float h, ARGB col) {
        if (!s_white) return;
        Vec4 c = Col(col);
        pStretch(x, y, w, h, 0.0f, 0.0f, 1.0f, 1.0f, &c, s_white);
    }

    float TextWSc(const char* s, float sc) {
        if (!s || !s[0]) return 0.0f;

        if (s_font) {
            int w = pTextW(0, s, 0x7FFFFFFF, s_font);
            if (w > 0) return (float)w * sc;
        }
        int n = 0;
        while (s[n]) ++n;

        return n * (sc * 21.5f);
    }
    float TextW(const char* s) { return TextWSc(s, L.scale); }

    float VisTextW(const char* s) {
        if (!s || !s[0]) return 0.0f;

        if (s_font) {
            int w = pTextW(0, s, 0x7FFFFFFF, s_font);
            if (w > 0) return (float)w * L.scale;
        }
        int u = 0;
        for (int i = 0; s[i];) {
            if (s[i] == '^' && s[i + 1] == 'B') {
                i += 2;
                while (s[i] && s[i] != '^') ++i;
                if (s[i] == '^') ++i;
                u += 2;
            } else if (s[i] == '^' && s[i + 1]) {
                i += 2;
            } else {
                ++u;
                ++i;
            }
        }

        return u * (L.scale * 21.5f);
    }
    float HudW(const char* s) { return VisTextW(s); }
    void DrawRight(const char* s, float rx, float y, ARGB col) { DrawTextS(s, rx - TextW(s), y, col); }
    float TextY(float rowTop, float rowH) { return rowTop + (rowH + L.emPx) * 0.5f; }
    void DrawBorder(float x, float y, float w, float h, float t, ARGB col) {
        DrawRect(x, y, w, t, col);
        DrawRect(x, y + h - t, w, t, col);
        DrawRect(x, y, t, h, col);
        DrawRect(x + w - t, y, t, h, col);
    }
    const char* kTitle = "luda v1.0.0";

    const char* kAimbotType[] = {"OFF", "Snap", "Silent", "Release"};
    const char* kLeaning[] = {"OFF", "Automatic", "Manual Aiming (Aim + LB/RB)", "Manual (LB/RB)"};
    const char* kDrawType[] = {"OFF", "2D Box", "3D Box", "Corner Box"};
    const char* kDrawEnt[] = {"OFF", "Icon Only", "Box Only", "Box and Icon"};
    const char* kSnaplines[] = {"OFF", "Bottom", "Middle", "Top"};

    const char* kCrosshair[] = {"OFF", "Plus"};
    const char* kCustomNotif[] = {"Default", "Toast"};
    const char* kPreset[] = {"HvH (RIOT)",
                             "Public Rage",
                             "Public Rage (SPIN)",
                             "Heaventh's Config (Resets Colours)",
                             "Cyanokit's Config (Resets Colours)",
                             "Default Config (Resets Colours)"};
    const char* kOpenBind[] = {"LT + DPAD-LEFT",  "LT + X",         "LT + R-STICK",
                               "LT + DPAD-RIGHT", "LT + DPAD-DOWN", "LT + DPAD-UP"};
    const char* kGtColour[] = {"Black", "Red",  "Green",   "Yellow", "Dark Blue",
                               "Cyan",  "Pink", "Default", "Grey",   "Brown"};
    const char* kAutoCrouch[] = {"OFF", "Crouch On Damage", "Crouch On Hittable", "Slither Mode"};
    const char* kAntiYaw[] = {"OFF", "Riot", "Face Away", "Spin", "Dynamic", "Random"};
    const char* kAntiPitch[] = {"OFF", "Riot", "Up", "Down", "Up & Down", "Spin", "Dynamic"};
    const char* kAimTag[] = {"Head End",
                             "Eyes",
                             "Head",
                             "Neck",
                             "Spine4",
                             "Stowed Back",
                             "Upper Spine",
                             "Main Body",
                             "Lower Spine",
                             "Right Shoulder",
                             "Left Shoulder",
                             "Left Clavicle",
                             "Right Clavicle",
                             "Right Shoulder Twist",
                             "Left Shoulder Twist",
                             "Right Shoulder Raise",
                             "Left Shoulder Raise",
                             "Right Elbow",
                             "Left Elbow",
                             "Right Elbow Bulge",
                             "Left Elbow Bulge",
                             "Right Wrist",
                             "Left Wrist",
                             "Right Wrist Twist",
                             "Left Wrist Twist",
                             "Right Mid",
                             "Left Mid",
                             "Lower Back",
                             "Pelvis",
                             "Right Hip",
                             "Left Hip",
                             "Right Hip Twist",
                             "Left Hip Twist",
                             "Right Knee",
                             "Left Knee",
                             "Right Knee Bulge",
                             "Left Knee Bulge",
                             "Right Ankle",
                             "Left Ankle",
                             "Right Toe",
                             "Left Toe",
                             "Left Pinky",
                             "Right Pinky",
                             "Right Index",
                             "Left Index",
                             "Left Middle",
                             "Right Middle",
                             "Left Ring",
                             "Right Ring",
                             "Left Thumb"};
    const char* kCamo[] = {"None",
                           "DEVGRU",
                           "A-TACS AU",
                           "ERDL",
                           "Siberia",
                           "Choco",
                           "Blue Tiger",
                           "Bloodshot",
                           "Ghostex: Delta 6",
                           "Kryptek: Typhon",
                           "Carbon Fiber",
                           "Cherry Blossom",
                           "Art Of War",
                           "Ronin",
                           "Skulls",
                           "Gold",
                           "Diamond",
                           "Elite Member",
                           "CE Digital",
                           "Jungle Warfare",
                           "UK",
                           "Benjamins",
                           "Dia de Muertos",
                           "Graffiti",
                           "Kawaii",
                           "Party Rock",
                           "Zombies",
                           "Viper",
                           "Bacon",
                           "Ghosts",
                           "Paladin",
                           "Cyborg",
                           "Dragon",
                           "Comics",
                           "Aqua",
                           "Breach",
                           "Coyote",
                           "Glam",
                           "Rogue",
                           "Pack-a-Punch",
                           "Dead Man's Hand",
                           "Beast",
                           "Octane",
                           "Weaponized 115",
                           "Afterlife",
                           "Advanced Warfare"};
    const char* kPresetGt[] = {"luda v1.0.0",        "Heaventh",      "My Crow Soft", "Chester",
                               "xbGuard On TOP!",    "DataArmor VPN", "Superiority",  "Heaventh's Pepsi",
                               "discord.gg/xbGuard", "Drink Tea",     "Lucifer"};

    Option g_options[kMaxOptions];
    int g_optCount = 0;

    int g_curPage = PAGE_MAIN;
    int g_sel[PAGE_N] = {0};
    bool g_open = false;
    bool g_cfgInit = false;
    int g_target = 0;
    HANDLE s_keyboardThread = 0;
    volatile bool s_keyboardStop = false;
    volatile int s_keyboardTarget = -1;
    char s_savedRename[18][0x20] = {{0}};
    u8 s_savedRenameSet[18] = {0};
    volatile u32 g_dbgLastAction = 0;
    volatile int g_dbgLastResult = 0;
}

extern "C" {
volatile unsigned int g_rrPhase = 0;
}
extern "C" void RR_DrawText(const char*, float, float, float, unsigned) {}
extern "C" void RR_DrawRect(float, float, float, float, unsigned) {}
