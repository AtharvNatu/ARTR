clear

SOURCE_PATH=Source
BIN_DIR=Bin

echo "--------------------------------------------------------------------------------"
echo "Creating Directory Layout ..."
echo "--------------------------------------------------------------------------------"
mkdir $BIN_DIR

rm -rf "$BIN_DIR/Window.app"
mkdir -p "$BIN_DIR/Window.app/Contents/MacOS"
mkdir -p "$BIN_DIR/Window.app/Contents/Resources"
cp Assets/* "$BIN_DIR/Window.app/Contents/Resources"

echo "--------------------------------------------------------------------------------"
echo "Compiling Cocoa Source Code And Linking Frameworks ..."
echo "--------------------------------------------------------------------------------"
clang \
    -Wno-deprecated-declarations \
    -arch arm64 \
    -o "$BIN_DIR/Window.app/Contents/MacOS/Window" \
    "$SOURCE_PATH/Window.m" \
    -framework Foundation \
    -framework Cocoa
    
if [ $? -ne 0 ]; then
    echo "Compilation Failed !!!"
    exit 1
else 
    echo "Done ..."
fi



