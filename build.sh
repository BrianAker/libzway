#!/bin/bash

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

# Dependencies

# gmp
GMP_VER="6.1.0"
GMP_ARC="gmp-$GMP_VER.tar.bz2"
GMP_URL="https://gmplib.org/download/gmp/$GMP_ARC"

# nettle
NETTLE_VER="3.2"
NETTLE_ARC="nettle-$NETTLE_VER.tar.gz"
NETTLE_URL="https://ftp.gnu.org/gnu/nettle/$NETTLE_ARC"

# gnutls
GNUTLS_VER="3.4.9"
GNUTLS_ARC="gnutls-$GNUTLS_VER.tar.xz"
GNUTLS_URL="ftp://ftp.gnutls.org/gcrypt/gnutls/v3.4/$GNUTLS_ARC"

# sqlite
SQLITE_VER="3120100"
SQLITE_ARC="sqlite-autoconf-$SQLITE_VER.tar.gz"
SQLITE_URL="https://www.sqlite.org/2016/$SQLITE_ARC"

# libexif
LIBEXIF_VER="0.6.21"
LIBEXIF_ARC="libexif-$LIBEXIF_VER.tar.bz2"
LIBEXIF_URL=""

check_dependencies()
{
    # gmp
    if [ ! -f deps/$GMP_ARC ]; then
        echo "Missing source package: $GMP_ARC"
        exit -1
    fi

    # nettle
    if [ ! -f deps/$NETTLE_ARC ]; then
        echo "Missing source package $NETTLE_ARC"
        exit -1
    fi

    # gnutls
    if [ ! -f deps/$GNUTLS_ARC ]; then
        echo "Missing source package $GNUTLS_ARC"
        exit -1
    fi

    # sqlite
    if [ ! -f deps/$SQLITE_ARC ]; then
        echo "Missing source package $SQLITE_ARC"
        exit -1
    fi

    # exif
    if [ ! -f deps/$LIBEXIF_ARC ]; then
        echo "Missing source package $LIBEXIF_ARC"
        exit -1
    fi
}

# Android

ANDROID_NDK_ROOT=$2
ANDROID_API_LEVEL=$3
ANDROID_BUILD_SYSTEM=$4

android_setenv_arm7()
{
    unset SDKROOT CFLAGS CC LD CPP CXX AR AS NM CXXCPP RANLIB LDFLAGS CPPFLAGS CXXFLAGS

    if [ $ANDROID_BUILD_SYSTEM == "osx" ]; then

        export PATH="$ANDROID_NDK_ROOT/toolchains/arm-linux-androideabi-4.9/prebuilt/darwin-x86_64/bin/:$PATH"

    elif [ $ANDROID_BUILD_SYSTEM == "linux_32" ]; then

        export PATH="$ANDROID_NDK_ROOT/toolchains/arm-linux-androideabi-4.9/prebuilt/linux-x86/bin/:$PATH"

    elif [ $ANDROID_BUILD_SYSTEM == "linux_64" ]; then

        export PATH="$ANDROID_NDK_ROOT/toolchains/arm-linux-androideabi-4.9/prebuilt/linux-x86_64/bin/:$PATH"
    fi

    export SYS_ROOT="$ANDROID_NDK_ROOT/platforms/android-$ANDROID_API_LEVEL/arch-arm/"
    export CXX="arm-linux-androideabi-g++ --sysroot=$SYS_ROOT"
    export CC="arm-linux-androideabi-gcc --sysroot=$SYS_ROOT"
    export LD="arm-linux-androideabi-ld"
    export AR="arm-linux-androideabi-ar"
    export RANLIB="arm-linux-androideabi-ranlib"
    export STRIP="arm-linux-androideabi-strip"
}

# OSX and iOS

XCODE_CONTENTS_DIR=/Applications/Xcode.app/Contents

IOS_BASE_SDK=8.1
IOS_DEPLOY_TGT=7.0

