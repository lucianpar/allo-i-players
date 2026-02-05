cmake_minimum_required(VERSION 3.29)


project(allo-i-players)
set(APP_NAME allo-i-players)

# cmake scripts are kept in the cmake directory
list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

# set compiler/platform specific flags
if(MSVC)
  set(CMAKE_BUILD_PARALLEL_LEVEL $ENV{NUMBER_OF_PROCESSORS})
  # add_compile_options(/utf-8)
  # set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /STACK:4194304")
endif()

# add source files to executable
# add_executable(${APP_NAME} adm-allo-player/mainplayer.cpp)
# add_executable(${APP_NAME} miniShader/src/shaderPlayback.cpp)
add_executable(${APP_NAME} immersivePlayer.cpp)

# add allolib to project & link
# Allow older subprojects that declare very old cmake_minimum_required() (e.g. 2.8)
# Some bundled/external CMakeLists use ancient VERSION arguments which CMake
# 3.19+ rejects by default. Set this cache variable so configuration will
# attempt to proceed without editing every sub-CMakeLists file.
if(NOT DEFINED CMAKE_POLICY_VERSION_MINIMUM)
  set(CMAKE_POLICY_VERSION_MINIMUM 3.5 CACHE STRING "Minimum CMake policy version to allow older subprojects" FORCE)
endif()

add_subdirectory(allolib)
# allolib creates the target `al` (see allolib/CMakeLists.txt). Older
# setups referenced `alapp`; link against the `al` target instead so the
# linker can find the library provided by the subproject.
# Check if `alapp` exists and can be linked
# Check if the `alapp` target exists (created by allolib)
if(TARGET alapp)
  message(STATUS "Found alapp target. Linking with alapp.")
  target_link_libraries(${APP_NAME} PRIVATE alapp)
else()
  # If `alapp` target does not exist, fall back to `al`
  message(STATUS "alapp target not found. Falling back to al.")
  target_link_libraries(${APP_NAME} PRIVATE al)
endif()
### CHECK THIS AREA IF THERE ARE CMAKE ISSUES WITH LINKING ALLOLIB ###

# add al_ext to project & link
if (EXISTS ${CMAKE_CURRENT_LIST_DIR}/al_ext)
  add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/al_ext)
  get_target_property(AL_EXT_LIBRARIES al_ext AL_EXT_LIBRARIES)
  target_link_libraries(${APP_NAME} PRIVATE ${AL_EXT_LIBRARIES})
endif()

# example line for find_package usage
# find_package(Qt5Core REQUIRED CONFIG PATHS "C:/Qt/5.12.0/msvc2017_64/lib" NO_DEFAULT_PATH)

# example of linking other libraries
# target_include_directories(${APP_NAME} PRIVATE ${PATH_TO_INCLUDE_DIR})
# target_link_libraries(${APP_NAME} PRIVATE ${PATH_TO_LIB_FILE})

# Suppress warnings for now
target_compile_options(${PROJECT_NAME} PRIVATE -w)

# binaries are put into the ./bin directory by default
set_target_properties(${APP_NAME} PROPERTIES
  CXX_STANDARD 17
  CXX_STANDARD_REQUIRED ON
  CXX_EXTENSIONS OFF
  RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/bin
  RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_CURRENT_LIST_DIR}/debug
  RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_CURRENT_LIST_DIR}/bin
  # ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/lib
  # LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/bin
)