cmake_minimum_required (VERSION 2.8)

set(MULTIMEDIA_LIB_NAME MultiMedia)
set(MULTIMEDIA_INC)

if (WIN32)
add_definitions(-DNOMINMAX)
add_definitions(-D_CRT_SECURE_NO_WARNINGS)
add_definitions(-DWIN32_LEAN_AND_MEAN)
endif()


macro(add_subdir)
    set(SUBDIR_SRC_LIST)
    foreach(DIR ${ARGV})
        list(APPEND MULTIMEDIA_INC ${CMAKE_CURRENT_SOURCE_DIR}/${DIR})

        list(APPEND MULTIMEDIA_INC_SUBDIR ${DIR})

        file(GLOB SRC_FILES 
            ${CMAKE_CURRENT_SOURCE_DIR}/${DIR}/*.cpp 
            ${CMAKE_CURRENT_SOURCE_DIR}/${DIR}/*.c 
            ${CMAKE_CURRENT_SOURCE_DIR}/${DIR}/*.cc
            ${CMAKE_CURRENT_SOURCE_DIR}/${DIR}/*.h
            ${CMAKE_CURRENT_SOURCE_DIR}/${DIR}/*.hpp
        )
   
        list(APPEND SUBDIR_SRC_LIST ${SRC_FILES})
    endforeach()

    source_group(${ARGV0} FILES ${SUBDIR_SRC_LIST})

    list(APPEND MULTIMEDIA_LIST ${SUBDIR_SRC_LIST})

    unset(SUBDIR_SRC_LIST)
    unset(SRC_FILES)
endmacro()


add_subdir(.)



add_library(${MULTIMEDIA_LIB_NAME} STATIC ${MULTIMEDIA_LIST})

target_include_directories(${MULTIMEDIA_LIB_NAME}
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
	${MINIOGRE_INC}
	${DYNETWORK_INCLUDE_HEADERS}
	${ROOT_INC}/3rdSource/utils/include
	${ROOT_INC}/3rd/ffmpeg/include
)

target_link_libraries(${MULTIMEDIA_LIB_NAME} )

set(MULTIMEDIA_INC ${MULTIMEDIA_INC}  CACHE INTERNAL "shared")


