cmake_minimum_required(VERSION 3.9)
project(ecat)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(source_dir ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(header_ext .h)
set(source_ext .cpp)

# Targets
set(exec_target run)
set(lib_target ethercatcim)

# IGH Ethercat libraries path
#set(igh_lib_path "/root/ethercat-stable-1.5/lib/") # стенд в тринити
set(igh_lib_path "/home/user/igh/ethercat-hg/lib") # стенд в МЭИ
#set(igh_lib_path "/home/artamonov/artamonov/ethercat-hg/lib") # artamonov
set(igh_objects 
    ${igh_lib_path}/libethercat_la-common.o
    ${igh_lib_path}/libethercat_la-domain.o
    ${igh_lib_path}/libethercat_la-master.o 
    ${igh_lib_path}/libethercat_la-reg_request.o
    ${igh_lib_path}/libethercat_la-sdo_request.o
    ${igh_lib_path}/libethercat_la-slave_config.o
    ${igh_lib_path}/libethercat_la-soe_request.o
    ${igh_lib_path}/libethercat_la-voe_handler.o
    )

# ! В БУДУЩЕМ УКАЗАТЬ КОНКРЕТНЫЕ СПИСКИ ! 
file(GLOB_RECURSE sources ${source_dir}/*${source_ext})
file(GLOB_RECURSE headers ${source_dir}/*${header_ext})

# Remove old headers from include/ dir
file(GLOB headers_copy ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h)
foreach(_header_copy ${headers_copy})
    get_filename_component(_header_filename ${_header_copy} NAME)
    file(REMOVE include/${_header_filename})
endforeach()


add_library(${lib_target} STATIC ${sources})
target_link_libraries(${lib_target} ${igh_objects}) 

# Create executable
add_executable(${exec_target} main${source_ext})
target_link_libraries(${exec_target} ${lib_target})

# Set headers
foreach(_header ${headers})
    get_filename_component(_include_dir ${_header} DIRECTORY)
    target_include_directories(${exec_target} PRIVATE ${_include_dir})
    target_include_directories(${lib_target} PRIVATE ${_include_dir}) 
    
    # Copy header to include/ directory
    file(REMOVE ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h)
    file(GLOB HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h)
    file(COPY ${_header} DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/include/)
endforeach()