ios_setenv_all()
{
    # Add internal libs
    #export CFLAGS="$CFLAGS -I$GLOBAL_OUTDIR/include -L$GLOBAL_OUTDIR/lib"

   #export CPP="$DEVROOT/usr/bin/cpp"
    export CXX="$DEVROOT/usr/bin/g++"
   #export CXXCPP="$DEVROOT/usr/bin/cpp"
    export CC="$DEVROOT/usr/bin/gcc"
    export LD=$DEVROOT/usr/bin/ld
   #export AR=$DEVROOT/usr/bin/ar
    export AS=$DEVROOT/usr/bin/as
   #export NM=$DEVROOT/usr/bin/nm
   #export RANLIB=$DEVROOT/usr/bin/ranlib
    export LDFLAGS="-L$SDKROOT/usr/lib/"

   #export CPPFLAGS=$CFLAGS
    export CXXFLAGS=$CFLAGS
}

ios_setenv_arm6()
{
    unset DEVROOT SDKROOT CFLAGS CC LD CPP CXX AR AS NM CXXCPP RANLIB LDFLAGS CPPFLAGS CXXFLAGS

    export DEVROOT=$XCODE_CONTENTS_DIR/Developer/Platforms/iPhoneOS.platform/Developer
    export SDKROOT=$DEVROOT/SDKs/iPhoneOS$IOS_BASE_SDK.sdk

    export CFLAGS="-arch armv6 -pipe -no-cpp-precomp -isysroot $SDKROOT -miphoneos-version-min=$IOS_DEPLOY_TGT -I$SDKROOT/usr/include/"

    ios_setenv_all
}

ios_setenv_arm7()
{
    unset DEVROOT SDKROOT CFLAGS CC LD CPP CXX AR AS NM CXXCPP RANLIB LDFLAGS CPPFLAGS CXXFLAGS

    export DEVROOT=$XCODE_CONTENTS_DIR/Developer/Platforms/iPhoneOS.platform/Developer
    export SDKROOT=$DEVROOT/SDKs/iPhoneOS$IOS_BASE_SDK.sdk

    export CFLAGS="-arch armv7 -pipe -no-cpp-precomp -isysroot $SDKROOT -miphoneos-version-min=$IOS_DEPLOY_TGT -I$SDKROOT/usr/include/"

    ios_setenv_all
}

ios_setenv_i386()
{
    unset DEVROOT SDKROOT CFLAGS CC LD CPP CXX AR AS NM CXXCPP RANLIB LDFLAGS CPPFLAGS CXXFLAGS

    export DEVROOT=$XCODE_CONTENTS_DIR/Developer/Platforms/iPhoneSimulator.platform/Developer
    export SDKROOT=$DEVROOT/SDKs/iPhoneSimulator$IOS_BASE_SDK.sdk

    export CFLAGS="-arch i386 -pipe -no-cpp-precomp -isysroot $SDKROOT -miphoneos-version-min=$IOS_DEPLOY_TGT"

    ios_setenv_all
}

# gmp

build_gmp()
{
    if [ ! -d gmp* ]; then
        echo "Extracting $GMP_ARC"
        tar -xf ../../deps/$GMP_ARC
    fi

    cd gmp*

    if [ ! -d build_$BUILD_TYPE ]; then

        mkdir build_$BUILD_TYPE
        cd build_$BUILD_TYPE

        echo "Configuring gmp $GMP_VER for $BUILD_TYPE"

        if   [ $BUILD_TYPE == "linux_32" ]; then

            ../configure --prefix=$INSTALL_DIR --host=i386-linux-gnu --disable-shared --with-pic > build_gmp.log 2>&1

        elif [ $BUILD_TYPE == "linux_64" ]; then

            ../configure --prefix=$INSTALL_DIR --host=x86_64-linux-gnu --disable-shared --with-pic > build_gmp.log 2>&1

        elif [ $BUILD_TYPE == "android_arm7" ]; then

            android_setenv_arm7
            ../configure --prefix=$INSTALL_DIR --host=arm-linux-androideabi --disable-shared --with-pic > build_gmp.log 2>&1

        elif [ $BUILD_TYPE == "osx" ]; then

            ../configure --prefix=$INSTALL_DIR --disable-shared --with-pic > build_gmp.log 2>&1

        elif [ $BUILD_TYPE == "ios_i386" ]; then

            ios_setenv_i386
            ../configure --prefix=$INSTALL_DIR --host=i386-apple-darwin --target=i386-apple-darwin --disable-shared --with-pic --disable-assembly > build_gmp.log 2>&1

        elif [ $BUILD_TYPE == "win_32" ]; then

            ../configure --prefix=$INSTALL_DIR --host=i686-w64-mingw32 --disable-shared --with-pic > build_gmp.log 2>&1

        elif [ $BUILD_TYPE == "win_64" ]; then

            ../configure --prefix=$INSTALL_DIR --host=x86_64-w64-mingw32 --disable-shared --with-pic > build_gmp.log 2>&1
        fi

        if [ $? != 0 ]; then
            echo "Failed to configure gmp, see build_gmp.log for details"
            cd ..
            exit -1
        fi

        cd ..
    fi

    cd build_$BUILD_TYPE

    echo "Building gmp"
    make -j2 > build_gmp.log 2>&1

    if [ $? != 0 ]; then
        echo "Failed to build gmp, see build_gmp.log for details"
        cd ../..
        exit -1
    fi

    echo "Installing gmp to $INSTALL_DIR"
    make install &> /dev/null

    cd ../..
}

