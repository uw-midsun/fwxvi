#!/usr/bin/env bash
set -e

add_line_if_dne () {
  if ! grep -q "$1" "$2" 2>/dev/null; then
    echo "$1" >> "$2"
  fi
}

if [[ "$OSTYPE" == "linux-gnu"* ]]; then
    echo "==> Detected Linux (Ubuntu/WSL assumed)"
    apt-get update

    echo "==> Install common tools"
    apt-get -y install tmux git vim curl

    echo "==> Install python tooling"
    apt-get -y install python3-pip virtualenv python3-autopep8 pylint
    #python3 -m pip install --upgrade --ignore-installed pip

    echo "==> Install Python requirements"
    if python3 -m pip install -h | grep -q -- '--break-system-packages'; then
        python3 -m pip install -r requirements.txt --break-system-packages
    else
        PYTHONBREAKSYSTEMPACKAGES=1 python3 -m pip install -r requirements.txt
    fi

    echo "==> Install tooling for CAN"
    apt-get -y install can-utils

    echo "==> Install nlohmann-json"
    apt-get install -y nlohmann-json3-dev

        echo "==> Install clang and gcc"
    apt-get -y install binutils-arm-none-eabi libncurses-dev software-properties-common
    add-apt-repository -y ppa:ubuntu-toolchain-r/test
    apt-get -y install gcc-11 g++-11 clang clang-format gdb

    ln -sf "$(which gcc-11)" /usr/bin/gcc

    # Dynamically find the latest installed clang and clang-format
    CLANG_BIN=$(ls /usr/bin/clang-[0-9]* 2>/dev/null | sort -V | tail -n 1)
    CLANG_FORMAT_BIN=$(ls /usr/bin/clang-format-[0-9]* 2>/dev/null | sort -V | tail -n 1)

    if [ -n "$CLANG_BIN" ]; then
      ln -sf "$CLANG_BIN" /usr/bin/clang
      ln -sf "${CLANG_BIN/clang/clang++}" /usr/bin/clang++
    fi

    if [ -n "$CLANG_FORMAT_BIN" ]; then
      ln -sf "$CLANG_FORMAT_BIN" /usr/bin/clang-format
    fi

    if [ $? -ne 0 ]; then
      echo "==> Install arm gcc 11.3"
      wget https://developer.arm.com/-/media/Files/downloads/gnu/11.3.rel1/binrel/arm-gnu-toolchain-11.3.rel1-x86_64-arm-none-eabi.tar.xz -O arm-gcc.tar.xz
      tar xf arm-gcc.tar.xz -C /usr/local
      add_line_if_dne 'PATH=$PATH:/usr/local/arm-gnu-toolchain-11.3.rel1-x86_64-arm-none-eabi/bin' /etc/profile
      rm arm-gcc.tar.xz
    fi

    echo "==> Install other toolchain pieces"
    apt-get -y install minicom openocd

    echo "==> Setup for minicom"
    mkdir -p /etc/minicom
    touch /etc/minicom/minirc.dfl
    add_line_if_dne "pu addcarreturn    Yes" /etc/minicom/minirc.dfl

elif [[ "$OSTYPE" == "darwin"* ]]; then
    echo "==> Detected macOS"
    echo "Installing packages with Homebrew (brew)..."

    # Check if brew exists
    if ! command -v brew >/dev/null 2>&1; then
        echo "Homebrew not found. Please install it from https://brew.sh/"
        exit 1
    fi

    brew update
    brew install tmux git vim curl scons can-utils gcc llvm gdb minicom openocd python3

    echo "==> Install Python requirements"
    pip3 install --upgrade pip
    pip3 install -r requirements.txt

else
    echo "Unsupported OS: $OSTYPE"
    exit 1
fi