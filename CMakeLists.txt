cmake_minimum_required(VERSION 3.25)
project(nalchi VERSION 0.0.1)

set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
set(CMAKE_CXX_EXTENSIONS FALSE)

option(BUILD_SHARED_LIBS "Build nalchi and deps as shared library" FALSE)
if(NOT USE_STEAMWORKS)
    set(USE_CRYPTO "OpenSSL" CACHE STRING "Crypto library to use for AES/SHA256 for stand-alone GameNetworkingSockets")
    option(USE_STEAMWEBRTC "Build WebRTC to get ICE support for P2P for stand-alone GameNetworkingSockets" FALSE)
endif()

# Set GameNetworkingSockets options
if(BUILD_SHARED_LIBS)
    set(BUILD_STATIC_LIB FALSE)
    set(BUILD_SHARED_LIB TRUE)
else()
    set(BUILD_STATIC_LIB TRUE)
    set(BUILD_SHARED_LIB FALSE)
endif()

# nalchi options
option(NALCHI_BUILD_TESTS "Build nalchi tests" FALSE)

# nalchi target
add_library(nalchi)
target_include_directories(nalchi
PRIVATE
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
    "$<INSTALL_INTERFACE:$<INSTALL_PREFIX>/${CMAKE_INSTALL_INCLUDEDIR}>"
    src
)
target_include_directories(nalchi SYSTEM
INTERFACE
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
    "$<INSTALL_INTERFACE:$<INSTALL_PREFIX>/${CMAKE_INSTALL_INCLUDEDIR}>"
)
if(BUILD_SHARED_LIBS)
    target_compile_definitions(nalchi PRIVATE NALCHI_BUILD_EXPORT)
else()
    target_compile_definitions(nalchi PUBLIC NALCHI_BUILD_STATIC)
endif()

# Compiler warnings
set(nalchi_compile_options
    $<$<CXX_COMPILER_ID:MSVC>:/W4 /Zc:preprocessor /Zc:__cplusplus>
    $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>,$<CXX_COMPILER_ID:GNU>>:
        -Wall -Wextra -Wpedantic>
)
target_compile_options(nalchi PRIVATE ${nalchi_compile_options})

# nalchi public headers
target_sources(nalchi
    PUBLIC
    FILE_SET nalchi_headers
    BASE_DIRS include
    TYPE HEADERS
    FILES
        include/nalchi/export.hpp
        include/nalchi/type_traits.hpp
        include/nalchi/concepts.hpp
        include/nalchi/server.hpp
        include/nalchi/shared_payload.hpp
        include/nalchi/shared_payload_flat.hpp
        include/nalchi/bit_stream.hpp
)

# nalchi sources
target_sources(nalchi PRIVATE
    src/server.cpp
    src/shared_payload.cpp
    src/shared_payload_flat.cpp
    src/bit_stream.cpp
)

