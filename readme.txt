Edit and run in Qt Creator (Linux):
* Open CMakeLists.txt using File > Open File or Project.
  Use this instead of the old .creator files, which contain machine-specific settings.
* Select your Desktop kit (GCC), choose Debug, and click Configure Project.
* Edit src/main.cpp. init() sets up the scene; render() draws each frame.
* Build with Ctrl+B, run with Ctrl+R, and press Escape to close the OpenGL window.
* CMake uses system OpenGL and GLEW. If GLFW is missing, it downloads and builds
  GLFW 3.4 automatically inside the build directory. The first configure needs internet.
* Assets from data/ are copied next to the executable on each build.
* When adding a .cpp file, add it to add_executable() in CMakeLists.txt.

Build with CMake from a terminal:
  cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug
  cmake --build build
  cd build
  ./opengl

Linux dependencies:
* Arch Linux: sudo pacman -S --needed base-devel cmake ninja glew glfw
* Installing glfw is optional for CMake. Building GLFW from source also needs
  the X11 and Wayland development libraries (see https://www.glfw.org/docs/3.4/compile.html).
* Qt Creator can edit this plain C++ project; no Qt libraries are required.
* Reference: https://doc.qt.io/qtcreator/creator-project-opening.html

Build on Linux:
* Run make (uses the CMake setup above, including the automatic GLFW download).
* The executable is also copied to ./opengl for existing Qt Creator Make projects.
* Run make run to build and launch, or make clean to clean the build outputs.

Build on Windows (Visual Studio 2019):
* GLEW and GLFW libraries are installed in the /include and /lib directories so no need to download anything.
* Just build the VS solution.

Build on Windows (mingw32):
* GLEW and GLFW libraries are installed in the /include and /lib-mingw32 directories so no need to download anything.
* Run mingw32-make.
