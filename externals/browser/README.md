# VTS Browser CPP - Build Wrapper

[VTS Browser CPP](https://github.com/melowntech/vts-browser-cpp) is a collection of libraries that bring VTS client capabilities to your native applications.

[This build wrapper](https://github.com/melowntech/vts-browser-cpp-build-wrapper) allows easy building of 3rd-party dependencies required for the browser.

It provides configured git submodules and cmake build scripts for all 3rd-party libraries and for the browser.

The primary intention of this repository was to allow building of the browser on Microsoft Windows.
However, it is possible to build all the libraries on all platforms supported by the browser to minimize number of external runtime dependencies, which is extremely useful for embedding.

## Cloning

Go to the directory where you have cloned the repository.

Make sure that all submodules are cloned too:
```bash
git pull
git submodule update --init --recursive
```

If you get an error that file names are too long, use this setting:
```bash
git config --system core.longpaths true
```

Always clone the repository - do NOT download the repository from github, the downloaded archive does not contain the submodules.

## Building on Windows

### Permission for symlinks

On Windows, a specific permissions are required to allow creating symbolic file links.
Go to Settings -> Update & Security -> For developers and ennable Developer mode.

Moreover, it is necessary to enable creating symlinks when installing the git.

### Compiler, Git & CMake

A C++14 capable compiler is required.
We recommend MS Visual Studio 2017 or newer.
It is freely available at: https://www.visualstudio.com/downloads/
Community edition is suitable.

The Visual Studio also contains git and cmake integrations.
However, we recommend standalone versions available here:
https://gitforwindows.org/ and https://cmake.org/download/

### Python

Python is available at: https://www.python.org/downloads/windows/

Also, during the installation, select to add the Python to environment variable PATH.

### Building for Windows desktop

Configure and build.
```bash
mkdir build
cd build
cmake -Ax64 ..
cmake --build . --config relwithdebinfo
```

The _-Ax64_ selects 64 bit architecture.

You may skip the last line and use the visual studio solution as usual instead.

### Building for UWP (Universal Windows Platform)

Configure and build.
```bash
mkdir build-uwp
cd build-uwp
cmake -Ax64 -DCMAKE_TOOLCHAIN_FILE=../externals/browser/browser/cmake/uwp.toolchain.cmake ..
cmake --build . --config relwithdebinfo
```

## Building for Linux desktop

Install some prerequisites.

```bash
sudo apt update
sudo apt install \
    cmake \
    nasm \
    libssl-dev \
    python-minimal
```

Configure and build.
```bash
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=RELWITHDEBINFO ..
cmake --build . -- -j5
```

## Building for web assembly

We assume that you are building on linux machine.

Prepare Emscripten build tools as described here:
https://emscripten.org/docs/getting_started/downloads.html

Configure and build.
```bash
mkdir build-wasm
cd build-wasm
cmake -DCMAKE_BUILD_TYPE=RELEASE -DCMAKE_TOOLCHAIN_FILE=../externals/browser/browser/cmake/wasm.toolchain.cmake ..
cmake --build . -- -j5
```

Use emrun to test the build.

## Building for Mac

Configure and build.
```bash
mkdir build
cd build
cmake -GXcode ..
```

Use the generated XCode project as usual.

## Building for iOS

Configure and build.
```bash
mkdir build-ios
cd build-ios
cmake -GXcode -DCMAKE_TOOLCHAIN_FILE=../externals/browser/browser/cmake/ios.toolchain.cmake ..
```

Use the generated XCode project as usual.

## Recommended folder structure

For standalone VTS build:
```
vts-browser-cpp-build-wrapper
├── build
│   ├── vts-browser-build-wrapper.sln -- here is your project for Windows
│   ├── Makefile -- here is your makefile for Linux
│   └── vts-browser-build-wrapper.xcodeproj -- here is your project for Mac
├── build-wasm
│   └── Makefile -- here is your makefile for web assembly build
├── build-uwp
│   └── vts-browser-build-wrapper.sln -- here is your solution for UWP
├── build-ios
│   └── vts-browser-build-wrapper.xcodeproj -- here is your project for iOS
├── externals
│   └── browser -- if this folder is empty, you forgot to clone the submodules (run 'git submodule update --init --recursive' to initialize the submodules now)
│       ├── browser
│       │   └── cmake
│       │       ├── wasm.toolchain.cmake -- this is where CMAKE_TOOLCHAIN_FILE points to when building for web assembly
│       │       ├── uwp.toolchain.cmake -- this is where CMAKE_TOOLCHAIN_FILE points to when building for UWP
│       │       └── ios.toolchain.cmake -- this is where CMAKE_TOOLCHAIN_FILE points to when building for iOS
|       ├── BUILDING.md -- build instructions for linux desktop only (with system dependencies)
│       ├── LICENSE
│       └── README.md
├── LICENSE
└── README.md -- the document you are reading just now
```

For your own application with cmake and vts as submodule:
```
your-awesome-application
├── build
├── externals
│   └── vts-browser-cpp-build-wrapper -- the submodule
└── CMakeLists.txt
```

## Bug reports

For bug reports on the build wrapper:
[Issue tracker](https://github.com/melowntech/vts-browser-cpp-build-wrapper/issues).

For bug reports on the libraries:
[Issue tracker](https://github.com/melowntech/vts-browser-cpp/issues).

## How to contribute

Check the [CONTRIBUTING.md](https://github.com/melowntech/vts-browser-cpp/blob/master/CONTRIBUTING.md).

## License

See the [LICENSE](LICENSE) file.



