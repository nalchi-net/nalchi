cmake_minimum_required(VERSION 3.25)
project(nalchi VERSION 0.0.1)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
set(CMAKE_CXX_EXTENSIONS FALSE)

add_library(nalchi SHARED)
target_include_directories(nalchi PUBLIC include src)

# Sources
target_sources(nalchi PRIVATE
    src/server.cpp
)

# Public headers
target_sources(nalchi
    PUBLIC
    FILE_SET public_headers
    TYPE HEADERS
    FILES
        include/nalchi/server.hpp
)

if(USE_STEAMWORKS)
    set(STEAMWORKS_REDIST_BIN_DIR ${CMAKE_CURRENT_SOURCE_DIR}/sdk/redistributable_bin)
    set(STEAMWORKS_PUBLIC_LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/sdk/public/steam/lib)

    add_library(steam_api SHARED IMPORTED SYSTEM)
    add_library(sdkencryptedappticket SHARED IMPORTED SYSTEM)

    target_include_directories(steam_api INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/sdk/public)
    target_include_directories(sdkencryptedappticket INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/sdk/public)

    target_link_libraries(nalchi PRIVATE steam_api sdkencryptedappticket)

    if(WIN32)
        if(CMAKE_SIZEOF_VOID_P EQUAL 8)
            set_target_properties(steam_api PROPERTIES
                IMPORTED_LOCATION ${STEAMWORKS_REDIST_BIN_DIR}/win64/steam_api64.dll
                IMPORTED_IMPLIB ${STEAMWORKS_REDIST_BIN_DIR}/win64/steam_api64.lib
            )
            set_target_properties(sdkencryptedappticket PROPERTIES
                IMPORTED_LOCATION ${STEAMWORKS_PUBLIC_LIB_DIR}/win64/sdkencryptedappticket64.dll
                IMPORTED_IMPLIB ${STEAMWORKS_PUBLIC_LIB_DIR}/win64/sdkencryptedappticket64.lib
            )
        else()
            set_target_properties(steam_api PROPERTIES
                IMPORTED_LOCATION ${STEAMWORKS_REDIST_BIN_DIR}/steam_api.dll
                IMPORTED_IMPLIB ${STEAMWORKS_REDIST_BIN_DIR}/steam_api.lib
            )
            set_target_properties(sdkencryptedappticket PROPERTIES
                IMPORTED_LOCATION ${STEAMWORKS_PUBLIC_LIB_DIR}/win32/sdkencryptedappticket.dll
                IMPORTED_IMPLIB ${STEAMWORKS_PUBLIC_LIB_DIR}/win32/sdkencryptedappticket.lib
            )
        endif()
    elseif(APPLE)
        set_target_properties(steam_api PROPERTIES
            IMPORTED_LOCATION ${STEAMWORKS_REDIST_BIN_DIR}/osx/libsteam_api.dylib
        )
        set_target_properties(sdkencryptedappticket PROPERTIES
            IMPORTED_LOCATION ${STEAMWORKS_PUBLIC_LIB_DIR}/osx/libsdkencryptedappticket.dylib
        )
    elseif(LINUX)
        if(CMAKE_SIZEOF_VOID_P EQUAL 8)
            set_target_properties(steam_api PROPERTIES
                IMPORTED_LOCATION ${STEAMWORKS_REDIST_BIN_DIR}/linux64/libsteam_api.so
            )
            set_target_properties(sdkencryptedappticket PROPERTIES
                IMPORTED_LOCATION ${STEAMWORKS_PUBLIC_LIB_DIR}/linux64/libsdkencryptedappticket.so
            )
        else()
            set_target_properties(steam_api PROPERTIES
                IMPORTED_LOCATION ${STEAMWORKS_REDIST_BIN_DIR}/linux32/libsteam_api.so
            )
            set_target_properties(sdkencryptedappticket PROPERTIES
                IMPORTED_LOCATION ${STEAMWORKS_PUBLIC_LIB_DIR}/linux32/libsdkencryptedappticket.so
            )
        endif()
    else()
        message(FATAL_ERROR "Invalid operating system")
    endif()
else() # Open source GameNetworkingSockets
    add_subdirectory(GameNetworkingSockets EXCLUDE_FROM_ALL SYSTEM)

    target_link_libraries(nalchi PRIVATE GameNetworkingSockets::shared)
endif()

if(WIN32)
    add_custom_command(TARGET nalchi POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_RUNTIME_DLLS:nalchi>
        $<TARGET_FILE_DIR:nalchi>
        COMMAND_EXPAND_LISTS
    )
endif()

# Install targets
install(TARGETS nalchi FILE_SET public_headers DESTINATION .)
if(WIN32)
    install(FILES $<TARGET_RUNTIME_DLLS:nalchi> TYPE BIN)
endif()

# Install licenses
install(FILES LICENSE DESTINATION .)
install(FILES LICENSE DESTINATION licenses RENAME nalchi.LICENSE.txt)
if(NOT USE_STEAMWORKS)
    set(GNS_EXTERNAL_SRCS_DIR GameNetworkingSockets/src/external)
    install(FILES GameNetworkingSockets/LICENSE DESTINATION licenses RENAME GameNetworkingSockets.LICENSE.txt)
    install(FILES ${GNS_EXTERNAL_SRCS_DIR}/abseil/LICENSE DESTINATION licenses RENAME Abseil.LICENSE.txt OPTIONAL)
    install(FILES ${GNS_EXTERNAL_SRCS_DIR}/ed25519-donna/readme_VALVE.txt DESTINATION licenses RENAME ed25519-donna.README.txt)
    install(FILES ${GNS_EXTERNAL_SRCS_DIR}/picojson/LICENSE DESTINATION licenses RENAME PicoJSON.LICENSE.txt OPTIONAL)
    install(FILES ${GNS_EXTERNAL_SRCS_DIR}/sha1-wpa/README DESTINATION licenses RENAME sha1-wpa.LICENSE.txt)
    install(FILES ${GNS_EXTERNAL_SRCS_DIR}/webrtc/LICENSE  DESTINATION licenses RENAME WebRTC.LICENSE.txt OPTIONAL)
endif()