# nettle

build_nettle()
{
    if [ ! -d nettle* ]; then
        echo "Extracting $NETTLE_ARC"
        tar -xf ../../deps/$NETTLE_ARC

        # need to prevent the build script from building tools and examples and stuff
        # due to the fact it produces errors when building for android arm7 on my linux machine
        # unfortunately there is no appropriate configure switch
        # so adjust Makefile.in

        cd nettle*

        #sed -n '/SUBDIRS = tools testsuite examples/!p' Makefile.in > Makefile.in.tmp
        #mv Makefile.in.tmp Makefile.in
    else

        cd nettle*

    fi

    if [ ! -d build_$BUILD_TYPE ]; then

        mkdir build_$BUILD_TYPE
        cd build_$BUILD_TYPE

        echo "Configuring nettle $NETTLE_VER for $BUILD_TYPE"

        if   [ $BUILD_TYPE == "linux_32" ]; then

                ../configure --prefix=$INSTALL_DIR --host=i386-linux-gnu --disable-shared --with-include-path=$INSTALL_DIR/include --with-lib-path=$INSTALL_DIR/lib --disable-documentation > build_nettle.log 2>&1

        elif [ $BUILD_TYPE == "linux_64" ]; then

            ../configure --prefix=$INSTALL_DIR --host=x86_64-linux-gnu --disable-shared --with-include-path=$INSTALL_DIR/include --with-lib-path=$INSTALL_DIR/lib --disable-documentation > build_nettle.log 2>&1

        elif [ $BUILD_TYPE == "android_arm7" ]; then

            android_setenv_arm7
            ../configure --prefix=$INSTALL_DIR --host=arm-linux-androideabi --disable-shared --with-include-path=$INSTALL_DIR/include --with-lib-path=$INSTALL_DIR/lib --disable-documentation > build_nettle.log 2>&1

        elif [ $BUILD_TYPE == "osx" ]; then

            ../configure --prefix=$INSTALL_DIR --disable-shared --with-include-path=$INSTALL_DIR/include --with-lib-path=$INSTALL_DIR/lib --disable-documentation > build_nettle.log 2>&1

        elif [ $BUILD_TYPE == "ios_i386" ]; then

            ios_setenv_i386
            export CC_FOR_BUILD="gcc $CFLAGS"
            ../configure --prefix=$INSTALL_DIR --host=i386-apple-darwin --disable-shared --with-include-path=$INSTALL_DIR/include --with-lib-path=$INSTALL_DIR/lib --disable-documentation > build_nettle.log 2>&1

        elif [ $BUILD_TYPE == "win_32" ]; then

            ../configure --prefix=$INSTALL_DIR --host=i686-w64-mingw32 --disable-shared --with-include-path=$INSTALL_DIR/include --with-lib-path=$INSTALL_DIR/lib --disable-documentation > build_nettle.log 2>&1

        elif [ $BUILD_TYPE == "win_64" ]; then

            ../configure --prefix=$INSTALL_DIR --host=x86_64-w64-mingw32 --disable-shared --with-include-path=$INSTALL_DIR/include --with-lib-path=$INSTALL_DIR/lib --disable-documentation > build_nettle.log 2>&1
        fi

        if [ $? != 0 ]; then
            echo "Failed to configure nettle, see build_nettle.log for details"
            cd ..
            exit -1
        fi

        cd ..
    fi

    cd build_$BUILD_TYPE

    echo "Building nettle"
    make -j2 > build_nettle.log 2>&1

    if [ $? != 0 ]; then
        echo "Failed to build nettle, see build_nettle.log for details"
        cd ../..
        exit -1
    fi

    echo "Installing nettle to $INSTALL_DIR"
    make install &> /dev/null

    cd ../..
}

