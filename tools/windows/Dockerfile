FROM archlinux

# Install packages
RUN pacman -Syu --noconfirm \
        && pacman -S --noconfirm base base-devel sudo git python go pacman-contrib \
        && echo "%wheel ALL=(ALL) NOPASSWD: ALL" >> /etc/sudoers \
        && useradd -m -G wheel -s /usr/bin/bash build \
        && paccache -r -k 0

RUN sudo -u build sh -c "YAY=$(mktemp -u -d) && git clone https://aur.archlinux.org/yay.git \$YAY && cd \$YAY && makepkg -si --noconfirm && cd / && rm -rf \$YAY"

# Install mingw
RUN sudo -u build sh -c "yay -S --noconfirm mingw-w64-toolchain mingw-w64" \
        && paccache -r -k 0 \
        && rm -rf ~/.cache/yay

# Install mingw qt6, cgal, nsis
RUN echo "[multilib]" >> /etc/pacman.conf \
        && echo "Include = /etc/pacman.d/mirrorlist" >> /etc/pacman.conf \
        && echo "MAKEFLAGS=\"-j$(nproc --ignore 4)\"" >> /etc/makepkg.conf \
        && pacman -Syyu --noconfirm \
        && sudo -u build sh -c "yay -S --pgpfetch --mflags '--skipinteg' --noconfirm wine mingw-w64-cairo-bootstrap mingw-w64-freetype2-bootstrap mingw-w64-qt6-base mingw-w64-qt6-5compat mingw-w64-qt6-tools mingw-w64-cgal mingw-w64-qt6-svg nsis" \
        && paccache -r -k 0 \
        && rm -rf ~/.cache/yay
