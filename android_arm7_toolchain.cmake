
## ============================================================ ##
##
##   d88888D db   d8b   db  .d8b.  db    db
##   YP  d8' 88   I8I   88 d8' `8b `8b  d8'
##      d8'  88   I8I   88 88ooo88  `8bd8'
##     d8'   Y8   I8I   88 88~~~88    88
##    d8' db `8b d8'8b d8' 88   88    88
##   d88888P  `8b8' `8d8'  YP   YP    YP
##
##   open-source, cross-platform, crypto-messenger
##
##   Copyright (C) 2016 Marc Weiler
##
##   This library is free software; you can redistribute it and/or
##   modify it under the terms of the GNU Lesser General Public
##   License as published by the Free Software Foundation; either
##   version 2.1 of the License, or (at your option) any later version.
##
##   This library is distributed in the hope that it will be useful,
##   but WITHOUT ANY WARRANTY; without even the implied warranty of
##   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
##   Lesser General Public License for more details.
##
## ============================================================ ##

set(CMAKE_SYSTEM_NAME Android)

set(CMAKE_SYSTEM_VERSION 1)

# set android ndk sysroot

# unfortunately no variables are evaluated in the following statement, why???
set(CMAKE_SYSROOT "${ANDROID_NDK_ROOT}/platforms/android-${ANDROID_API_LEVEL}/arch-arm")

# while the following works fine!!!
message(STATUS "CMAKE_SYSROOT=${ANDROID_NDK_ROOT}/platforms/android-${ANDROID_API_LEVEL}/arch-arm")

# hardcode it :(
set(CMAKE_SYSROOT "/home/marc/Android/ndk/platforms/android-19/arch-arm")

# set android ndk compilers (and yes, it works even here)

set(CMAKE_C_COMPILER "${ANDROID_NDK_ROOT}/toolchains/arm-linux-androideabi-4.9/prebuilt/linux-x86_64/bin/arm-linux-androideabi-gcc")

set(CMAKE_CXX_COMPILER "${ANDROID_NDK_ROOT}/toolchains/arm-linux-androideabi-4.9/prebuilt/linux-x86_64/bin/arm-linux-androideabi-g++")

# provide compiler flags for CMakeLists.txt

set(ANDROID_INCLUDES "-I${ANDROID_NDK_ROOT}/sources/cxx-stl/gnu-libstdc++/4.9/include -I${ANDROID_NDK_ROOT}/sources/cxx-stl/gnu-libstdc++/4.9/libs/armeabi-v7a/include")

set(ANDROID_CXX_FLAGS "-Wno-psabi -march=armv7-a -mfloat-abi=softfp -mfpu=vfp -ffunction-sections -funwind-tables -fstack-protector -fno-short-enums -DANDROID -Wa,--noexecstack -fno-builtin-memmove")
