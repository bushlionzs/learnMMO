cmake_minimum_required (VERSION 2.8)

set(MINIOGRE_LIB_NAME miniOgre)
set(MINIOGRE_INC)

if (WIN32)
add_definitions(-DNOMINMAX)
add_definitions(-D_CRT_SECURE_NO_WARNINGS)
add_definitions(-DFILAMENT_DRIVER_SUPPORTS_VULKAN)
add_definitions(-DVK_NO_PROTOTYPES)
endif()


macro(add_subdir)
    set(SUBDIR_SRC_LIST)
    foreach(DIR ${ARGV})

        list(APPEND MINIOGRE_INC_SUBDIR ${DIR})

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

    list(APPEND MINIOGRE_SRC_LIST ${SUBDIR_SRC_LIST})

    unset(SUBDIR_SRC_LIST)
    unset(SRC_FILES)
endmacro()


add_subdir(animation)
add_subdir(base)
add_subdir(Utilities)
add_subdir(component)
add_subdir(models)
add_subdir(parser)
add_subdir(particle)
add_subdir(particle/ParticleFX)
add_subdir(render)
add_subdir(render/dx11Render)
add_subdir(render/dx12Render)
add_subdir(render/vulkanRender)
add_subdir(wow)
add_subdir(fg)
add_subdir(filament)

list(APPEND MINIOGRE_INC ${CMAKE_CURRENT_SOURCE_DIR}/animation)
list(APPEND MINIOGRE_INC ${CMAKE_CURRENT_SOURCE_DIR}/base)
list(APPEND MINIOGRE_INC ${CMAKE_CURRENT_SOURCE_DIR}/component)
list(APPEND MINIOGRE_INC ${CMAKE_CURRENT_SOURCE_DIR}/models)
list(APPEND MINIOGRE_INC ${CMAKE_CURRENT_SOURCE_DIR}/parser)
list(APPEND MINIOGRE_INC ${CMAKE_CURRENT_SOURCE_DIR}/particle)
list(APPEND MINIOGRE_INC ${CMAKE_CURRENT_SOURCE_DIR}/particle/ParticleFX)
list(APPEND MINIOGRE_INC ${CMAKE_CURRENT_SOURCE_DIR}/render)
list(APPEND MINIOGRE_INC ${CMAKE_CURRENT_SOURCE_DIR}/render/dx11Render)
list(APPEND MINIOGRE_INC ${CMAKE_CURRENT_SOURCE_DIR}/render/dx12Render)
list(APPEND MINIOGRE_INC ${CMAKE_CURRENT_SOURCE_DIR}/render/vulkanRender)
list(APPEND MINIOGRE_INC ${CMAKE_CURRENT_SOURCE_DIR}/wow)
list(APPEND MINIOGRE_INC ${CMAKE_CURRENT_SOURCE_DIR}/base)
list(APPEND MINIOGRE_INC ${CMAKE_CURRENT_SOURCE_DIR}/Utilities)
list(APPEND MINIOGRE_INC ${CMAKE_CURRENT_SOURCE_DIR}/filament)
list(APPEND MINIOGRE_INC ${CMAKE_CURRENT_SOURCE_DIR}/fg)
list(APPEND MINIOGRE_INC ${CMAKE_CURRENT_SOURCE_DIR})
list(APPEND MINIOGRE_INC ${ROOT_INC}/3rdSource/math/include)

add_library(${MINIOGRE_LIB_NAME} STATIC ${MINIOGRE_SRC_LIST})



target_include_directories(${MINIOGRE_LIB_NAME}
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${MINIOGRE_INC}
	${GAMEUTIL_INC}
	${ROOT_INC}/3rd
	${ROOT_INC}/3rdSource/OIS/includes
	${ROOT_INC}/GameUtil
	${ROOT_INC}/GlslUtil
	${ROOT_INC}/3rd/bullet3/include
	${ROOT_INC}/3rd/ffmpeg/include
	${ROOT_INC}/3rd/stormlib
	${ROOT_INC}/3rdSource/utils/include
	${ROOT_INC}/3rdSource/math/include
	${ROOT_INC}/3rdSource/vkmemalloc/include
	${ROOT_INC}/3rdSource/D3D12MemoryAllocator
	${ROOT_INC}/3rdSource/bluevk/include
	${ROOT_INC}/3rdSource
	${DYNETWORK_INCLUDE_HEADERS}
)

set( MyFavLib_LIBRARIES 
    debug shaderc_sharedd.lib optimized shaderc_shared.lib
)
	
target_link_libraries(${MINIOGRE_LIB_NAME} ${MyFavLib_LIBRARIES} SpirvTools utils bluevk VulkanMemoryAllocator
dxgi
dxguid
d3d12
d3d11
D3DCompiler)

set(MINIOGRE_INC ${MINIOGRE_INC} CACHE INTERNAL "shared")
