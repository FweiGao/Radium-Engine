#set the compile defdinition for current directory (i.e. externals)
if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    if(CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
        string(REGEX REPLACE "/W[0-4]" "/w " EXTERNAL_CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
    else()
        set(EXTERNAL_CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /w ")
    endif()
else()
    set(EXTERNAL_CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -w")
endif()

# if we are in a radium process, use radium specific option, else use cmake option.
if(RADIUM_EXTERNAL_CMAKE_INSTALL_MESSAGE)
    set(EXTERNAL_INSTALL_MESSAGE ${RADIUM_EXTERNAL_CMAKE_INSTALL_MESSAGE})
else()
    set(EXTERNAL_INSTALL_MESSAGE ${CMAKE_INSTALL_MESSAGE})
endif()

set(RADIUM_EXTERNAL_CMAKE_OPTIONS
    -DCMAKE_CXX_STANDARD=${CMAKE_CXX_STANDARD}
    -DCMAKE_CXX_STANDARD_REQUIRED=${CMAKE_CXX_STANDARD_REQUIRED}
    -DCMAKE_GENERATOR=${CMAKE_GENERATOR}
    -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
    -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}
    -DCMAKE_INSTALL_MESSAGE=${EXTERNAL_INSTALL_MESSAGE}
    -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
    -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
    -DCMAKE_CXX_FLAGS=${EXTERNAL_CMAKE_CXX_FLAGS}
    -DCMAKE_CXX_FLAGS_DEBUG=${CMAKE_CXX_FLAGS_DEBUG}
    -DCMAKE_CXX_FLAGS_RELEASE=${CMAKE_CXX_FLAGS_RELEASE}
    -DCMAKE_CXX_FLAGS_RELWITHDEBINFO=${CMAKE_CXX_FLAGS_RELWITHDEBINFO}
    -DCMAKE_SHARED_LINKER_FLAGS=${CMAKE_SHARED_LINKER_FLAGS}
    -DCMAKE_PREFIX_PATH=${CMAKE_PREFIX_PATH}
    -DCMAKE_MODULE_PATH=${CMAKE_MODULE_PATH}
    -DCMAKE_OBJECT_PATH_MAX=${CMAKE_OBJECT_PATH_MAX}
    -DCMAKE_MACOSX_RPATH=TRUE
    --no-warn-unused-cli
)

# First message formating function, need other to have better consistency
macro(status_message MODULE NAME VAR)
    message(STATUS "${MODULE} Using ${NAME} from ${VAR}")
endmacro()

macro(check_externals_prerequisite)
    find_package(Git REQUIRED)
endmacro()
