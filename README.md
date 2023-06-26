# TinCan.NET (WIP)
A work-in-progress GUI input plugin for N64 emulators.

## Architecture
The process split is inevitable—there's no clean way to have two GUI event loops running in the same process. Thus, 
this plugin uses ZeroMQ as a middleman to send requests to and from the GUI.

## Building instructions
You'll need .NET 6, CMake (minimum version 3.24), and vcpkg (Windows only). Be sure to set the environment variable 
`VCPKG_ROOT` if you're using vcpkg; this is required for the CMake preset to work.

- Build `TinCan.NET.Native` using the provided presets, and `TinCan.NET` as you would any other .NET project.
- Publish `TinCan.NET` as a single-file executable, and place it in the same folder as `tincan-bridge.dll`.
- Use it as a plugin in your favourite frontend and pray it works.