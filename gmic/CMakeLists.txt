#
#  File        : CMakeLists.txt
#                ( cmake-based build system configuration file )
#
#  Basic usage :
#
#                 - cmake .
#                 - make
#
#                ( https://gmic.eu )
#
#  Author      : Sander Knopper
#                ( https://github.com/saknopper )
#
#                Based on work by Ștefan Talpalaru
#                ( https://github.com/stefantalpalaru )
#
#  Licenses    : This file is 'dual-licensed', you have to choose one
#                 of the two licenses below to apply.
#
#                CeCILL-C
#                The CeCILL-C license is close to the GNU LGPL.
#                ( http://cecill.info/licences/Licence_CeCILL-C_V1-en.html )
#
#            or  CeCILL v2.1
#                The CeCILL license is compatible with the GNU GPL.
#                ( http://cecill.info/licences/Licence_CeCILL_V2.1-en.html )
#
#  This software is governed either by the CeCILL or the CeCILL-C license
#  under French law and abiding by the rules of distribution of free software.
#  You can  use, modify and or redistribute the software under the terms of
#  the CeCILL or CeCILL-C licenses as circulated by CEA, CNRS and INRIA
#  at the following URL: "http://cecill.info".
#
#  As a counterpart to the access to the source code and  rights to copy,
#  modify and redistribute granted by the license, users are provided only
#  with a limited warranty  and the software's author,  the holder of the
#  economic rights,  and the successive licensors  have only  limited
#  liability.
#
#  In this respect, the user's attention is drawn to the risks associated
#  with loading,  using,  modifying and/or developing or reproducing the
#  software by the user in light of its specific status of free software,
#  that may mean  that it is complicated to manipulate,  and  that  also
#  therefore means  that it is reserved for developers  and  experienced
#  professionals having in-depth computer knowledge. Users are therefore
#  encouraged to load and test the software's suitability as regards their
#  requirements in conditions enabling the security of their systems and/or
#  data to be ensured and,  more generally, to use and operate it in the
#  same conditions as regards security.
#
#  The fact that you are presently reading this means that you have had
#  knowledge of the CeCILL and CeCILL-C licenses and that you accept its terms.
#

cmake_minimum_required(VERSION 3.8)
cmake_policy(SET CMP0046 OLD)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
  message("Build directory is equal to source directory. Binaries will be put in the src directory.")
  message("")

  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/src/" CACHE FILEPATH "Output directory for all targets." FORCE)
  set(GMIC_BINARIES_PATH ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
else()
  set(GMIC_BINARIES_PATH ${CMAKE_BINARY_DIR})
endif()

set(CMAKE_CXX_STANDARD 11)
set(CXX_STANDARD_REQUIRED ON)

project(gmic CXX C)

find_package(PkgConfig)
include(FeatureSummary)
include(GNUInstallDirs)

# options controlling the build process
option(BUILD_LIB "Build the GMIC shared library" ON)
option(BUILD_LIB_STATIC "Build the GMIC static library" ON)
option(BUILD_CLI "Build the CLI interface" ON)
option(BUILD_MAN "Build the manpage" ON)
option(BUILD_BASH_COMPLETION "Build Bash completion" ON)
option(CUSTOM_CFLAGS "Override default compiler optimization flags" OFF)
option(ENABLE_CURL "Add support for curl" ON)
option(ENABLE_X "Add support for X11" ON)
option(ENABLE_FFMPEG "Add support for FFMpeg" ON)
option(ENABLE_FFTW "Add support for FFTW" ON)
option(ENABLE_GRAPHICSMAGICK "Add support for GrahicsMagick" ON)
option(ENABLE_JPEG "Add support for handling images in Jpeg format" ON)
option(ENABLE_OPENCV "Add support for OpenCV" ON)
option(ENABLE_OPENEXR "Add support for handling images in EXR format" ON)
option(ENABLE_OPENMP "Add support for parallel processing" ON)
option(ENABLE_PNG "Add support for handling images in PNG format" ON)
option(ENABLE_TIFF "Add support for handling images in Tiff format" ON)
option(ENABLE_ZLIB "Add support for data compression via Zlib" ON)
option(ENABLE_DYNAMIC_LINKING "Dynamically link the binaries to the GMIC shared library" OFF)
option(ENABLE_LTO "Enable -flto (Link Time Optimizer) on gcc and clang" OFF)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

# compile flags
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/modules)
set(CMAKE_POSITION_INDEPENDENT_CODE True)

