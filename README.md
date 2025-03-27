# nalchi

<img align="right" src="https://raw.githubusercontent.com/nalchi-net/nalchi/refs/heads/main/doc/nalchi.png" width="100px">

<img src="https://github.com/nalchi-net/nalchi/actions/workflows/cd-pc.yml/badge.svg"> <img src="https://github.com/nalchi-net/nalchi/actions/workflows/deploy-docs.yml/badge.svg">

[nalchi](https://github.com/nalchi-net/nalchi) aims to provide utilities for efficient message sending over [ValveSoftware/GameNetworkingSockets](https://github.com/ValveSoftware/GameNetworkingSockets).

## Features

* Efficient [multicast](https://nalchi-net.github.io/nalchi/classnalchi_1_1socket__extensions.html) support with reference counted [`nalchi::shared_payload`](https://nalchi-net.github.io/nalchi/structnalchi_1_1shared__payload.html).
* Bit-level serialization support with [`nalchi::bit_stream_writer`](https://nalchi-net.github.io/nalchi/classnalchi_1_1bit__stream__writer.html) & [`nalchi::bit_stream_reader`](https://nalchi-net.github.io/nalchi/classnalchi_1_1bit__stream__reader.html)

See <https://nalchi-net.github.io/nalchi/> for the full API reference.

# Build

nalchi supports either building with the stand-alone version of GameNetworkingSockets or Steamworks SDK version of it.

## Build with stand-alone GameNetworkingSockets

If you don't need the Steamworks SDK integration because you're building your game for platforms other than Steam,<br>
you can build the [stand-alone GameNetworkingSockets](https://github.com/ValveSoftware/GameNetworkingSockets) which is licensed under the [BSD 3-Clause "New" or "Revised" License](https://github.com/ValveSoftware/GameNetworkingSockets/blob/master/LICENSE).

1. Recursively clone this repo to get [GameNetworkingSockets/](GameNetworkingSockets/) submodule.
1. Prepare the dependencies of the GameNetworkingSockets.
    * Refer to the [`BUILDING.md`](https://github.com/ValveSoftware/GameNetworkingSockets/blob/master/BUILDING.md) on GameNetworkingSockets.
        * If you're using *Developer Powershell for VS 2022* on Windows, do note that it defaults to x86 environment, which obviously doesn't work when building for the AMD64.<br>
          You need to switch to AMD64 environment manually with following:
          ```powershell
          Enter-VsDevShell -DevCmdArguments "-arch=x64 -host_arch=x64" -VsInstallPath "C:/Program Files/Microsoft Visual Studio/2022/Community" -SkipAutomaticLocation
          ```
1. Run `cmake --preset nalchi-gns` to configure.
    * If you're on Windows, you need to specify the vcpkg toolchain file location with `CMAKE_TOOLCHAIN_FILE` variable.<br>
      You could create a seperate `CMakeUserPresets.json` which inherits the `nalchi-msvc-gns` preset to do this. e.g:
      ```json
      {
          "version": 6,
          "configurePresets": [
              {
                  "name": "my-preset",
                  "inherits": "nalchi-msvc-gns",
                  "cacheVariables": {
                      "CMAKE_TOOLCHAIN_FILE": "C:/vcpkg/scripts/buildsystems/vcpkg.cmake"
                  }
              }
          ]
      }
      ```
      And then you can run `cmake --preset my-preset` instead.
1. Depending on the preset, different build directory must have been created.<br>
   Run `cmake --build <build directory>` to build.

## Build with Steamworks SDK

If you need to integrate with Steamworks SDK to make use of other Steamworks API functionality, you can build with Steamworks SDK.

1. Download the Steamworks SDK from the [Steamworks partner site](https://partner.steamgames.com/).
1. Unzip it, and copy the `public/` and `redistributable_bin/` into the [sdk/](sdk/) directory.
1. Run `cmake --preset nalchi-steamworks` to configure.
    * If you're on Windows, you might want to run `cmake --preset nalchi-msvc-steamworks` instead.
    * If you're using *Developer Powershell for VS 2022* on Windows, do note that it defaults to x86 environment, which obviously doesn't work when building for the AMD64.<br>
      You need to switch to AMD64 environment manually with following:
      ```powershell
      Enter-VsDevShell -DevCmdArguments "-arch=x64 -host_arch=x64" -VsInstallPath "C:/Program Files/Microsoft Visual Studio/2022/Community" -SkipAutomaticLocation
      ```
1. Depending on the preset, different build directory must have been created.<br>
   Run `cmake --build <build directory>` to build.

# License

nalchi is licensed under the [MIT License](https://github.com/nalchi-net/nalchi/blob/main/LICENSE).

This project depends on either the stand-alone GameNetworkingSockets or the Steamworks SDK.
* [Stand-alone GameNetworkingSockets](https://github.com/ValveSoftware/GameNetworkingSockets) is licensed under the [BSD 3-Clause "New" or "Revised" License](https://github.com/ValveSoftware/GameNetworkingSockets/blob/master/LICENSE).
    * Refer to the GameNetworkingSockets' GitHub repo for other dependencies' licenses.
* Steamworks SDK is licensed under the [STEAMWORKS SDK license](https://partner.steamgames.com/documentation/sdk_access_agreement).
