#pragma once

#include <steam/steamnetworkingtypes.h>

#ifdef STEAMNETWORKINGSOCKETS_OPENSOURCE
#include <steam/steamnetworkingsockets.h>
#else // Steamworks SDK
#include <steam/steam_api.h>
#endif

#include <stdlib.h>
#if defined(_WIN32)
#define NALCHI_TESTS_PUTENV _putenv
#else
#define NALCHI_TESTS_PUTENV putenv
#endif

namespace nalchi::tests
{

inline bool gns_init()
{
#ifdef STEAMNETWORKINGSOCKETS_OPENSOURCE
    SteamNetworkingErrMsg err_msg;
    return GameNetworkingSockets_Init(nullptr, err_msg);
#else // Steamworks SDK
    char app_id[] = "SteamAppId=480";
    char game_id[] = "SteamGameId=480";
    NALCHI_TESTS_PUTENV(app_id);
    NALCHI_TESTS_PUTENV(game_id);

    return SteamAPI_Init();
#endif
}

inline void gns_kill()
{
#ifdef STEAMNETWORKINGSOCKETS_OPENSOURCE
    GameNetworkingSockets_Kill();
#else // Steamworks SDK
    SteamAPI_Shutdown();
#endif
}

} // namespace nalchi::tests
