cmake_minimum_required(VERSION 3.16)
project(PingAnalyzer VERSION 1.0.0)

# 设置C++标准
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 设置输出目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# 定义源文件列表
set(SOURCES
    src/TODO1.cpp
    src/TODO2.cpp
    src/TODO3.cpp
    src/TODO4.cpp
    src/TODO5.cpp
    src/TODO6.cpp
)

# 为每个源文件创建可执行文件
foreach(SOURCE ${SOURCES})
    # 从源文件路径提取文件名（不含扩展名）
    get_filename_component(EXECUTABLE_NAME ${SOURCE} NAME_WE)
    
    # 创建可执行文件
    add_executable(${EXECUTABLE_NAME} ${SOURCE})
    
    # 设置包含目录
    target_include_directories(${EXECUTABLE_NAME} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
    )
    
    # 设置编译选项
    if(MSVC)
        target_compile_options(${EXECUTABLE_NAME} PRIVATE /W4)
    else()
        target_compile_options(${EXECUTABLE_NAME} PRIVATE -Wall -Wextra)
    endif()
endforeach()

# 复制数据文件到构建目录
file(COPY data DESTINATION ${CMAKE_BINARY_DIR})