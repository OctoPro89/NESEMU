set echo on

cFilenames=$(find . -type f -name "*.c")

echo "Files:" $cFilenames

assembly="nesemu"
compilerFlags="-g"
includeFlags="-Isrc -I"src/" -I"lib/glad/include""
linkerFlags="-lGL -lxcb -lX11 -lX11-xcb -lxkbcommon -L/user/X11R6/lib"
defines="-D_DEBUG"

echo "Building nesemu..."
clang $cFilenames $compilerFlags -o bin/$assembly $defines $includeFlags $linkerFlags