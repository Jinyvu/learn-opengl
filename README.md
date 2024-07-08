**# How to run this project**

1. download dependencies
   + `git submodule init && git submodule update`
   + Enter ./CMakeLists.txt and change FFMPEG_DIR to your local FFmpeg dir
2. makefile
   + `mkdir build && cd build`
   + `cmake ..`
   + `make && ./HelloGL`

> to run different file, you need to change SOURCES in ./CMakeLists.txt