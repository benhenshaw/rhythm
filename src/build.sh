# Build script for rhythm
# Benedict Henshaw, 2018
# Uncomment the line for your platform.

# Common flags.
FLAGS="main.c -o rhythm -Wall -O2"

# macOS (clang)
clang $FLAGS -framework SDL2

# windows (MinGW)
# gcc $FLAGS -mwindows -lmingw32 -lSDL2main -lSDL2

# Run on successful build.
if [[ $? -eq 0 ]]; then
    ./rhythm
fi

# Build the sound converter
cc sound_converter.c -o af32c -Wall -Wextra -O2
