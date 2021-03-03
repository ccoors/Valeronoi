# Valeronoi

Valeronoi (Valetudo + Voronoi) is a companion for [Valetudo](https://valetudo.cloud) for generating WiFi signal strength maps. It visualizes them using a [Voronoi diagram](https://en.wikipedia.org/wiki/Voronoi_diagram).

![Screenshot](assets/screenshot.png)

## Installation

Binary distributions are available for Linux and Windows (64-bit). They can be found in the [releases](https://github.com/ccoors/Valeronoi/releases). Binary releases for Linux are [AppImages](https://appimage.org/), the binary releases for Windows are installers. Please note that there are two AppImages, "legacy" and "current". Try "current" first, if that does not work, use "legacy". The "current" AppImage is built on Ubuntu 20.04 LTS (Focal) and Qt 6, so it probably won't run on anything older than that (specifically glibc 2.29). The "legacy" AppImage is built on Ubuntu 18.04 LTS (Bionic) and Qt 5. The Windows release _should_ only run on Windows 10.

Builds for macOS are currently not provided, as I have no means to create them.

## Basic Usage

1. Make sure you have a supported robot running a recent Valetudo version (API v2, Valetudo 2020.02.0 recommended). Having persistent maps is highly recommended.
2. Set up the robot connection in Valeronoi (Robot -> Setup)
3. Connect Valeronoi to the robot using the "Connect" button
4. Verify that the correct map is displayed
5. Start recording WiFi measurements by clicking the "Begin recording" button
6. Start a cleanup, either using the controls in the "Control" tab or Valetudo/Home Assistant/... If you don't have persistent maps, make sure to not start a full cleanup! Major map updates can not be handled in Valeronoi.
7. While cleaning, watch the map update
8. After the robot returned to the dock, stop the recording and/or disconnect Valeronoi

## Support

### Graphics

If the graphics are slow and unresponsive, try the following things in the "Display" tab:

- Disable drawing the floor
- Disabling the "Restrict to ..." checkboxes
- Increase the "Simplify" slider
- Enabling drawing using OpenGL

**Warning:** Using OpenGL may increase or decrease performance, depending on your system. Enabling OpenGL can also lead to various issues, like garbage text or inverted graphics. Valeronoi requests OpenGL 3.2, which should be available on most systems, but may not be available in VMs or Remote Desktop scenarios.

## Building from source

Install the required Libraries using the method of your choice:

- Qt 5.15/6
- CGAL (4.x/5.x should work)

You also need a C++-Compiler capable of C++17. g++ 7 or later works.

Then use the CMake-Project as you would in any other project.

```
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j
```

### Arch Linux

```
# pacman -S base-devel cgal qt6-base qt6-imageformats qt6-svg
```

Alternatively for Qt 5 (not recommended):

```
# pacman -S base-devel cgal qt5-base qt5-imageformats qt5-svg qt5-x11extras
```

### Building the binary distributions

**You probably don't need this.**
Generating binary distributions for projects using Qt is messy. That is why Valeronoi uses various Docker images known to work to build the Linux AppImage and the Windows Installer. You can find the required scripts and Dockerfiles in `tools/`.

## License

GPLv3