# Steamworks SDK or stand-alone GameNetworkingSockets?
if(USE_STEAMWORKS)
    set(STEAMWORKS_REDIST_BIN_DIR ${CMAKE_CURRENT_SOURCE_DIR}/sdk/redistributable_bin)
    set(STEAMWORKS_PUBLIC_LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/sdk/public/steam/lib)

    add_library(steam_api SHARED IMPORTED SYSTEM)
    add_library(sdkencryptedappticket SHARED IMPORTED SYSTEM)

    target_include_directories(steam_api INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/sdk/public)
    target_include_directories(sdkencryptedappticket INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/sdk/public)

    target_link_libraries(nalchi PUBLIC steam_api sdkencryptedappticket)

    if(WIN32)
        if(CMAKE_SIZEOF_VOID_P EQUAL 8)
            set(STEAM_API_IMPORTED_LOCATION ${STEAMWORKS_REDIST_BIN_DIR}/win64/steam_api64.dll)
            set(STEAM_API_IMPORTED_IMPLIB ${STEAMWORKS_REDIST_BIN_DIR}/win64/steam_api64.lib)
            set(LIBSDKENCRYPTEDAPPTICKET_IMPORTED_LOCATION ${STEAMWORKS_PUBLIC_LIB_DIR}/win64/sdkencryptedappticket64.dll)
            set(LIBSDKENCRYPTEDAPPTICKET_IMPORTED_IMPLIB ${STEAMWORKS_PUBLIC_LIB_DIR}/win64/sdkencryptedappticket64.lib)
        else()
            set(STEAM_API_IMPORTED_LOCATION ${STEAMWORKS_REDIST_BIN_DIR}/steam_api.dll)
            set(STEAM_API_IMPORTED_IMPLIB ${STEAMWORKS_REDIST_BIN_DIR}/steam_api.lib)
            set(LIBSDKENCRYPTEDAPPTICKET_IMPORTED_LOCATION ${STEAMWORKS_PUBLIC_LIB_DIR}/win32/sdkencryptedappticket.dll)
            set(LIBSDKENCRYPTEDAPPTICKET_IMPORTED_IMPLIB ${STEAMWORKS_PUBLIC_LIB_DIR}/win32/sdkencryptedappticket.lib)
        endif()
    elseif(APPLE)
        set(STEAM_API_IMPORTED_LOCATION ${STEAMWORKS_REDIST_BIN_DIR}/osx/libsteam_api.dylib)
        set(LIBSDKENCRYPTEDAPPTICKET_IMPORTED_LOCATION ${STEAMWORKS_PUBLIC_LIB_DIR}/osx/libsdkencryptedappticket.dylib)
    elseif(LINUX)
        if(CMAKE_SIZEOF_VOID_P EQUAL 8)
            set(STEAM_API_IMPORTED_LOCATION ${STEAMWORKS_REDIST_BIN_DIR}/linux64/libsteam_api.so)
            set(LIBSDKENCRYPTEDAPPTICKET_IMPORTED_LOCATION ${STEAMWORKS_PUBLIC_LIB_DIR}/linux64/libsdkencryptedappticket.so)
        else()
            set(STEAM_API_IMPORTED_LOCATION ${STEAMWORKS_REDIST_BIN_DIR}/linux32/libsteam_api.so)
            set(LIBSDKENCRYPTEDAPPTICKET_IMPORTED_LOCATION ${STEAMWORKS_PUBLIC_LIB_DIR}/linux32/libsdkencryptedappticket.so)
        endif()
    else()
        message(FATAL_ERROR "Invalid operating system")
    endif()

    if(WIN32)
        set_target_properties(steam_api PROPERTIES
            IMPORTED_LOCATION ${STEAM_API_IMPORTED_LOCATION}
            IMPORTED_IMPLIB ${STEAM_API_IMPORTED_IMPLIB}
        )
        set_target_properties(sdkencryptedappticket PROPERTIES
            IMPORTED_LOCATION ${LIBSDKENCRYPTEDAPPTICKET_IMPORTED_LOCATION}
            IMPORTED_IMPLIB ${LIBSDKENCRYPTEDAPPTICKET_IMPORTED_IMPLIB}
        )
    else()
        set_target_properties(steam_api PROPERTIES
            IMPORTED_LOCATION ${STEAM_API_IMPORTED_LOCATION}
        )
        set_target_properties(sdkencryptedappticket PROPERTIES
            IMPORTED_LOCATION ${LIBSDKENCRYPTEDAPPTICKET_IMPORTED_LOCATION}
        )
    endif()

else() # Open source GameNetworkingSockets
    add_subdirectory(GameNetworkingSockets SYSTEM)

    if (BUILD_SHARED_LIBS)
        target_link_libraries(nalchi PUBLIC GameNetworkingSockets)
    else()
        target_link_libraries(nalchi PUBLIC GameNetworkingSockets_s)
    endif()
endif()

# tests
if(NALCHI_BUILD_TESTS)
    add_subdirectory(tests)
endif()

# Copy dependencies
if(WIN32 AND BUILD_SHARED_LIBS)
    add_custom_command(TARGET nalchi POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<$<BOOL:${BUILD_SHARED_LIBS}>:$<TARGET_RUNTIME_DLLS:nalchi>>
        $<TARGET_FILE_DIR:nalchi>
        COMMAND_EXPAND_LISTS
    )
