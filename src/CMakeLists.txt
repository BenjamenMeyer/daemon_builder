CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
PROJECT(daemon C CXX)

INCLUDE_DIRECTORIES(../include)

SET(COMMON_SOURCES daemon_common.cpp)

SET(APP_SOURCES daemon_app.cpp)

SET(CONTROLLER_SOURCES daemon_controller.cpp)

ADD_LIBRARY(daemon_common STATIC ${COMMON_SOURCES})

ADD_EXECUTABLE(daemon_app ${APP_SOURCES})
TARGET_LINK_LIBRARIES(daemon_app daemon_common)

ADD_EXECUTABLE(daemon_controller ${CONTROLLER_SOURCES})
TARGET_LINK_LIBRARIES(daemon_controller daemon_common)
