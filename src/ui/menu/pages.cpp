#include "internal.hpp"

namespace reconrender {
    void BuildPage(int page) {

        g_optCount = 0;

        switch (page) {
        case PAGE_MAIN:
            addToggle("Anti End Game + Anti Server Crash (HOST)", 0x90B4339C);
            addToggle("Anti Change Name (HOST)", 0x90B4339D);
            addToggle("Anti RME", 0x90B4339F);
            addToggle("Third Person", 0x90B433A1);
            addToggle("Laser Sight", 0x90B433A2);
            addAction("Crash Server", 0x1000);
            addAction("End Game", 0x1001);
            addAction("Leave Game", 0x1002);
            addAction("Detect Menu", 0x1003);
            if (ForceStartEligible()) addAction("Force Start Game", 0x1004);
            addToggle("Auto Host Menu Detection", 0x90B43BDC);
            addSlider("Field Of View", 0x90B433A4, 45.0f, 160.0f, 1.0f, FMT_2DEC);
            addAction("Recovery Menu", PAGE_RECOVERY);
            addAction("Gamertag Menu", PAGE_GAMERTAG);
            addAction("Third Person Editor", PAGE_THIRDP);
            break;
        case PAGE_AIMBOT:
            addEnum("Aimbot Type", kAimbotType, 4, 0x90B438B0);
            addEnum("Aim Tag", kAimTag, 50, 0x90B438CC);
            addToggle("Always Check Head", 0x90B438C2);
            addToggle("Auto Bone", 0x90B438C1);
            addToggle("Auto Wall", 0x90B438C3);
            addToggle("Auto Shoot", 0x90B438B8);
            addEnum("Leaning", kLeaning, 4, 0x90B438B4);
            addToggle("Remove Recoil", 0x90B438BA);
            addToggle("Remove Sway", 0x90B438BD);
            if (*Bp(0x90B438B0)) addToggle("Remove Spread", 0x90B438BB);
            addToggle("Remove Flinch", 0x90B438BC);
            addToggle("Fast Reload", 0x90B438BE);
            addToggle("Rapid Fire", 0x90B438B9);
            addToggle("Auto Burst", 0x90B438C0);
            addAction("Advanced Settings", PAGE_ADVANCED);
            addAction("Anti-Aim Menu", PAGE_ANTIAIM);
            break;
        case PAGE_VISUAL:
            if (*Bp(0x90B4328E) == 0) addToggle("Constant Radar", 0x90B433AA);
            addToggle("Bullet Tracers", 0x90B438BF);
            addToggle("Draw Enemy", 0x90B433AB);
            addToggle("Draw Frendly", 0x90B433AC);
            addToggle("Draw Names", 0x90B433B4);
            addToggle("Draw Distance", 0x90B433B5);
            addToggle("Draw Skeleton", 0x90B433AE);
            addToggle("Draw Pointers", 0x90B433AD);
            addToggle("Draw Weapon Names", 0x90B433B6);
            addEnum("Draw Type", kDrawType, 4, 0x90B433C4);
            addEnum("Draw Entities", kDrawEnt, 4, 0x90B433BC);
            addEnum("Draw Scavenger Packs", kDrawEnt, 4, 0x90B433B8);
            addEnum("Draw Snaplines", kSnaplines, 4, 0x90B433B0);
            addToggle("Draw Health Bar", 0x90B433C0);
            addToggle("Disable Crosshair", 0x90B433C1);
            addEnum("Custom Crosshair", kCrosshair, 2, 0x90B433C8);
            break;
        case PAGE_PLAYER: {
                static char nmbuf[18][64];
                int hostIdx   = PlayerMenuHostIndex();
                u32 cg        = PlayerMenuCg();
                bool hostMode = PlayerMenuHostMode(cg);

                for (int i = 0; i < 18; ++i) {
                    u64 xuid = PlayerXuid(i);
                    const char* nm = hostMode ? HostSlotName(cg, i) :(xuid ? ClientTableName(i) : 0);

                    if (!nm) {
                        addInfo(hostMode ? "No Player" : "No Online Player");
                        continue;
                    }

                    char* d = nmbuf[i];
                    int k   = 0;

                    if (i == hostIdx)
                        for (const char* t = "[^6H^7]"; *t && k < 58;) d[k++] = *t++;

                    if (DetectedModderXuid(xuid))
                        for (const char* t = "[^1M^7]"; *t && k < 58;) d[k++] = *t++;

                    d[k++] = ' ';
                    int copied = 0;

                    for (const char* s = nm; *s && copied < 31 && k < 62; ++s, ++copied) {
                        char c = *s;

                        if (c == '^' && !(s[1] >= '0' && s[1] <= '9')) c = ' ';
                        d[k++] = c;
                    }

                    d[k] = 0;
                    addPlainAction(nmbuf[i], ACT_SELPLAYER + i);
                }

                break;
            }
        case PAGE_PLAYERACT: {
                u64 xuid = PlayerXuid(g_target);

                if (!xuid) {
                    addToggle("Prioritize", 0x90B433DF + g_target);
                    addToggle("Whitelist", 0x90B433CD + g_target);
                } else {
                    bool prioritized = PersistentSetContains(0x90B4A454u, s_priorityIds, xuid);
                    bool whitelisted = PersistentSetContains(0x90B4A444u, s_whitelistIds, xuid);

                    *Bp(0x90B433DFu + (u32)g_target) = prioritized ? 1 : 0;
                    *Bp(0x90B433CDu + (u32)g_target) = whitelisted ? 1 : 0;
                    addPlainAction(prioritized ? "Remove From Priority List" : "Add To Priority List",
                        prioritized ? ACT_PRIORITY_REMOVE : ACT_PRIORITY_ADD);
                    addPlainAction(whitelisted ? "Remove From Whitelist" : "Add To Whitelist",
                        whitelisted ? ACT_WHITELIST_REMOVE : ACT_WHITELIST_ADD);
                }

                addPlainAction("Give Migration Screen", 0x1200);
                addPlainAction("Offhost Kick", 0x1201);
                addPlainAction("Offhost Crash", 0x1202);
                addPlainAction("Steal Gamertag", 0x1203);
                if (xuid) addPlainAction("Set Gamertag As Their IP", 0x1204);
                if (EngineClientArrayReady()) addPlainAction("Host Options", PAGE_HOST);
                break;
            }
        case PAGE_SETTINGS:
            addAction("Menu Customization", PAGE_COLOURS);
            addToggle("Disable VSync", 0x90B438D1);
            addToggle("Disable DLCs", 0x90B42A4C);
            addToggle("Force Host", 0x90B42A40);
            addToggle("Never Host", 0x90B42A35);
            addToggle("Modded Camos", 0x90B438D0);

            addEnumToggle("Custom Notifications", kCustomNotif, 2, 0x90B43398, 0x90B43394);

            addEnumAction("Load Preset Config", kPreset, 6, 0x90B438E0, ACT_APPLY_PRESET);
            addToggle("Unlock Camos", 0x90B438D2);

            addEnumToggle("Set Camo In Game", kCamo, 46, 0x90B438DC, 0x90B438D3);
            addEnum("Open Bind", kOpenBind, 6, 0x90B43A64);
            addToggle("Streamer Mode", 0x90B43BDD);
            addPlainAction("Clear Prioritize List", 0x1010);
            addPlainAction("Clear Whitelist", 0x1011);
            break;
        case PAGE_RECOVERY:
            addAction("Unlock All", 0x1020);
            addAction("Reset Stats", 0x1021);
            break;
        case PAGE_GAMERTAG:
            addAction("Set Gamertag", 0x1030);
            addAction("Set Clantag", 0x1031);
            addAction("Freeze Gamertag", 0x1032);
            addAction("Reset Gamertag", 0x1033);
            addSlider("Gamertag Effect Delay", 0x90B43A90, 10.0f, 5000.0f, 10.0f, FMT_MS);
            addEnum("Gamertag Colour A", kGtColour, 10, 0x90B42A44);
            addEnum("Gamertag Colour B", kGtColour, 10, 0x90B42A3C);
            addToggle("Rainbow Gamertag", 0x90B4338C);
            addToggle("Highlighter Gamertag", 0x90B42A43);
            addToggle("Police Gamertag", 0x90B42A42);
            addToggle("SCC Gamertag", 0x90B42A41);
            addAction("Preset Gamertags", PAGE_PRESETGT);
            addAction("Custom Gamertags", PAGE_CUSTOMGT);
            break;
        case PAGE_PRESETGT:
            for (int i = 0; i < 11; ++i) addAction(kPresetGt[i], 0x1040 + i);
            break;
        case PAGE_HOST:
            addToggle("Player God Mode", 0x90B433F1 + g_target);
            addPlainAction("Rename Player", 0x1050);
            addPlainAction("Kick Player", 0x1051);
            addPlainAction("Crash Player", 0x1052);
            addPlainAction("Ban From Session", 0x1053);
            break;
        case PAGE_COLOURS:
            addAction("UI Colour (Game)", PAGE_ED_UI);
            addAction("In-Game Colour", PAGE_ED_SMOKE);
            addAction("Text Colour", PAGE_ED_TEXT);
            addAction("Menu Colour", PAGE_ED_MENU);
            addAction("Scroller Colour", PAGE_ED_SCROLL);
            addAction("ESP Colour", PAGE_ESPCOL);
            addToggle("Draw Scrollbar", 0x90B43395);
            addToggle("Draw Instructions", 0x90B433A8);
            addToggle("Draw Debug Information", 0x90B433A9);
            addSlider("UI Safe Area X", 0x90B43A78, 0.0f, 125.0f, 1.0f, FMT_0DEC);
            addSlider("UI Safe Area Y", 0x90B43A7C, 0.0f, 125.0f, 1.0f, FMT_0DEC);
            addSlider("Menu Offset X", 0x90B43A80, -125.0f, 125.0f, 1.0f, FMT_0DEC);
            addSlider("Menu Offset Y", 0x90B43A84, -125.0f, 125.0f, 1.0f, FMT_0DEC);
            break;
        case PAGE_ESPCOL:
            addAction("Enemy Colour", PAGE_ED_ENEMY);
            addAction("Hittable Colour", PAGE_ED_HITTABLE);
            addAction("Visible Colour", PAGE_ED_VISIBLE);
            addAction("Prioritized Colour", PAGE_ED_PRIO);
            addAction("Whitelisted Colour", PAGE_ED_WL);
            addAction("Friendly Colour", PAGE_ED_FRIENDLY);
            break;
        case PAGE_ED_MENU:
            buildColorEditor("Menu Colour", 0x90B43934, 0x90B438F5, 0);
            addSlider("Background Opacity", 0x90B43930, 0.0f, 1.0f, 0.003f, FMT_255);
            break;
        case PAGE_ED_SCROLL:
            buildColorEditor("Scroll Colour", 0x90B43954, 0x90B438F6, 0);
            break;
        case PAGE_ED_ENEMY:
            buildColorEditor("Enemy Colour", 0x90B439C4, 0x90B438F4, 0);
            break;
        case PAGE_ED_HITTABLE:
            buildColorEditor("Hittable Colour", 0x90B43A14, 0, 0);
            break;
        case PAGE_ED_VISIBLE:
            buildColorEditor("Visible Colour", 0x90B43A24, 0, 0);
            break;
        case PAGE_ED_PRIO:
            buildColorEditor("Prioritized Colour", 0x90B43A54, 0, 0);
            break;
        case PAGE_ED_WL:
            buildColorEditor("Whitelisted Colour", 0x90B43A44, 0, 0);
            break;
        case PAGE_ED_FRIENDLY:
            buildColorEditor("Friendly Colour", 0x90B439F4, 0, 0);
            break;
        case PAGE_ED_SMOKE:
            buildColorEditor("Smoke/Player Colour", 0x90B439D4, 0x90B438F9, 0x90B438F8);
            break;
        case PAGE_ED_UI:
            buildColorEditor("UI Colour", 0x90B439B4, 0x90B438FB, 0x90B438FA);
            addAction("Elements To Colour", PAGE_UIELEM);
            break;
        case PAGE_ED_TEXT:
            buildColorEditor("Text Colour", 0x90B43944, 0x90B438F7, 0);
            break;
        case PAGE_UIELEM:
            addToggle("Globe", 0x90B438FC);
            addToggle("Main Menu", 0x90B438FD);
            addToggle("LUI Elements", 0x90B43900);
            addToggle("HUD Elements", 0x90B43901);
            addToggle("Background", 0x90B438FE);
            addToggle("Minimap", 0x90B43902);
            break;
        case PAGE_THIRDP:
            addToggle("Show Anti-Aim Angles", 0x90B438E4);
            addSlider("Third Person Distance", 0x90B438EC, 0.0f, 400.0f, 1.0f, FMT_0DEC);
            addSlider("Third Person Height", 0x90B438F0, 0.0f, 120.0f, 1.0f, FMT_0DEC);
            addSlider("Third Person Angle", 0x90B438E8, -360.0f, 360.0f, 1.0f, FMT_0DEC);
            break;
        case PAGE_ANTIAIM:
            addToggle("Anti Aim", 0x90B43895);
            addEnum("Auto Crouch", kAutoCrouch, 4, 0x90B43898);
            addEnum("Anti Aim Yaw", kAntiYaw, 6, 0x90B438A0);
            addEnum("Anti Aim Pitch", kAntiPitch, 7, 0x90B438A4);
            addSlider("Spin Speed Yaw", 0x90B438AC, 0.0f, 40.0f, 5.0f, FMT_1DEC);
            addSlider("Spin Speed Pitch", 0x90B438A8, 0.0f, 40.0f, 1.0f, FMT_1DEC);
            break;
        case PAGE_ADVANCED:
            addSliderToggle("Position Prediction", 0x90B43BD8, 0x90B43BD4, 0.0f, 10.0f, 0.1f, FMT_1DEC);
            addSlider("Position Correction", 0x90B43A70, 0.0f, 10.0f, 0.1f, FMT_1DEC);
            addSliderToggle(*Bp(0x90B43A88) ? "Ping Prediction" : "Prediction Amount", 0x90B43A6C, 0x90B43A88, 0.0f,
                10.0f, 0.1f, FMT_1DEC);
            addSliderToggle("Backtrack (BETA)", 0x90B438C8, 0x90B438C4, 0.0f, 100.0f, 1.0f, FMT_0DEC);
            break;
        case PAGE_CUSTOMGT:
            addInfo("Custom Gamertag Editor (X To Remove, A To Add)");
            for (int i = 0; i < 10; ++i) addInfo("No Gamertag");
            addAction("Save Current Gamertag", 0x1060);
            addAction("Gamertag Looper", 0x1061);
            break;
        case PAGE_PROFILER:
            addToggle("Engine Profiler", 0x90B42A36);
            break;
        default:
            addInfo("No Options ..");
            break;
        }

        int& sel = g_sel[page < PAGE_N ? page : 0];

        if (sel >= g_optCount) sel = g_optCount ? g_optCount - 1 : 0;
        if (sel < 0) sel           = 0;

    }

    const int kTopTabs[5] = {
        PAGE_MAIN, PAGE_AIMBOT, PAGE_VISUAL, PAGE_PLAYER, PAGE_SETTINGS
    };
    const char* const kTabLabels[5] = {
        "Main Menu", "Aimbot Menu", "Visual Menu", "Player Menu", "Settings Menu"
    };
}
