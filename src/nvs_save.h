#ifndef NVS_SAVE_H
#define NVS_SAVE_H

#include <Preferences.h>
#include "globals.h"

namespace NVS {

    static void save() {
        Preferences prefs;
        prefs.begin("retro-console", false);

        prefs.putString("name",   g_app.playerName);
        prefs.putUChar("bright",  g_app.brightness);
        prefs.putBool("sound",    g_app.soundOn);

        for (int i = 0; i < 6; i++) {
            char key[4];
            snprintf(key, sizeof(key), "hs%u", i);
            prefs.putULong(key, g_app.highScores[i]);
        }

        prefs.end();
    }

    static void load() {
        Preferences prefs;
        prefs.begin("retro-console", true);

        String name = prefs.getString("name", "Player");
        strlcpy(g_app.playerName, name.c_str(), sizeof(g_app.playerName));

        g_app.brightness = prefs.getUChar("bright", 75);
        g_app.soundOn     = prefs.getBool("sound",   true);

        for (int i = 0; i < 6; i++) {
            char key[4];
            snprintf(key, sizeof(key), "hs%u", i);
            g_app.highScores[i] = prefs.getULong(key, 0);
        }

        prefs.end();
    }

}

#endif
