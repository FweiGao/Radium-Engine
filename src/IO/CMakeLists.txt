set(ra_io_target IO)
list(APPEND CMAKE_MESSAGE_INDENT "[${ra_io_target}] ")

project(${ra_io_target} LANGUAGES CXX VERSION ${Radium_VERSION})

option(RADIUM_IO_DEPRECATED "Provide deprecated loaders (to be removed without notice)" ON)
option(RADIUM_IO_ASSIMP "Provide loaders based on Assimp library" ON)
option(RADIUM_IO_TINYPLY "Provide loaders based on TinyPly library" ON)
option(RADIUM_IO_VOLUMES "Provide loader for volume pvm file format" ON)

include(filelistIO)

add_library(${ra_io_target} SHARED ${io_sources} ${io_headers} ${io_inlines})

# LocalDependencies form parent scope
populate_local_dependencies(NAME "assimp_DIR")
populate_local_dependencies(NAME "tinyply_DIR")

add_external_folder(
    IO ${RADIUM_EXTERNAL_DIR}/IO -DRADIUM_IO_ASSIMP=${RADIUM_IO_ASSIMP}
    -DRADIUM_IO_TINYPLY=${RADIUM_IO_TINYPLY} ${LocalDependencies}
)

if(RADIUM_IO_ASSIMP)
    find_package(assimp 5.0 REQUIRED NO_DEFAULT_PATH)
    set_target_properties(${ra_io_target} PROPERTIES IO_ASSIMP ${RADIUM_IO_ASSIMP})
endif(RADIUM_IO_ASSIMP)
if(RADIUM_IO_TINYPLY)
    find_package(tinyply REQUIRED NO_DEFAULT_PATH)
    set_target_properties(${ra_io_target} PROPERTIES IO_TINYPLY ${RADIUM_IO_TINYPLY})
endif(RADIUM_IO_TINYPLY)

if(RADIUM_IO_ASSIMP)
    if(MSVC)
        if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
            get_target_property(assimplib assimp::assimp IMPORTED_IMPLIB_DEBUG)
        else()
            get_target_property(assimplib assimp::assimp IMPORTED_IMPLIB_RELEASE)
        endif()
        target_link_libraries(${ra_io_target} PUBLIC ${assimplib})
    else()
        target_link_libraries(${ra_io_target} PUBLIC assimp::assimp)
    endif()

    set_target_properties(
        assimp::assimp PROPERTIES MAP_IMPORTED_CONFIG_MINSIZEREL Release
                                  MAP_IMPORTED_CONFIG_RELWITHDEBINFO Release
    )
endif(RADIUM_IO_ASSIMP)
if(RADIUM_IO_TINYPLY)
    target_link_libraries(${ra_io_target} PUBLIC tinyply)
endif(RADIUM_IO_TINYPLY)

if(RADIUM_IO_VOLUMES)
    set_target_properties(${ra_io_target} PROPERTIES RADIUM_IO_HAS_VOLUMES ${RADIUM_IO_VOLUMES})
endif()

add_dependencies(${ra_io_target} Core)
target_link_libraries(${ra_io_target} PUBLIC Core)
target_compile_definitions(${ra_io_target} PRIVATE RA_IO_EXPORTS IO_USE_CAMERA_LOADER)

message(STATUS "Configuring library ${ra_io_target} with standard settings")
configure_radium_target(${ra_io_target})
configure_radium_library(TARGET ${ra_io_target} FILES "${io_headers};${io_inlines}")

list(REMOVE_AT CMAKE_MESSAGE_INDENT -1)
