#!/usr/bin/env bash
set -euxo pipefail

add_line_if_dne () {
  if ! grep -q "$1" "$2" 2>/dev/null; then
    echo "$1" >> "$2"
  fi
}

echo "==> Linux container setup"

apt-get update

echo "==> Install common tools"
apt-get -y install tmux git vim curl

echo "==> Install tooling for CAN"
apt-get -y install can-utils

echo "==> Install nlohmann-json"
apt-get -y install nlohmann-json3-dev

echo "==> Install ARM + compiler toolchain"
apt-get -y install software-properties-common
add-apt-repository -y ppa:ubuntu-toolchain-r/test
apt-get update


apt-get -y install \
  binutils-arm-none-eabi \
  gcc-arm-none-eabi \
  gcc-11 g++-11 \
  clang clang-format gdb \
  libncurses-dev libsdl2-dev

ln -sf /usr/bin/gcc-11 /usr/bin/gcc || true

echo "==> Install other tools"
apt-get -y install minicom openocd

echo "==> Setup minicom"
mkdir -p /etc/minicom
touch /etc/minicom/minirc.dfl
add_line_if_dne "pu addcarreturn    Yes" /etc/minicom/minirc.dfl
