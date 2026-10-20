cmake_minimum_required(VERSION 3.16)

project(lvgl_simulator
    VERSION 1.0.0
    LANGUAGES C CXX)

# ---------------------------------------------------------------------------
# 语言标准 / 默认构建类型
# ---------------------------------------------------------------------------
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 未指定时默认 RelWithDebInfo（-O2 -g，但走 CMake 标准流程）
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING "构建类型" FORCE)
endif()

# ---------------------------------------------------------------------------
# 目录布局
#
#   sm/
#   ├── main.c                 入口（lv_init + SDL HAL + 主循环）
#   ├── include/               模拟器自己的 lv_conf.h / lv_drv_conf.h
#   ├── libs/lvgl              子模块：LVGL 内核
#   ├── libs/lv_drivers        子模块：PC 端 SDL 驱动
#   └── myCode/                界面代码 + 资源（资源根目录就是它）
# ---------------------------------------------------------------------------
set(SIM_DIR       ${CMAKE_CURRENT_SOURCE_DIR})
set(LIB_DIR       ${SIM_DIR}/libs)
set(MYCODE_DIR    ${SIM_DIR}/myCode)
set(RUNTIME_DIR   ${SIM_DIR}/bin)

# 资源根目录：编译时写死绝对路径，运行期可用环境变量 MYGUI_ASSETS 覆盖
file(TO_CMAKE_PATH "${MYCODE_DIR}" MYGUI_ASSET_DIR_POSIX)

# ---------------------------------------------------------------------------
# 依赖
# ---------------------------------------------------------------------------
find_package(SDL2 REQUIRED)

# ---------------------------------------------------------------------------
# 源文件
# ---------------------------------------------------------------------------
# LVGL 内核（C）
file(GLOB_RECURSE LVGL_SOURCES CONFIGURE_DEPENDS ${LIB_DIR}/lvgl/src/*.c)

# lv_drivers：只编译 SDL 后端（sdl.c / sdl_common.c / sdl_gpu.c）
file(GLOB_RECURSE LV_DRIVER_SOURCES CONFIGURE_DEPENDS ${LIB_DIR}/lv_drivers/sdl/*.c)
if(NOT LV_DRIVER_SOURCES)
    message(FATAL_ERROR "lv_drivers/sdl 源码为空，请检查子模块：${LIB_DIR}/lv_drivers")
endif()

# 界面代码：myCode 下所有 .c（APP/ GUI/ Common/ + launcher.c）
file(GLOB_RECURSE APP_SOURCES CONFIGURE_DEPENDS ${MYCODE_DIR}/*.c)
if(NOT APP_SOURCES)
    message(FATAL_ERROR "myCode 下没有找到任何 .c 文件：${MYCODE_DIR}")
endif()

# ---------------------------------------------------------------------------
# 可执行目标
# ---------------------------------------------------------------------------
add_executable(lvgl_simulator
    main.c
    mouse_cursor_icon.c
    ${APP_SOURCES}
    ${LVGL_SOURCES}
    ${LV_DRIVER_SOURCES})

set_target_properties(lvgl_simulator PROPERTIES
    OUTPUT_NAME main
    RUNTIME_OUTPUT_DIRECTORY ${RUNTIME_DIR})

# 头文件搜索顺序有意为之：模拟器自己的配置必须排在固件配置之前。
#   ${SIM_DIR}   -> #include "myCode/..."
#   ${LIB_DIR}   -> #include "lvgl/lvgl.h"、#include "lv_drivers/sdl/sdl.h"
#   ${SIM_DIR}/include -> #include "lv_conf.h" / "lv_drv_conf.h"
target_include_directories(lvgl_simulator PRIVATE
    ${SIM_DIR}/include
    ${SIM_DIR}
    ${LIB_DIR}
    ${LIB_DIR}/lvgl)

target_compile_definitions(lvgl_simulator PRIVATE
    LV_CONF_INCLUDE_SIMPLE
    MYGUI_ASSET_DIR="${MYGUI_ASSET_DIR_POSIX}")

# 优先使用 SDL2 的 imported target。
# 注意：Windows 上必须链接 SDL2main（SDL.h 会把 main 重定义为 SDL_main），
# 且它要排在 SDL2 之前；该包的 SDL2::SDL2main 不会自动带上 SDL2::SDL2。
if(TARGET SDL2::SDL2 AND TARGET SDL2::SDL2main)
    set(SDL2_LINK_LIBS SDL2::SDL2main SDL2::SDL2)
elseif(TARGET SDL2::SDL2)
    set(SDL2_LINK_LIBS SDL2::SDL2)
else()
    include_directories(${SDL2_INCLUDE_DIRS})
    set(SDL2_LINK_LIBS ${SDL2_LIBRARIES})
endif()

target_link_libraries(lvgl_simulator PRIVATE ${SDL2_LINK_LIBS})

# ---------------------------------------------------------------------------
# 把 SDL2.dll 复制到 bin/，这样双击 bin/main.exe 也能直接跑
# （从命令行运行时若 ucrt64/bin 已在 PATH 里，这一步只是多余的保险）
# ---------------------------------------------------------------------------
get_target_property(SDL2_IMPORTED_DLL SDL2::SDL2 IMPORTED_LOCATION_RELEASE)
if(NOT SDL2_IMPORTED_DLL)
    get_target_property(SDL2_IMPORTED_DLL SDL2::SDL2 IMPORTED_LOCATION)
endif()
if(SDL2_IMPORTED_DLL AND EXISTS "${SDL2_IMPORTED_DLL}")
    add_custom_command(TARGET lvgl_simulator POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${SDL2_IMPORTED_DLL}" "${RUNTIME_DIR}/"
        COMMENT "复制 SDL2.dll 到 bin/")
endif()

# ---------------------------------------------------------------------------
# 便捷目标：编译并运行
# ---------------------------------------------------------------------------
add_custom_target(run
    COMMAND $<TARGET_FILE:lvgl_simulator>
    DEPENDS lvgl_simulator
    USES_TERMINAL
    COMMENT "运行 LVGL 模拟器")
