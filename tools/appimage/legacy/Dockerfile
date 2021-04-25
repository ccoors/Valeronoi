FROM ubuntu:bionic

ENV VALERONOI_QT_BASE_VERSION=5.15 VALERONOI_QT_VERSION=5.15.2 VALERONOI_CMAKE_BASE_VERSION=3.20 VALERONOI_CMAKE_VERSION=3.20.1

RUN apt update && apt --yes dist-upgrade
RUN DEBIAN_FRONTEND="noninteractive" apt install --yes build-essential g++ git cmake wget file fuse ninja-build openssl ca-certificates libssl-dev libgl1-mesa-dev libfontconfig1-dev libfreetype6-dev libx11-dev libx11-xcb-dev libxext-dev libxfixes-dev libxi-dev libxrender-dev libxcb1-dev libxcb-glx0-dev libxcb-keysyms1-dev libxcb-image0-dev libxcb-shm0-dev libxcb-icccm4-dev libxcb-sync-dev libxcb-xfixes0-dev libxcb-shape0-dev libxcb-randr0-dev libxcb-render-util0-dev libxcb-util-dev libxcb-xinerama0-dev libxcb-xkb-dev libxkbcommon-dev libxkbcommon-x11-dev libcgal-dev python3

RUN TMPDIR=$(mktemp -d) \
        && cd $TMPDIR \
        && wget https://cmake.org/files/v${VALERONOI_CMAKE_BASE_VERSION}/cmake-${VALERONOI_CMAKE_VERSION}-linux-x86_64.sh \
        && chmod +x cmake-${VALERONOI_CMAKE_VERSION}-linux-x86_64.sh \
        && ./cmake-${VALERONOI_CMAKE_VERSION}-linux-x86_64.sh --skip-license --prefix=/usr \
        && cd \
        && rm -rf $TMPDIR

RUN TMPDIR=$(mktemp -d) \
        && cd $TMPDIR \
        && wget https://download.qt.io/official_releases/qt/${VALERONOI_QT_BASE_VERSION}/${VALERONOI_QT_VERSION}/single/qt-everywhere-src-${VALERONOI_QT_VERSION}.tar.xz \
        && tar -xf qt-everywhere-src-${VALERONOI_QT_VERSION}.tar.xz \
        && cd qt-everywhere-src-${VALERONOI_QT_VERSION} \
        && ./configure -confirm-license -opensource -v -prefix /usr -docdir /usr/share/doc/qt -headerdir /usr/include/qt -archdatadir /usr/lib/qt -datadir /usr/share/qt -sysconfdir /etc/xdg -examplesdir /usr/share/doc/qt/examples -nomake examples -no-rpath -no-mimetype-database -no-use-gold-linker -reduce-relocations -opengl desktop -openssl-runtime -release \
        && make -j \
        && make install \
        && cd \
        && rm -rf $TMPDIR

RUN mkdir /tools \
        && cd /tools \
        && wget https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-x86_64.AppImage \
        && wget https://github.com/linuxdeploy/linuxdeploy-plugin-qt/releases/download/continuous/linuxdeploy-plugin-qt-x86_64.AppImage \
        && chmod a+x *