set(COMPILE_FLAGS "-Dgmic_build -Dcimg_use_vt100 -Dgmic_is_parallel -Dcimg_use_abort")
if(APPLE)
   set(COMPILE_FLAGS "${COMPILE_FLAGS} -mmacosx-version-min=10.8 -stdlib=libc++ -Wno-error=c++11-narrowing -Wc++11-extensions -fpermissive")
else()
  set(COMPILE_FLAGS "${COMPILE_FLAGS} -Wno-error=narrowing -fno-ipa-sra -fpermissive")
endif()

if(NOT "${PRERELEASE_TAG}" STREQUAL "")
  set(COMPILE_FLAGS "${COMPILE_FLAGS} -Dgmic_prerelease=\"${PRERELEASE_TAG}\"")
endif()

set(EXTRA_LIBRARIES)
set(CLI_COMPILE_FLAGS)

# OpenMP support
if(ENABLE_OPENMP)
  if(NOT APPLE)
    set(COMPILE_FLAGS "${COMPILE_FLAGS} -fopenmp -Dcimg_use_openmp")
    list(APPEND EXTRA_LIBRARIES "-lgomp")
  endif()
endif()

# Zlib support
if(ENABLE_ZLIB)
  find_package(ZLIB)
endif()
if(ZLIB_FOUND)
  set(COMPILE_FLAGS "${COMPILE_FLAGS} -Dcimg_use_zlib")
  include_directories(${ZLIB_INCLUDE_DIRS})
  link_directories(${ZLIB_LIBRARY_DIRS})
endif()

# Curl support
if(ENABLE_CURL)
  find_package(CURL)
endif()
if(CURL_FOUND)
  set(COMPILE_FLAGS "${COMPILE_FLAGS} -Dcimg_use_curl")
  include_directories(${CURL_INCLUDE_DIRS})
  link_directories(${CURL_LIBRARY_DIRS})
endif()

# X11 support
if(ENABLE_X)
  find_package(X11)
endif()
if(X11_FOUND)
  set(COMPILE_FLAGS "${COMPILE_FLAGS} -Dcimg_display=1 -Dcimg_appname=\\\"gmic\\\"")
  include_directories(${X11_INCLUDE_DIR})
  include_directories(${X11_INCLUDE_DIRS})
  link_directories(${X11_LIBRARY_DIR})
  link_directories(${X11_LIBRARY_DIRS})
else()
  set(COMPILE_FLAGS "${COMPILE_FLAGS} -Dcimg_display=0 -Dcimg_appname=\\\"gmic\\\"")
endif()
message( "X11_INCLUDE_DIR: " ${X11_INCLUDE_DIR} )
message( "X11_INCLUDE_DIRs: " ${X11_INCLUDE_DIRs} )
message( "X11_LIBRARY_DIR: " ${X11_LIBRARY_DIR} )
message( "X11_LIBRARY_DIRS: " ${X11_LIBRARY_DIRS} )
if(X11_XShm_FOUND)
  set(COMPILE_FLAGS "${COMPILE_FLAGS} -Dcimg_use_xshm")
endif()

if(ENABLE_FFTW)
  pkg_check_modules(FFTW3 fftw3>=3.0)
