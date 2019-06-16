
cmake_minimum_required(VERSION 2.8.12)

project(Bolt5_Client)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

find_package(SoapySDR "0.4.0" NO_MODULE REQUIRED)
if (NOT SoapySDR_FOUND)
    message(FATAL_ERROR "Soapy SDR development files not found...")
endif()


set(BOLT_5_SOURCES
    main.cpp
    SoapyEnum.cpp
    SDRDevInfo.cpp
    client.cpp
    utils.cpp
	zlib/adler32.c
	zlib/compress.c
	zlib/crc32.c
	zlib/deflate.c
	zlib/gzclose.c
	zlib/gzlib.c
	zlib/gzread.c
	zlib/gzwrite.c
	zlib/infback.c
	zlib/inffast.c
	zlib/inflate.c
	zlib/inftrees.c
	zlib/trees.c
	zlib/uncompr.c
	zlib/zutil.c
)

set(BOLT_5_HEADERS
    SoapyEnum.h
    SDRDevInfo.h
    client.h
    utils.h
    fdacoefs.h
	zlib/crc32.h
	zlib/deflate.h
	zlib/gzguts.h
	zlib/inffast.h
	zlib/inffixed.h
	zlib/inflate.h
	zlib/inftrees.h
	zlib/trees.h
	zlib/zconf.h
	zlib/zlib.h
	zlib/zutil.h

)

add_executable(${PROJECT_NAME} ${BOLT_5_SOURCES} ${BOLT_5_HEADERS})

target_link_libraries(${PROJECT_NAME} SoapySDR)
target_link_libraries(${PROJECT_NAME} pthread)

