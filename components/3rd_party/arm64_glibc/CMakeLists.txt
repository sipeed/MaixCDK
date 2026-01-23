# 定义库文件根目录（相对当前CMakeLists.txt的路径）
set(arm64_glibc_dir "${CMAKE_CURRENT_SOURCE_DIR}/lib")

# 1. 配置需要添加的所有头文件路径
list(APPEND ADD_INCLUDE 
    "${CMAKE_CURRENT_SOURCE_DIR}/include"          # 根include目录
)

# 2. 自动扫描lib目录下所有的.a静态库和.so动态库
# 扫描所有.a文件
file(GLOB ALL_STATIC_LIBS "${arm64_glibc_dir}/*.a")
# 扫描所有.so文件
file(GLOB ALL_SHARED_LIBS "${arm64_glibc_dir}/*.so")

# 3. 将扫描到的库添加到MaixCDK的组件注册列表中
list(APPEND ADD_STATIC_LIB ${ALL_STATIC_LIBS})  # 静态库
list(APPEND ADD_SHARED_LIB ${ALL_SHARED_LIBS})  # 动态库


# 4. 注册组件（MaixCDK核心接口，自动处理头文件和库链接）
register_component()

