#ifndef LUDA_FEATURES_AIM_AUTOBONE_HPP
#define LUDA_FEATURES_AIM_AUTOBONE_HPP

namespace autobone {
    typedef unsigned char u8;

    struct Vec3 {
        float x;
        float y;
        float z;
    };

    typedef bool (*ResolveDirect)(void* context, u8 selector, Vec3* position);
    typedef bool (*TrySelector)(void* context, u8 selector, Vec3* position, float* bestDamage);

    struct VirtualPair {
        u8 first;
        u8 second;
        float weight;
    };

    static bool VirtualPairForSelector(u8 selector, VirtualPair* pair) {

        if (!pair) return false;

        switch (selector) {
            case 0x33:
                pair->first = 0x27;
                pair->second = 0x25;
                pair->weight = 2.6700000762939453125f;
                return true;
            case 0x34:
                pair->first = 0x28;
                pair->second = 0x26;
                pair->weight = 2.099999904632568359375f;
                return true;
            case 0x35:
                pair->first = 0x21;
                pair->second = 0x1F;
                pair->weight = 0.875f;
                return true;
            case 0x36:
                pair->first = 0x22;
                pair->second = 0x20;
                pair->weight = 0.875f;
                return true;
            case 0x37:
                pair->first = 0x22;
                pair->second = 0x26;
                pair->weight = -0.5f;
                return true;
            case 0x38:
                pair->first = 0x21;
                pair->second = 0x25;
                pair->weight = -0.5f;
                return true;
            case 0x39:
                pair->first = 0x20;
                pair->second = 0x1F;
                pair->weight = 1.125f;
                return true;
            case 0x3A:
                pair->first = 0x20;
                pair->second = 0x1F;
                pair->weight = 0.75f;
                return true;
            case 0x3B:
                pair->first = 0x20;
                pair->second = 0x1F;
                pair->weight = 0.9375f;
                return true;
        }

        return false;

    }

    static bool ResolvePosition(void* context, u8 selector, ResolveDirect resolveDirect, Vec3* position) {

        if (!resolveDirect || !position || selector >= 0x3C) return false;
        if (selector < 0x33) return resolveDirect(context, selector, position);

        VirtualPair pair;
        if (!VirtualPairForSelector(selector, &pair)) return false;

        Vec3 first;
        Vec3 second;
        if (!resolveDirect(context, pair.first, &first)) return false;
        if (!resolveDirect(context, pair.second, &second)) return false;

        const float scale = 0.5f * pair.weight;
        position->x = (first.x - second.x) * scale + second.x;
        position->y = (first.y - second.y) * scale + second.y;
        position->z = (first.z - second.z) * scale + second.z;

        return true;

    }

    struct SelectInput {
        u8 configuredSelector;
        bool alwaysCheckHead;
        bool priorityClient;
        bool shieldWeapon;
        bool paused;
    };

    struct SelectState {
        u8 rollingSelector;
    };

    struct SelectResult {
        u8 selector;
        Vec3 position;
        float bestDamage;
    };

    static bool Try(void* context, TrySelector trySelector, u8 selector, float* bestDamage, SelectResult* result) {

        Vec3 position;
        if (!trySelector(context, selector, &position, bestDamage)) return false;
        result->selector = selector;
        result->position = position;
        result->bestDamage = *bestDamage;

        return true;

    }

    static bool SelectPriority(const SelectInput& input, void* context, TrySelector trySelector, float* bestDamage,
                               SelectResult* result) {

        if (Try(context, trySelector, input.configuredSelector, bestDamage, result)) return true;

        bool found = false;

        if (input.shieldWeapon) {
            bool foundSelectorOne = false;
            if (input.configuredSelector != 1 && Try(context, trySelector, 1, bestDamage, result)) {
                found = true;
                foundSelectorOne = true;
                if (*bestDamage >= 25.0f) return true;
            }

            const int lowerBound = foundSelectorOne ? 0 : 0x16;
            for (int selector = 0x39; selector > lowerBound; --selector) {
                if (input.paused) return false;
                if (selector == input.configuredSelector || selector == 1) continue;
                if (!Try(context, trySelector, (u8)selector, bestDamage, result)) continue;
                found = true;
                if (*bestDamage >= 25.0f) return true;
            }
            return found;
        }

        for (int selector = 0; selector < 0x3C; ++selector) {
            if (input.paused) return false;
            if (selector == input.configuredSelector) continue;
            if (!Try(context, trySelector, (u8)selector, bestDamage, result)) continue;
            found = true;
            if (*bestDamage >= 25.0f) return true;
        }

        return found;

    }

    static bool Select(const SelectInput& input, SelectState* state, void* context, TrySelector trySelector,
                       float initialBestDamage, SelectResult* result) {

        if (!state || !trySelector || !result || input.paused) return false;

        float bestDamage = initialBestDamage;

        if (input.priorityClient) return SelectPriority(input, context, trySelector, &bestDamage, result);
        if (input.alwaysCheckHead && Try(context, trySelector, 2, &bestDamage, result)) return true;
        if (Try(context, trySelector, input.configuredSelector, &bestDamage, result)) return true;

        const u8 preferred = input.shieldWeapon ? 0x26 : 3;

        if (preferred != input.configuredSelector && Try(context, trySelector, preferred, &bestDamage, result))
            return true;

        if (state->rollingSelector == input.configuredSelector) {
            const u8 old = state->rollingSelector;
            state->rollingSelector = (u8)(old + 1);
            if (old >= 0x3C) state->rollingSelector = 0;
        }
        if (Try(context, trySelector, state->rollingSelector, &bestDamage, result)) return true;

        const u8 old = state->rollingSelector;
        state->rollingSelector = (u8)(old + 1);
        if (old >= 0x3C) state->rollingSelector = 0;

        return Try(context, trySelector, state->rollingSelector, &bestDamage, result);

    }
}

#endif
