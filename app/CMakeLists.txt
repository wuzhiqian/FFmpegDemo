cmake_minimum_required(VERSION 3.4.1)

file(GLOB source src/main/cpp/*.cpp src/main/cpp/*.c  src/main/cpp/puller/*.cpp src/main/cpp/puller/*.c)
set(distribution_DIR  ../../../../libs)
add_library(native-lib
             SHARED
             ${source} )


add_library( ffmpeg
             SHARED
             IMPORTED)
set_target_properties( ffmpeg
                       PROPERTIES IMPORTED_LOCATION
                       ${distribution_DIR}/armeabi/libffmpeg.so)


add_library( faac
             SHARED
             IMPORTED)
set_target_properties( faac
                       PROPERTIES IMPORTED_LOCATION
                       ${distribution_DIR}/armeabi/libfaac.so)


#add_library( rtmp
#             SHARED
#             IMPORTED)
#set_target_properties( rtmp
#                       PROPERTIES IMPORTED_LOCATION
#                       ${distribution_DIR}/armeabi/librtmp.so)

add_library( x264
             SHARED
             IMPORTED)
set_target_properties( x264
                       PROPERTIES IMPORTED_LOCATION
                       ${distribution_DIR}/armeabi/libx264.so)

include_directories(libs/include)
target_link_libraries( native-lib
                       log
                       ffmpeg
                       faac
#                       rtmp
                       x264
                       android
                       OpenSLES)