cmake_minimum_required(VERSION 3.13)
if(APPLE)
    cmake_policy(SET CMP0042 NEW)
endif(APPLE)

project(HeadlessLibAndApp VERSION 1.0.2)

# To prevent installation ordering issue and make the library installed before the application, so
# that fixup_bundle will success on windows, we have to define libraries and executable in different
# subdirectories. see https://gitlab.kitware.com/cmake/cmake/-/issues/17989

# configure, build and install the Radium headless library
add_subdirectory(libHeadless)

# configure, build and install the demo application for Radium headless
set(IN_LIBHEADLESS_BUILD_TREE TRUE)
add_subdirectory(Application)
