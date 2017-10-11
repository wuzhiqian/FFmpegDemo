cmake_minimum_required(VERSION 3.4.1)

file(GLOB source src/main/cpp/*.cpp src/main/cpp/*.c)
set(distribution_DIR ../../../libs)

add_library(native-lib
             SHARED
             ${source} )


add_library( ffmpeg
             SHARED
             IMPORTED)
set_target_properties( ffmpeg
                       PROPERTIES IMPORTED_LOCATION
                       ${distribution_DIR}/armeabi/libffmpeg.so)

include_directories(libs/include)
target_link_libraries( native-lib
                       log
                       ffmpeg)