cmake_minimum_required(VERSION 3.21.0)
project(umba-tr VERSION 0.1.0 LANGUAGES C CXX)
# set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)


set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)


set(UMBA_STATIC_RUNTIME ON)
set(UMBA_USE_BOOST      OFF)

include(${CMAKE_CURRENT_LIST_DIR}/.cmake/umba.cmake)



# message("BOOST_ROOT: ${BOOST_ROOT}")
# message("Compiler: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
# MinGW GCC 7.3 - Compiler version: 7.3.0
# MinGW GCC 13.2 - Compiler version: 13.2.0

add_compile_definitions("YAML_CPP_STATIC_DEFINE")
add_compile_definitions("MARTY_CPP_USE_MARTY_TR")


include_directories(${LIB_ROOT})

add_subdirectory(${LIB_ROOT}/encoding)
add_subdirectory(${LIB_ROOT}/marty_cpp)
add_subdirectory(${LIB_ROOT}/marty_tr)
add_subdirectory(${LIB_ROOT}/marty_utf)
add_subdirectory(${LIB_ROOT}/marty_yaml_toml_json)
add_subdirectory(${LIB_ROOT}/nlohmann)    # https://json.nlohmann.me/integration/cmake/#external
add_subdirectory(${LIB_ROOT}/sfmt)
add_subdirectory(${LIB_ROOT}/umba)
add_subdirectory(${LIB_ROOT}/yaml-cpp)



umba_configure_boost("STATIC_LIBS" "MULTITHREADED" "${STATIC_RUNTIME}")


if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    #-fdiagnostics-color=always
	#set(CMAKE_CXX_FLAGS "-Wa,-mbig-obj -flto -flto-report  -fno-rtti -Wall -Wextra -Werror -pedantic-errors -Wctor-dtor-privacy -Wextra-semi -Wnon-virtual-dtor -Woverloaded-virtual -Wredundant-decls -Wsign-promo")
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  #-fignore-unknown-pragmas
  set(CMAKE_CXX_FLAGS "-Woverloaded-virtual -Wno-ctor-dtor-privacy -Wsign-promo")
  #add_compile_options(-fdiagnostics-color=always)
  #add_compile_options(-Wa,-mbig-obj)
  # -fno-rtti
  #add_compile_options(-Wall -Wno-unknown-pragmas -Wduplicated-branches -Wduplicated-cond -Wlogical-op -Wredundant-decls -Wno-unused-function)
  #add_compile_options(-Wno-unknown-pragmas)
  #-Wno-unknown-pragmas
  #-Wnon-virtual-dtor -Wctor-dtor-privacy
  #add_compile_options(-w)
  #add_compile_options(-w -Wa,-mbig-obj)
  #-fdiagnostics-color=always 
  #-Wall   
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Intel")
  # using Intel C++
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
  # using Visual Studio C++
endif()


add_compile_definitions("USE_UMBA_LOUT_COUT")

file(GLOB_RECURSE sources   "${SRC_ROOT}/*.cpp")
file(GLOB_RECURSE headers   "${SRC_ROOT}/*.h")
file(GLOB_RECURSE resources "${SRC_ROOT}/*.rc")
umba_make_sources_tree("${SRC_ROOT}" "${sources}" "${headers}" "${resources}")


set(PLATFORM_LIBS)
if(WIN32) # Всё нормально, это про таргет
    list(APPEND PLATFORM_LIBS ws2_32 iphlpapi shell32)
endif()

set(COMMON_LIBS encoding::encoding marty::cpp marty::tr marty::utf marty::yaml_toml_json nlohmann_json::nlohmann_json sfmt::sfmt umba::umba yaml-cpp::yaml-cpp)


set(COMMON_UMBA_OPTIONS "BIGOBJ" "UTF8" "WALL" "WERR")


set(TARGET umba-tr)
add_executable(${TARGET} "${SRC_ROOT}/umba-tr/${TARGET}.cpp" "${headers}" ${all_docs} ${all_docs_src})
umba_add_target_options(${TARGET} ${COMMON_UMBA_OPTIONS} "CONSOLE") 
target_link_libraries(${TARGET} PRIVATE "${PLATFORM_LIBS}" "${COMMON_LIBS}")