endif()
if(FFTW3_FOUND)
  set(COMPILE_FLAGS "${COMPILE_FLAGS} -Dcimg_use_fftw3")
  include_directories(${FFTW3_INCLUDE_DIRS})
  link_directories(${FFTW3_LIBRARY_DIRS})

  find_library(FFTW3_THREADS_LIB fftw3_threads PATHS ${FFTW3_LIBRARY_DIRS})
  if(FFTW3_THREADS_LIB STREQUAL "FFTW3_THREADS_LIB-NOTFOUND")
    set(COMPILE_FLAGS "${COMPILE_FLAGS} -Dcimg_use_fftw3_singlethread")
  else()
    list(APPEND EXTRA_LIBRARIES "-lfftw3_threads")
  endif()
endif()

if(ENABLE_OPENCV)
  pkg_check_modules(OPENCV opencv)
endif()
if(OPENCV_FOUND)
  set(CLI_COMPILE_FLAGS "${CLI_COMPILE_FLAGS} -Dcimg_use_opencv")
  include_directories(${OPENCV_INCLUDE_DIRS})
  link_directories(${OPENCV_LIBRARY_DIRS})
endif()

if(ENABLE_GRAPHICSMAGICK)
  pkg_check_modules(GRAPHICSMAGICK GraphicsMagick++)
endif()
if(GRAPHICSMAGICK_FOUND)
  set(CLI_COMPILE_FLAGS "${CLI_COMPILE_FLAGS} -Dcimg_use_magick")
  include_directories(${GRAPHICSMAGICK_INCLUDE_DIRS})
  link_directories(${GRAPHICSMAGICK_LIBRARY_DIRS})
endif()

if(ENABLE_TIFF)
  find_package(TIFF)
endif()
if(TIFF_FOUND)
  set(CLI_COMPILE_FLAGS "${CLI_COMPILE_FLAGS} -Dcimg_use_tiff")
  include_directories(${TIFF_INCLUDE_DIRS})
  link_directories(${TIFF_LIBRARY_DIRS})
endif()

if(ENABLE_PNG)
  find_package(PNG)
endif()
if(PNG_FOUND)
  set(CLI_COMPILE_FLAGS "${CLI_COMPILE_FLAGS} -Dcimg_use_png")
  include_directories(${PNG_INCLUDE_DIRS})
  link_directories(${PNG_LIBRARY_DIRS})
endif()

if(ENABLE_JPEG)
  find_package(JPEG)
endif()
if(JPEG_FOUND)
  set(CLI_COMPILE_FLAGS "${CLI_COMPILE_FLAGS} -Dcimg_use_jpeg")
  include_directories(${JPEG_INCLUDE_DIRS})
  link_directories(${JPEG_LIBRARY_DIRS})
endif()

if(ENABLE_OPENEXR)
  pkg_check_modules(OPENEXR OpenEXR)
endif()
if(OPENEXR_FOUND)
  set(CLI_COMPILE_FLAGS "${CLI_COMPILE_FLAGS} -Dcimg_use_openexr")
  include_directories(${OPENEXR_INCLUDE_DIRS})
  link_directories(${OPENEXR_LIBRARY_DIRS})
endif()

if (ENABLE_LTO AND (CMAKE_COMPILER_IS_GNUCC OR (CMAKE_CSS_COMPILER_IS STREQUAL "Clang")))
  set(CLI_COMPILE_FLAGS "${CLI_COMPILE_FLAGS} -flto")
  list(APPEND EXTRA_LIBRARIES "-flto")
endif()

if(ENABLE_DYNAMIC_LINKING)
  if(NOT BUILD_LIB)
    message(FATAL_ERROR "ENABLE_DYNAMIC_LINKING needs BUILD_LIB")
  endif()
  set(CMAKE_SKIP_RPATH TRUE)
endif()

