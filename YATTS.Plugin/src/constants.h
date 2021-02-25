#pragma once
#define YATTS_MAKE_VERSION_HELPER(major, minor, patch) #major "." #minor "." #patch
#define YATTS_MAKE_VERSION(major, minor, patch) YATTS_MAKE_VERSION_HELPER(major, minor, patch)

#define YATTS_VERSION_MAJOR 0
#define YATTS_VERSION_MINOR 1
#define YATTS_VERSION_PATCH 0

#define YATTS_VERSION YATTS_MAKE_VERSION(YATTS_VERSION_MAJOR, YATTS_VERSION_MINOR, YATTS_VERSION_PATCH)

//path is relative to the game's executable
#define CONFIG_PATH "plugins\\YATTS.Config.json"
