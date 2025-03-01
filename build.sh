#/usr/bin/bash

set -xe

mkdir -p build

echo "Building x86-64 version"
gcc -O3 -Wall -Wextra wol-bot.c -o build/wol-bot-x86_64  

echo "Building arm64 version"
aarch64-linux-gnu-gcc -O3 -Wall -Wextra -static -o build/wol-bot-arm64 wol-bot.c

echo "Done!"