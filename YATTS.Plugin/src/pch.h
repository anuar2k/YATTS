// pch.h: This is a precompiled header file.
// Files listed below are compiled only once, improving build performance for future builds.
// This also affects IntelliSense performance, including code completion and many code browsing features.
// However, files listed here are ALL re-compiled if any one of them is updated between builds.
// Do not add files here that you will be updating frequently as this negates the performance advantage.

#pragma once

// add headers that you want to pre-compile here

// Windows stuff.
#define WIN32_LEAN_AND_MEAN
#define WINVER 0x0500
#define _WIN32_WINNT 0x0500

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdarg.h>

// SDK
#include "scs_sdk_1_12/include/scssdk_telemetry.h"
#include "scs_sdk_1_12/include/eurotrucks2/scssdk_eut2.h"
#include "scs_sdk_1_12/include/eurotrucks2/scssdk_telemetry_eut2.h"
#include "scs_sdk_1_12/include/amtrucks/scssdk_ats.h"
#include "scs_sdk_1_12/include/amtrucks/scssdk_telemetry_ats.h"

// JSON for Modern C++
#include "nlohmann/json.hpp"
