cmake_minimum_required(VERSION 3.7 FATAL_ERROR)
project(MacedonianLetterFrequencies)
set(CMAKE_CXX_STANDARD 11)

# Compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wextra")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pedantic")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pedantic-errors")

set(ICU_DEBUG ON)
# Boost and ICU
find_package(Boost 1.63.0 COMPONENTS system thread)

include_directories(${PROJECT_SOURCE_DIR}/include)

# Executable
add_executable(scrape src/http_client.cpp src/scraper.cpp)
target_link_libraries(scrape ${Boost_LIBRARIES})
target_link_libraries(scrape icui18n icuuc icudata icuio)
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
target_link_libraries(scrape Threads::Threads)
