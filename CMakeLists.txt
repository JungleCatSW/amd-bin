cmake_minimum_required(VERSION 3.5)

project(amd-bin)

include_directories(${PROJECT_SOURCE_DIR}/common-lib-amd-APPSDK-3.0/3-0/include)
link_directories(${PROJECT_SOURCE_DIR}/common-lib-amd-APPSDK-3.0/3-0/lib/x86_64)


set(SOURCES main.cpp BinGen.cpp)
set(EXE_NAME amd-bin)
add_executable(${EXE_NAME} ${SOURCES})
target_link_libraries(${EXE_NAME} PUBLIC OpenCL)


