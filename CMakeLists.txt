cmake_minimum_required(VERSION 3.2)

include(../simplicity/simplicity-common.cmake)

# Project
#########################
project(simplicity-live555)

# Project Dependencies
#########################

# FFMPEG
set(AVCODEC_PATH ${CMAKE_CURRENT_SOURCE_DIR}/lib/ffmpeg/libavcodec/libavcodec.a)
set(AVUTIL_PATH ${CMAKE_CURRENT_SOURCE_DIR}/lib/ffmpeg/libavutil/libavutil.a)
set(SWRESAMPLE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/lib/ffmpeg/libswresample/libswresample.a)
set(SWSCALE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/lib/ffmpeg/libswscale/libswscale.a)
add_custom_command(
	OUTPUT ${AVCODEC_PATH} ${AVUTIL_PATH} ${SWRESAMPLE_PATH} ${SWSCALE_PATH}
	COMMAND ./configure --enable-gpl --enable-libx264
	COMMAND make
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/lib/ffmpeg
	COMMENT "Building FFmpeg")
add_custom_target(ffmpeg DEPENDS ${AVCODEC_PATH} ${AVUTIL_PATH} ${SWRESAMPLE_PATH} ${SWSCALE_PATH})

# Live555
set(BASICUSAGEENVIRONMENT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/lib/live/BasicUsageEnvironment/libBasicUsageEnvironment.a)
set(GROUPSOCK_PATH ${CMAKE_CURRENT_SOURCE_DIR}/lib/live/groupsock/libgroupsock.a)
set(LIVEMEDIA_PATH ${CMAKE_CURRENT_SOURCE_DIR}/lib/live/liveMedia/libliveMedia.a)
set(USAGEENVIRONMENT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/lib/live/UsageEnvironment/libUsageEnvironment.a)
add_custom_command(
	OUTPUT ${BASICUSAGEENVIRONMENT_PATH} ${GROUPSOCK_PATH} ${LIVEMEDIA_PATH} ${USAGEENVIRONMENT_PATH}
	COMMAND ./genMakefiles linux
	COMMAND make
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/lib/live
	COMMENT "Building Live555 Streaming Media")
add_custom_target(live DEPENDS ${BASICUSAGEENVIRONMENT_PATH} ${GROUPSOCK_PATH} ${LIVEMEDIA_PATH} ${USAGEENVIRONMENT_PATH})

# Source
#########################
file(GLOB_RECURSE SRC_FILES src/main/c++/*.cpp src/main/c++/*.h)

# Target
#########################
add_library(simplicity-live555 STATIC ${SRC_FILES})
target_include_directories(simplicity-live555 PUBLIC src/main/c++)

# Target Dependencies
#########################

# FFMPEG
add_dependencies(simplicity-live555 ffmpeg)
target_include_directories(simplicity-live555 PUBLIC lib/ffmpeg)
target_link_libraries(simplicity-live555 ${AVCODEC_PATH})
target_link_libraries(simplicity-live555 ${AVUTIL_PATH})
target_link_libraries(simplicity-live555 ${SWRESAMPLE_PATH})
target_link_libraries(simplicity-live555 ${SWSCALE_PATH})
target_link_libraries(simplicity-live555 x264)
target_link_libraries(simplicity-live555 pthread)
target_link_libraries(simplicity-live555 z)

# Live555
add_dependencies(simplicity-live555 live)
target_include_directories(simplicity-live555 PUBLIC lib/live/BasicUsageEnvironment/include)
target_include_directories(simplicity-live555 PUBLIC lib/live/groupsock/include)
target_include_directories(simplicity-live555 PUBLIC lib/live/liveMedia/include)
target_include_directories(simplicity-live555 PUBLIC lib/live/UsageEnvironment/include)
target_link_libraries(simplicity-live555 ${LIVEMEDIA_PATH})
target_link_libraries(simplicity-live555 ${GROUPSOCK_PATH})
target_link_libraries(simplicity-live555 ${BASICUSAGEENVIRONMENT_PATH})
target_link_libraries(simplicity-live555 ${USAGEENVIRONMENT_PATH})

# Simplicity
target_link_libraries(simplicity-live555 simplicity)