# gnutls

build_gnutls()
{
    if [ ! -d gnutls* ]; then
        echo "Extracting $GNUTLS_ARC"
        tar -xf ../../deps/$GNUTLS_ARC
    fi

    cd gnutls*

    if [ ! -d build_$BUILD_TYPE ]; then

        mkdir build_$BUILD_TYPE
        cd build_$BUILD_TYPE

        export GMP_CFLAGS="-I$INSTALL_DIR/include"
        export GMP_LIBS="-L$INSTALL_DIR/lib -lgmp"
        export NETTLE_CFLAGS="-I$INSTALL_DIR/include"
        export NETTLE_LIBS="-L$INSTALL_DIR/lib -lnettle"
        export HOGWEED_CFLAGS="-I$INSTALL_DIR/include/"
        export HOGWEED_LIBS="-L$INSTALL_DIR/lib/ -lhogweed"

        echo "Configuring gnutls $GNUTLS_VER for $BUILD_TYPE"

        if   [ $BUILD_TYPE == "linux_32" ]; then

            ../configure --prefix=$INSTALL_DIR --host=i386-linux-gnu --disable-shared --disable-cxx --disable-heartbeat-support --disable-openssl-compatibility --with-pic --with-included-libtasn1 --without-p11-kit --disable-doc --disable-tests > build_gnutls.log 2>&1

        elif [ $BUILD_TYPE == "linux_64" ]; then

            ../configure --prefix=$INSTALL_DIR --host=x86_64-linux-gnu --disable-shared --disable-cxx --disable-heartbeat-support --disable-openssl-compatibility --with-pic --with-included-libtasn1 --without-p11-kit --disable-doc --disable-tests > build_gnutls.log 2>&1

        elif [ $BUILD_TYPE == "android_arm7" ]; then

            android_setenv_arm7
            ../configure --prefix=$INSTALL_DIR --host=arm-linux-androideabi --disable-shared --disable-cxx --disable-heartbeat-support --disable-openssl-compatibility --with-pic --with-included-libtasn1 --without-p11-kit --disable-doc --disable-tests > build_gnutls.log 2>&1

        elif [ $BUILD_TYPE == "osx" ]; then

            ../configure --prefix=$INSTALL_DIR --disable-shared --disable-cxx --disable-heartbeat-support --disable-openssl-compatibility --with-pic --with-included-libtasn1 --without-p11-kit --without-idn --disable-doc --disable-tests > build_gnutls.log 2>&1

        elif [ $BUILD_TYPE == "ios_i386" ]; then

            ios_setenv_i386
            ../configure --prefix=/$INSTALL_DIR --host=i386-apple-darwin --disable-shared --disable-cxx --disable-heartbeat-support --disable-openssl-compatibility --with-pic --with-included-libtasn1 --without-p11-kit --without-idn --disable-doc --disable-tests > build_gnutls.log 2>&1

        elif [ $BUILD_TYPE == "win_32" ]; then

            ../configure --prefix=$INSTALL_DIR --host=i686-w64-mingw32 --disable-shared --disable-cxx --disable-heartbeat-support --disable-openssl-compatibility --with-pic --with-included-libtasn1 --without-p11-kit --disable-doc --disable-tests > build_gnutls.log 2>&1

        elif [ $BUILD_TYPE == "win_64" ]; then

            ../configure --prefix=$INSTALL_DIR --host=x86_64-w64-mingw32 --disable-shared --disable-cxx --disable-heartbeat-support --disable-openssl-compatibility --with-pic --with-included-libtasn1 --without-p11-kit --disable-doc --disable-tests > build_gnutls.log 2>&1
        fi

        if [ $? != 0 ]; then
            echo "Failed to configure gnutls, see build_gnutls.log for details"
            cd ..
            exit -1
        fi

        cd ..
    fi

    cd build_$BUILD_TYPE

    echo "Building gnutls"
    make -j2 > build_gnutls.log 2>&1

    if [ $? != 0 ]; then
        echo "Failed to build gnutls, see build_gnutls.log for details"
        cd ../..
        exit -1
    fi

    echo "Installing gnutls to $INSTALL_DIR"
    make install &> /dev/null

    cd ../..
}

