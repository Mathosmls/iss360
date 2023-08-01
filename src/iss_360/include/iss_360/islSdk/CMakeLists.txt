cmake_minimum_required(VERSION 3.0.0)

project(exampleApp)

include_directories( ${PROJECT_SOURCE_DIR}/exampleApp/sdkWrapper)
include_directories( ${PROJECT_SOURCE_DIR}/islSdk)

file(STRINGS ${PROJECT_SOURCE_DIR}/exampleApp/sdkWrapper/platformSetup.h FILE_DATA REGEX "^#define OS_WINDOWS")
string(FIND "${FILE_DATA}" "#define OS_WINDOWS" matches)

if(${matches} EQUAL -1)
    set(PLATFORM_DIR unix)
    set(LIB_NAME islSdk_X64.a)
else()
    set(PLATFORM_DIR windows)
    set(LIB_NAME islSdk_X64.lib)
endif()

set(HEADERS
    exampleApp/sdkWrapper/${PLATFORM_DIR}/platformFile.h
    exampleApp/sdkWrapper/${PLATFORM_DIR}/platformNetwork.h
    exampleApp/sdkWrapper/${PLATFORM_DIR}/platformTime.h
    exampleApp/sdkWrapper/${PLATFORM_DIR}/platformUart.h
    exampleApp/sdkWrapper/ahrs.h
    exampleApp/sdkWrapper/device.h
    exampleApp/sdkWrapper/isa500.h
    exampleApp/sdkWrapper/isd4000.h
    exampleApp/sdkWrapper/ism3d.h
    exampleApp/sdkWrapper/iss360.h
    exampleApp/sdkWrapper/logger.h
    exampleApp/sdkWrapper/platformSetup.h
    exampleApp/sdkWrapper/sdkInterface.h
    exampleApp/sdkWrapper/sigSlot.h
    exampleApp/sdkWrapper/sonarImage.h
    exampleApp/sdkWrapper/sysPort.h
    exampleApp/isa500App.h
    exampleApp/isd4000App.h
    exampleApp/ism3dApp.h
    exampleApp/iss360App.h
    exampleApp/terminal.h
)

set(SOURCES
    exampleApp/sdkWrapper/${PLATFORM_DIR}/platformFile.c
    exampleApp/sdkWrapper/${PLATFORM_DIR}/platformNetwork.c
    exampleApp/sdkWrapper/${PLATFORM_DIR}/platformTime.c
    exampleApp/sdkWrapper/${PLATFORM_DIR}/platformUart.c
    exampleApp/sdkWrapper/ahrs.cpp
    exampleApp/sdkWrapper/device.cpp
    exampleApp/sdkWrapper/isa500.cpp
    exampleApp/sdkWrapper/isd4000.cpp
    exampleApp/sdkWrapper/ism3d.cpp
    exampleApp/sdkWrapper/iss360.cpp
    exampleApp/sdkWrapper/logger.cpp
    exampleApp/sdkWrapper/platformSetup.c
    exampleApp/sdkWrapper/sdkInterface.cpp
    exampleApp/sdkWrapper/sonarImage.cpp
    exampleApp/sdkWrapper/sysPort.cpp
    exampleApp/isa500App.cpp
    exampleApp/isd4000App.cpp
    exampleApp/ism3dApp.cpp
    exampleApp/iss360App.cpp
    exampleApp/terminal.cpp
    exampleApp/main.cpp
)

add_executable (exampleApp ${SOURCES} ${HEADERS})
target_link_libraries(exampleApp ${PROJECT_SOURCE_DIR}/islSdk/${LIB_NAME})
