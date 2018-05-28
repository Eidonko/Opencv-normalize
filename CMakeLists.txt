cmake_minimum_required(VERSION 2.8)
project( Normalize2RGB )
find_package( OpenCV REQUIRED )
add_executable( n2rgb n2rgb.cpp )
target_link_libraries( n2rgb ${OpenCV_LIBS} )