endif()
if(USE_STEAMWORKS)
    add_custom_command(TARGET nalchi POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${STEAM_API_IMPORTED_LOCATION}
        ${LIBSDKENCRYPTEDAPPTICKET_IMPORTED_LOCATION}
        $<TARGET_FILE_DIR:nalchi>
    )
    if(WIN32)
        add_custom_command(TARGET nalchi POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${STEAM_API_IMPORTED_IMPLIB}
            ${LIBSDKENCRYPTEDAPPTICKET_IMPORTED_IMPLIB}
            $<TARGET_FILE_DIR:nalchi>
        )
    endif()
endif()


# Install binaries
install(TARGETS nalchi
    EXPORT nalchiTargets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    FILE_SET nalchi_headers
)
install(EXPORT nalchiTargets
    FILE nalchiTargets.cmake
    NAMESPACE nalchi::
    DESTINATION lib/cmake/nalchi
)

if(BUILD_SHARED_LIBS AND WIN32)
    install(FILES $<TARGET_RUNTIME_DLLS:nalchi> TYPE BIN)
    if(MSVC)
        install(FILES $<TARGET_PDB_FILE:nalchi> TYPE BIN OPTIONAL)
    endif()
endif()
if(USE_STEAMWORKS)
    if(WIN32)
        install(FILES ${STEAM_API_IMPORTED_LOCATION} TYPE BIN)
        install(FILES ${LIBSDKENCRYPTEDAPPTICKET_IMPORTED_LOCATION} TYPE BIN)
        install(FILES ${STEAM_API_IMPORTED_IMPLIB} TYPE LIB)
        install(FILES ${LIBSDKENCRYPTEDAPPTICKET_IMPORTED_IMPLIB} TYPE LIB)
    else()
        install(FILES ${STEAM_API_IMPORTED_LOCATION} TYPE LIB)
        install(FILES ${LIBSDKENCRYPTEDAPPTICKET_IMPORTED_LOCATION} TYPE LIB)
    endif()
endif()

# Install headers
if(USE_STEAMWORKS)
    file(GLOB STEAM_API_HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/sdk/public/steam/*.h)
    install(FILES ${STEAM_API_HEADERS} DESTINATION include/SteamworksSDK/steam)
endif()

# Install licenses
install(FILES LICENSE DESTINATION .)
install(FILES LICENSE DESTINATION licenses RENAME nalchi.LICENSE.txt)
if(NOT USE_STEAMWORKS)
    set(GNS_EXTERNAL_SRCS_DIR GameNetworkingSockets/src/external)
    install(FILES GameNetworkingSockets/LICENSE DESTINATION licenses RENAME GameNetworkingSockets.LICENSE.txt)
    install(FILES downloaded-licenses/protobuf.LICENSE.txt DESTINATION licenses)
    if(USE_STEAMWEBRTC)
        install(FILES ${GNS_EXTERNAL_SRCS_DIR}/abseil/LICENSE DESTINATION licenses RENAME Abseil.LICENSE.txt OPTIONAL)
        install(FILES ${GNS_EXTERNAL_SRCS_DIR}/webrtc/LICENSE DESTINATION licenses RENAME WebRTC.LICENSE.txt OPTIONAL)
    endif()
    if(USE_CRYPTO STREQUAL "BCrypt")
        install(FILES downloaded-licenses/bcrypt.LICENSE.txt DESTINATION licenses)
        install(FILES ${GNS_EXTERNAL_SRCS_DIR}/ed25519-donna/readme_VALVE.txt DESTINATION licenses RENAME ed25519-donna.README.txt)
    elseif(USE_CRYPTO STREQUAL "OpenSSL")
        install(FILES downloaded-licenses/OpenSSL.LICENSE.txt DESTINATION licenses)
    elseif(USE_CRYPTO STREQUAL "libsodium")
        install(FILES downloaded-licenses/libsodium.LICENSE.txt DESTINATION licenses)
        install(FILES ${GNS_EXTERNAL_SRCS_DIR}/sha1-wpa/README DESTINATION licenses RENAME sha1-wpa.LICENSE.txt)
    endif()
endif()
