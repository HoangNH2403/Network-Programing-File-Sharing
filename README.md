<kbd> MiniDrive</kbd>

## Structure
``` bash
.
├── CMakeLists.txt
├── app
│   └── main.cpp  # Application source code.    
├── include
│   ├── example.h # Library header file.
├── src
    └── example.cpp # Library source code.

```

Sources go in [src/](src/), header files in [include/](include/), main programs in [app/](app).

If you add a new executable, say `app/hello.cpp`, you only need to add the following two lines to [CMakeLists.txt](CMakeLists.txt):

```cmake
add_executable(main app/main.cpp)   # Name of exec. and location of file.
target_link_libraries(main PRIVATE ${LIBRARY_NAME})  # Link the executable to lib built from src/*.cpp (if it uses it).
```

## Building

Build by making a build directory (i.e. `build/`), run `cmake` in that dir, and then use `make` to build the desired target.

Example:

```bash
mkdir build && cd build
cmake ..
make
./main

```

## Setup
### Dependencies
+ [CMake](https://cmake.org/)
+ C++ Compiler: `g++`, `clang` or `msvc`
+ `zip`, `unzip`, `fd`


On Ubuntu:
```
sudo apt-get install build-essential cmake fd-find zip unzip libssl-dev
ln -s $(which fdfind) ~/.local/bin/fd
```
### Visual Studio Code
Install the following extensions:
- [C/C++](https://marketplace.visualstudio.com/items?itemName=ms-vscode.cpptools)
- [CMake Tools](https://marketplace.visualstudio.com/items?itemName=ms-vscode.cmake-tools)