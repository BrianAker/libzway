
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

project(libzway)

cmake_minimum_required(VERSION 2.6)

set(CMAKE_VERBOSE_MAKEFILE ON)

if (DEFINED ANDROID_CXX_FLAGS)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${ANDROID_CXX_FLAGS} -std=c++11 ${ANDROID_INCLUDES}")

set(INSTALL_TARGET android_arm7)

message(STATUS "Building for Android arm-v7a")

elseif(CMAKE_HOST_APPLE)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(INSTALL_TARGET osx)

elseif(CMAKE_HOST_UNIX)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(INSTALL_TARGET linux_64)

endif()

include_directories(
    ${PROJECT_SOURCE_DIR}/build/install/${INSTALL_TARGET}/include
    include
)

link_directories(
    ${PROJECT_SOURCE_DIR}/build/install/${INSTALL_TARGET}/lib
)

set(libzway_SRCS
    src/ubj/ubjr.c 
    src/ubj/ubjw.c 
    src/ubj/value.cpp 
    src/event/event.cpp 
    src/event/eventdispatcher.cpp 
    src/crypto/crypto.cpp 
    src/crypto/random.cpp 
    src/crypto/secmem.cpp 
    src/crypto/digest.cpp 
    src/crypto/aes.cpp 
    src/crypto/rsa.cpp 
    src/message/message.cpp 
    src/message/resource.cpp 
    src/message/messageevent.cpp 
    src/message/messagereceiver.cpp 
    src/message/messagesender.cpp 
    src/request/addcontactrequest.cpp 
    src/request/configrequest.cpp 
    src/request/acceptcontactrequest.cpp 
    src/request/contactstatusrequest.cpp 
    src/request/createaccountrequest.cpp 
    src/request/dispatchrequest.cpp 
    src/request/rejectcontactrequest.cpp 
    src/request/findcontactrequest.cpp 
    src/request/inboxrequest.cpp 
    src/request/loginrequest.cpp 
    src/request/messagerequest.cpp 
    src/request/request.cpp 
    src/request/requestevent.cpp 
    src/storage/node.cpp 
    src/storage/storage.cpp 
    src/buffer.cpp 
    src/client.cpp 
    src/util/exif.cpp 
    src/packet.cpp 
    src/thread.cpp
)

if (DEFINED ANDROID_CXX_FLAGS)

link_directories(${ANDROID_NDK_ROOT}/sources/cxx-stl/gnu-libstdc++/4.9/libs/armeabi-v7a)

set (libzway_LIBS
    gnustl_shared
    gnutls
    hogweed
    nettle
    gmp
    sqlite3
    exif
    z
)  

add_library (ZwayCore SHARED ${libzway_SRCS})

else()

set (libzway_LIBS
    gnutls
    hogweed
    nettle
    gmp
    sqlite3
    exif
)  

add_library (ZwayCore STATIC ${libzway_SRCS})

endif()

target_link_libraries(ZwayCore ${libzway_LIBS})

#set_target_properties(MyLibStatic PROPERTIES OUTPUT_NAME MyLib)

install (TARGETS ZwayCore
         ARCHIVE DESTINATION ${PROJECT_SOURCE_DIR}/build/install/${INSTALL_TARGET}/lib
         LIBRARY DESTINATION ${PROJECT_SOURCE_DIR}/build/install/${INSTALL_TARGET}/lib
         RUNTIME DESTINATION ${PROJECT_SOURCE_DIR}/build/install/${INSTALL_TARGET}/lib)

#add_executable (clienttest test.cpp)

#target_link_libraries(clienttest
#        zway
#)

#install(TARGETS server
#	RUNTIME DESTINATION bin
#	LIBRARY DESTINATION lib
#)