# sqlite

build_sqlite()
{
    if [ ! -d sqlite* ]; then
        echo "Extracting $SQLITE_ARC"
        tar -xf ../../deps/$SQLITE_ARC
    fi

    cd sqlite*

    if [ ! -d build_$BUILD_TYPE ]; then

        mkdir build_$BUILD_TYPE
        cd build_$BUILD_TYPE

        echo "Configuring sqlite $SQLITE_VER for $BUILD_TYPE"

        if   [ $BUILD_TYPE == "linux_32" ]; then

            ../configure --prefix=$INSTALL_DIR --host=i386-linux-gnu --disable-shared --with-pic > build_sqlite.log 2>&1

        elif [ $BUILD_TYPE == "linux_64" ]; then

            ../configure --prefix=$INSTALL_DIR --host=x86_64-linux-gnu --disable-shared --with-pic > build_sqlite.log 2>&1

        elif [ $BUILD_TYPE == "android_arm7" ]; then

            android_setenv_arm7
            ../configure --prefix=$INSTALL_DIR --host=arm-linux-androideabi --disable-shared --with-pic > build_sqlite.log 2>&1

        elif [ $BUILD_TYPE == "osx" ]; then

            ../configure --prefix=$INSTALL_DIR --disable-shared --with-pic > build_sqlite.log 2>&1

        elif [ $BUILD_TYPE == "ios_i386" ]; then

            ios_setenv_i386
            ../configure --prefix=$INSTALL_DIR --host=i386-apple-darwin --disable-shared --with-pic > build_sqlite.log 2>&1

        elif [ $BUILD_TYPE == "win_32" ]; then

            ../configure --prefix=$INSTALL_DIR --host=i686-w64-mingw32 --disable-shared --with-pic > build_sqlite.log 2>&1

        elif [ $BUILD_TYPE == "win_64" ]; then

            ../configure --prefix=$INSTALL_DIR --host=x86_64-w64-mingw32 --disable-shared --with-pic > build_sqlite.log 2>&1
        fi

        if [ $? != 0 ]; then
            echo "Failed to configure sqlite, see build_sqlite.log for details"
            cd ..
            exit -1
        fi

        cd ..
    fi

    cd build_$BUILD_TYPE

    echo "Building sqlite"
    make -j2 >> build_sqlite.log 2>&1

    if [ $? != 0 ]; then
        echo "Failed to build sqlite, see build_sqlite.log for details"
        cd ../..
        exit -1
    fi

    echo "Installing sqlite to $INSTALL_DIR"
    make install &> /dev/null

    cd ../..
}

# libexif

build_libexif()
{
    if [ ! -d libexif* ]; then
        echo "Extracting $LIBEXIF_ARC"
        tar -xf ../../deps/$LIBEXIF_ARC
    fi

    cd libexif*

    if [ ! -d build_$BUILD_TYPE ]; then

        # update some files in order to build for android
        cp /usr/share/misc/config.{sub,guess} .

        mkdir build_$BUILD_TYPE
        cd build_$BUILD_TYPE

        echo "Configuring libexif $LIBEXIF_VER for $BUILD_TYPE"

        if   [ $BUILD_TYPE == "linux_32" ]; then

            ../configure --prefix=$INSTALL_DIR --host=i386-linux-gnu --disable-shared --disable-nls --with-pic > build_libexif.log 2>&1

        elif [ $BUILD_TYPE == "linux_64" ]; then

            ../configure --prefix=$INSTALL_DIR --host=x86_64-linux-gnu --disable-shared --disable-nls --with-pic > build_libexif.log 2>&1

        elif [ $BUILD_TYPE == "android_arm7" ]; then

            android_setenv_arm7
            ../configure --prefix=$INSTALL_DIR --host=arm-linux-androideabi --disable-shared --disable-nls --with-pic > build_libexif.log 2>&1

        elif [ $BUILD_TYPE == "osx" ]; then

            ../configure --prefix=$INSTALL_DIR --disable-shared --disable-nls --with-pic > build_libexif.log 2>&1

        elif [ $BUILD_TYPE == "ios_i386" ]; then

            ios_setenv_i386
            ../configure --prefix=$INSTALL_DIR --host=i386-apple-darwin --disable-shared --disable-nls --with-pic > build_libexif.log 2>&1

        elif [ $BUILD_TYPE == "win_32" ]; then

            ../configure --prefix=$INSTALL_DIR --host=i686-w64-mingw32 --disable-shared --disable-nls --with-pic > build_libexif.log 2>&1

        elif [ $BUILD_TYPE == "win_64" ]; then

            ../configure --prefix=$INSTALL_DIR --host=x86_64-w64-mingw32 --disable-shared --disable-nls --with-pic > build_libexif.log 2>&1
        fi

        if [ $? != 0 ]; then
            echo "Failed to configure libexif, see build_libexif.log for details"
            cd ../..
            exit -1
        fi

        cd ..
    fi

    cd build_$BUILD_TYPE

    echo "Building libexif"
    make -j2 >> build_libexif.log 2>&1

    if [ $? != 0 ]; then
        echo "Failed to build libexif, see build_libexif.log for details"
        cd ../..
        exit -1
    fi

    echo "Installing libexif to $INSTALL_DIR"
    make install &> /dev/null

    cd ../..
}

