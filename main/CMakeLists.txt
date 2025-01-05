# 设置变量APP_SRC，包含driver/src/目录下的所有.cpp和.c文件
file(GLOB APP_SRC "driver/src/*.cpp" "driver/src/*.c" "utils/src/*.c" "utils/src/*.cpp" "main.cpp")
set(APP_INC "." "driver/inc" "utils/inc")

idf_component_register(SRCS ${APP_SRC}
                       INCLUDE_DIRS ${APP_INC})

# 产生配置文件
execute_process(
    COMMAND python -c "open('${CMAKE_BINARY_DIR}/config.ini.bin', 'wb').write(open('config.ini', 'rb').read() + b'\\0')"
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../config"
    RESULT_VARIABLE result
)

# 检查命令执行结果
if(result EQUAL "0")
    message(STATUS "Python command executed successfully")
else()
    message(FATAL_ERROR "Python command failed with result: ${result}")
endif()