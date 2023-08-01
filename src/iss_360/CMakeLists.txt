cmake_minimum_required(VERSION 3.8)
project(iss_360)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

include_directories( ${PROJECT_SOURCE_DIR}/include/iss_360/islSdk/exampleApp/sdkWrapper)
include_directories( ${PROJECT_SOURCE_DIR}/include/iss_360/islSdk/exampleApp)
include_directories( ${PROJECT_SOURCE_DIR}/include/iss_360/islSdk/islSdk)

file(STRINGS ${PROJECT_SOURCE_DIR}/include/iss_360/islSdk/exampleApp/sdkWrapper/platformSetup.h FILE_DATA REGEX "^#define OS_WINDOWS")
string(FIND "${FILE_DATA}" "#define OS_WINDOWS" matches)

if(${matches} EQUAL -1)
    set(PLATFORM_DIR unix)
    set(LIB_NAME islSdk_X64.a)
else()
    set(PLATFORM_DIR windows)
    set(LIB_NAME islSdk_X64.lib)
endif()

set(HEADERS
    include/iss_360/islSdk/exampleApp/sdkWrapper/${PLATFORM_DIR}/platformFile.h
    include/iss_360/islSdk/exampleApp/sdkWrapper/${PLATFORM_DIR}/platformNetwork.h
    include/iss_360/islSdk/exampleApp/sdkWrapper/${PLATFORM_DIR}/platformTime.h
    include/iss_360/islSdk/exampleApp/sdkWrapper/${PLATFORM_DIR}/platformUart.h
    include/iss_360/islSdk/exampleApp/sdkWrapper/ahrs.h
    include/iss_360/islSdk/exampleApp/sdkWrapper/device.h
    include/iss_360/islSdk/exampleApp/sdkWrapper/isa500.h
    include/iss_360/islSdk/exampleApp/sdkWrapper/isd4000.h
    include/iss_360/islSdk/exampleApp/sdkWrapper/ism3d.h
    include/iss_360/islSdk/exampleApp/sdkWrapper/iss360.h
    include/iss_360/islSdk/exampleApp/sdkWrapper/logger.h
    include/iss_360/islSdk/exampleApp/sdkWrapper/platformSetup.h
    include/iss_360/islSdk/exampleApp/sdkWrapper/sdkInterface.h
    include/iss_360/islSdk/exampleApp/sdkWrapper/sigSlot.h
    include/iss_360/islSdk/exampleApp/sdkWrapper/sonarImage.h
    include/iss_360/islSdk/exampleApp/sdkWrapper/sysPort.h
    include/iss_360/islSdk/exampleApp/isa500App.h
    include/iss_360/islSdk/exampleApp/isd4000App.h
    include/iss_360/islSdk/exampleApp/ism3dApp.h
    include/iss_360/islSdk/exampleApp/iss360App.h
    include/iss_360/islSdk/exampleApp/terminal.h
)

set(SOURCES
    include/iss_360/islSdk/exampleApp/sdkWrapper/${PLATFORM_DIR}/platformFile.c
    include/iss_360/islSdk/exampleApp/sdkWrapper/${PLATFORM_DIR}/platformNetwork.c
    include/iss_360/islSdk/exampleApp/sdkWrapper/${PLATFORM_DIR}/platformTime.c
    include/iss_360/islSdk/exampleApp/sdkWrapper/${PLATFORM_DIR}/platformUart.c
    include/iss_360/islSdk/exampleApp/sdkWrapper/ahrs.cpp
    include/iss_360/islSdk/exampleApp/sdkWrapper/device.cpp
    include/iss_360/islSdk/exampleApp/sdkWrapper/isa500.cpp
    include/iss_360/islSdk/exampleApp/sdkWrapper/isd4000.cpp
    include/iss_360/islSdk/exampleApp/sdkWrapper/ism3d.cpp
    include/iss_360/islSdk/exampleApp/sdkWrapper/iss360.cpp
    include/iss_360/islSdk/exampleApp/sdkWrapper/logger.cpp
    include/iss_360/islSdk/exampleApp/sdkWrapper/platformSetup.c
    include/iss_360/islSdk/exampleApp/sdkWrapper/sdkInterface.cpp
    include/iss_360/islSdk/exampleApp/sdkWrapper/sonarImage.cpp
    include/iss_360/islSdk/exampleApp/sdkWrapper/sysPort.cpp
    include/iss_360/islSdk/exampleApp/isa500App.cpp
    include/iss_360/islSdk/exampleApp/isd4000App.cpp
    include/iss_360/islSdk/exampleApp/ism3dApp.cpp
    include/iss_360/islSdk/exampleApp/iss360App.cpp
    include/iss_360/islSdk/exampleApp/terminal.cpp    
)

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(sonar_ping_msgs REQUIRED)
find_package(cv_bridge REQUIRED)
find_package(image_transport REQUIRED)
find_package(OpenCV )

add_library(lib_iss360 ${SOURCES} ${HEADERS})

# find dependencies

# uncomment the following section in order to fill in
# further dependencies manually.
# find_package(<dependency> REQUIRED)
# add_executable(iss_360 ${SOURCES} ${HEADERS})
add_executable(iss_360 src/iss_360.cpp)
target_link_libraries(iss_360 PUBLIC  lib_iss360 ${PROJECT_SOURCE_DIR}/include/iss_360/islSdk/islSdk/${LIB_NAME})
ament_target_dependencies(iss_360 PUBLIC rclcpp sensor_msgs sonar_ping_msgs cv_bridge image_transport OpenCV)


# this installs the executable in the lib folder to be seen by setup.bash to be seen by ros2 :)
install(TARGETS 
iss_360
  DESTINATION lib/${PROJECT_NAME}/
)
install(
  DIRECTORY include/iss_360
  DESTINATION include
)

install(DIRECTORY
  config
  DESTINATION lib/${PROJECT_NAME}/
)
# ament_export_targets(lib_iss360 HAS_LIBRARY_TARGET)

ament_package()
