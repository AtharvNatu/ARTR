#!/bin/bash

SOURCE_PATH=Source
BIN_DIR=Bin

clear

mkdir -p "$BIN_DIR"

rm -f "$BIN_DIR"/*.o 2>/dev/null


echo "--------------------------------------------------------------------------------"
echo "Compiling X11 Source Code ..."
echo "--------------------------------------------------------------------------------"
g++ -c -o XWindow.o "$SOURCE_PATH/XWindow.c"

if [ $? -ne 0 ]; then
    echo "Compilation Failed !!!"
    exit 1
fi

mv *.o "$BIN_DIR" 2>/dev/null

echo
echo "--------------------------------------------------------------------------------"
echo "Linking Object Files and Creating Binary ..."
echo "--------------------------------------------------------------------------------"

g++ "$BIN_DIR"/XWindow.o -o "$BIN_DIR"/XWindow -lX11

if [ $? -ne 0 ]; then
    echo "Linking Failed !!!"
    exit 1
fi

echo
echo "--------------------------------------------------------------------------------"
echo "Launching Application ..."
echo "--------------------------------------------------------------------------------"
cd "$BIN_DIR"
./XWindow