# CImg.h header
if(NOT EXISTS ${CMAKE_SOURCE_DIR}/src/CImg.h)
  file(DOWNLOAD https://github.com/dtschump/CImg/raw/master/CImg.h ${CMAKE_SOURCE_DIR}/src/CImg.h)
  execute_process(COMMAND ${CMAKE_COMMAND} -E touch ${CMAKE_SOURCE_DIR}/src/CImg.h)
endif()

# gmic_stdlib.h header
if(NOT EXISTS ${CMAKE_SOURCE_DIR}/src/gmic_stdlib.h)
  file(DOWNLOAD https://gmic.eu/gmic_stdlib.h ${CMAKE_SOURCE_DIR}/src/gmic_stdlib.h)
  execute_process(COMMAND ${CMAKE_COMMAND} -E touch ${CMAKE_SOURCE_DIR}/src/gmic_stdlib.h)
endif()

add_custom_target(gmic_extra_headers DEPENDS src/CImg.h src/gmic_stdlib.h)

set(CMAKE_CXX_FLAGS_DEBUG "-g -ansi -Wall -Wextra -pedantic -Dcimg_verbosity=3 ${COMPILE_FLAGS}")
set(CMAKE_CXX_FLAGS_RELEASE "${COMPILE_FLAGS}")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-g ${COMPILE_FLAGS}")

if(NOT CUSTOM_CFLAGS)
  set(CMAKE_CXX_FLAGS_DEBUG "-Og ${CMAKE_CXX_FLAGS_DEBUG}")
  set(CMAKE_CXX_FLAGS_RELEASE "-Ofast ${CMAKE_CXX_FLAGS_RELEASE}")
  set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-Ofast ${CMAKE_CXX_FLAGS_RELWITHDEBINFO}")
endif()

# source files
set(CLI_Includes src/gmic.h)
set(CLI_Sources src/gmic.cpp)

if(MINGW)
  list(APPEND EXTRA_LIBRARIES "-Wl,--stack,16777216")
else()
  list(APPEND EXTRA_LIBRARIES "-lpthread")
endif()


if(BUILD_LIB)
  add_library(libgmic SHARED ${CLI_Includes} ${CLI_Sources})
  add_dependencies(libgmic gmic_extra_headers)
  set_target_properties(libgmic PROPERTIES COMPILE_FLAGS "${CLI_COMPILE_FLAGS}")
  set_target_properties(libgmic PROPERTIES SOVERSION "1" OUTPUT_NAME "gmic")
  if(NOT APPLE)
    #the following is automatic unless NO_SONAME is set
    #set_target_properties(libgmic PROPERTIES LINK_FLAGS "-Wl,-soname,libgmic.so.1")
  endif()
  target_link_libraries(libgmic
    ${X11_LIBRARIES}
    ${TIFF_LIBRARIES}
    ${PNG_LIBRARIES}
    ${JPEG_LIBRARIES}
    ${GRAPHICSMAGICK_LIBRARIES}
    ${OPENEXR_LIBRARIES}
    ${OPENCV_LIBRARIES}
    ${ZLIB_LIBRARIES}
    ${CURL_LIBRARIES}
    ${FFTW3_LIBRARIES}
    ${EXTRA_LIBRARIES}
  )

  install(TARGETS libgmic EXPORT GmicTargets
    PUBLIC_HEADER DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
    ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
  )
  install(FILES src/gmic.h DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}")
endif()


if(BUILD_LIB_STATIC)
  add_library(libgmicstatic STATIC ${CLI_Includes} ${CLI_Sources})
  add_dependencies(libgmicstatic gmic_extra_headers)
  set_target_properties(libgmicstatic PROPERTIES COMPILE_FLAGS "${CLI_COMPILE_FLAGS}")
  set_target_properties(libgmicstatic PROPERTIES OUTPUT_NAME "gmic")
  target_link_libraries(libgmicstatic
    ${X11_LIBRARIES}
    ${TIFF_LIBRARIES}
    ${PNG_LIBRARIES}
    ${JPEG_LIBRARIES}
    ${GRAPHICSMAGICK_LIBRARIES}
    ${OPENEXR_LIBRARIES}
    ${OPENCV_LIBRARIES}
    ${ZLIB_LIBRARIES}
    ${CURL_LIBRARIES}
    ${FFTW3_LIBRARIES}
    ${EXTRA_LIBRARIES}
  )

  install(TARGETS libgmicstatic EXPORT GmicTargets ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}")
  install(FILES src/gmic.h DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}")
endif()


if(BUILD_CLI)
  if(ENABLE_DYNAMIC_LINKING)
    add_executable(gmic ${CLI_Includes} src/gmic_cli.cpp)
    add_dependencies(gmic gmic_extra_headers)
    add_dependencies(gmic libgmic)
    target_link_libraries(gmic
      "libgmic"
    )
  else()
    add_executable(gmic ${CLI_Includes} ${CLI_Sources} src/gmic_cli.cpp)
    add_dependencies(gmic gmic_extra_headers)
    target_link_libraries(gmic
      ${X11_LIBRARIES}
      ${TIFF_LIBRARIES}
      ${PNG_LIBRARIES}
      ${JPEG_LIBRARIES}
      ${GRAPHICSMAGICK_LIBRARIES}
      ${OPENEXR_LIBRARIES}
      ${OPENCV_LIBRARIES}
      ${ZLIB_LIBRARIES}
      ${CURL_LIBRARIES}
      ${FFTW3_LIBRARIES}
      ${EXTRA_LIBRARIES}
    )
  endif()

  set_target_properties(gmic PROPERTIES COMPILE_FLAGS "${CLI_COMPILE_FLAGS}")
  install(TARGETS gmic RUNTIME DESTINATION bin LIBRARY DESTINATION lib)
endif()

if(BUILD_MAN)
  file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/man)
  add_custom_command(
    OUTPUT ${CMAKE_BINARY_DIR}/man/gmic.1
    DEPENDS gmic
    COMMAND LD_LIBRARY_PATH=${GMIC_BINARIES_PATH} ${GMIC_BINARIES_PATH}/gmic -v - ${CMAKE_SOURCE_DIR}/src/gmic_stdlib.gmic raw:${CMAKE_SOURCE_DIR}/src/gmic_stdlib.gmic,uchar -__help man 2> ${CMAKE_BINARY_DIR}/man/gmic.1
  )
  add_custom_target(man ALL DEPENDS ${CMAKE_BINARY_DIR}/man/gmic.1)
  install(FILES ${CMAKE_BINARY_DIR}/man/gmic.1 DESTINATION ${CMAKE_INSTALL_MANDIR}/man1/)
endif()

if(BUILD_BASH_COMPLETION)
  file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/resources)
  add_custom_command(
    OUTPUT ${CMAKE_BINARY_DIR}/resources/gmic_bashcompletion.sh
    DEPENDS gmic
    COMMAND LD_LIBRARY_PATH=${GMIC_BINARIES_PATH} ${GMIC_BINARIES_PATH}/gmic -v - ${CMAKE_SOURCE_DIR}/src/gmic_stdlib.gmic raw:${CMAKE_SOURCE_DIR}/src/gmic_stdlib.gmic,uchar -document_gmic bash 2> ${CMAKE_BINARY_DIR}/resources/gmic_bashcompletion.sh
  )
  add_custom_target(bashcompletion ALL DEPENDS ${CMAKE_BINARY_DIR}/resources/gmic_bashcompletion.sh)
endif()

include(CMakePackageConfigHelpers)
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/GmicConfig.cmake.in" "@PACKAGE_INIT@\ninclude(\${CMAKE_CURRENT_LIST_DIR}/GmicTargets.cmake)\n")
configure_package_config_file(
  ${CMAKE_CURRENT_BINARY_DIR}/GmicConfig.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/GmicConfig.cmake
  INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/gmic
)
install(EXPORT GmicTargets DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/gmic)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/GmicConfig.cmake DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/gmic)
feature_summary(WHAT ALL)