# libzway

build_libzway()
{
    if [ ! -d libZwayCore ]; then
        mkdir libZwayCore
    fi

    cd libZwayCore

    if   [ $BUILD_TYPE == "linux_32" ]; then

        cmake ../..

    elif [ $BUILD_TYPE == "linux_64" ]; then

        mkdir linux_64 &> /dev/null
        cd linux_64

        cmake ../../..

        if [ $? == 0 ]; then
           make && make install
        fi

        cd ..

    elif [ $BUILD_TYPE == "android_arm7" ]; then

        mkdir android_arm7 &> /dev/null
        cd android_arm7

        cmake -DANDROID_NDK_ROOT=$ANDROID_NDK_ROOT -DANDROID_API_LEVEL=19 -DCMAKE_TOOLCHAIN_FILE=android_arm7_toolchain.cmake ../../..

        if [ $? == 0 ]; then
           make && make install
        fi

        cd ..

    elif [ $BUILD_TYPE == "osx" ]; then

        mkdir osx &> /dev/null
        cd osx

        cmake ../../..

        if [ $? == 0 ]; then
           make && make install
        fi

        cd ..

    fi

    cd ..
}

# Build

build()
{
    if [ ! -d build ]; then
        mkdir build
    fi

    cd build

    if [ ! -d deps ]; then
        mkdir deps
    fi

    cd deps

    build_gmp

    build_nettle

    build_gnutls

    build_sqlite

    build_libexif

    cd ..

    build_libzway

    cd ..
}

# Start

BUILD_TYPE=$1
INSTALL_DIR=`pwd`/build/install/$BUILD_TYPE

platform='unknown'
unamestr=`uname`
architecture=`uname -m`

if [ "$unamestr" == "Linux" ]; then

    if [ "$architecture" == "x86_64" ]; then
        echo "Build system detected: Linux 64bit"
        BUILD_TYPE="linux_64"
    fi

elif [ "$unamestr" == "Darwin" ]; then
    echo "Build system detected: $unamestr"
    BUILD_TYPE="osx"
fi

if [ "$1" == "clear-deps" ]; then

    CLEAR_TYPE=$2

    #clear_deps

    exit 0
fi

if [ $BUILD_TYPE == "" ]; then
    echo "Build type missing, possible values are:"
    echo " - linux_32"
    echo " - linux_64"
    echo " - win_32"
    echo " - win_64"
    echo " - osx"
    echo " - ios_i386"
    echo " - android_arm7"
    exit -1
fi

INSTALL_DIR=`pwd`/build/install/$BUILD_TYPE

if ( [ $BUILD_TYPE == "android_arm7" ] && [ $# -lt 2 ] ); then
    echo "NDK root missing"
    exit -1
fi

if ( [ $BUILD_TYPE == "android_arm7" ] && [ $# -lt 3 ] ); then
    echo "API level missing"
    exit -1
fi

if ( [ $BUILD_TYPE == "android_arm7" ] && [ $# -lt 4 ] ); then
    echo "Build system type missing, possible values are:"
    echo " - linux_64"
    echo " - osx"
    exit -1
fi

check_dependencies

build
