# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html.
# For more examples on how to use CMake, see https://github.com/android/ndk-samples.

# Sets the minimum CMake version required for this project.
cmake_minimum_required(VERSION 3.22.1)

# Declares the project name. The project name can be accessed via ${ PROJECT_NAME},
# Since this is the top level CMakeLists.txt, the project name is also accessible
# with ${CMAKE_PROJECT_NAME} (both CMake variables are in-sync within the top level
# build script scope).
project("lightdigitalhuman")

# Creates and names a library, sets it as either STATIC
# or SHARED, and provides the relative paths to its source code.
# You can define multiple libraries, and CMake builds them for you.
# Gradle automatically packages shared libraries with your APK.
#
# In this top level CMakeLists.txt, ${CMAKE_PROJECT_NAME} is used to define
# the target library name; in the sub-module's CMakeLists.txt, ${PROJECT_NAME}
# is preferred for the same purpose.
#
# In order to load a library into your app from Java/Kotlin, you must call
# System.loadLibrary() and pass the name of the library defined here;
# for GameActivity/NativeActivity derived applications, the same library name must be
# used in the AndroidManifest.xml file.
add_library(${CMAKE_PROJECT_NAME} SHARED
        # List C/C++ source files with relative paths to this CMakeLists.txt.
        third_party/tinygltf/tiny_gltf.cc
        NativeInterface.cpp
        gltfdata/GltfObject.cpp
        utils/utils.cpp
        gltfdata/UserCamera.cpp
        gltfdata/UniformStruct.cpp
        gltfdata/ShaderCache.cpp
        gltfdata/math_utils.cpp
        gltfdata/JsonLoad.cpp
        gltfdata/ImageMimeTypes.cpp
        gltfdata/GltfOpenGLContext.cpp
        engine/Engine.cpp
        gltfdata/GltfUtils.cpp
        gltfdata/GltfTexture.cpp
        gltfdata/GltfState.cpp
        gltfdata/GltfSkin.cpp
        gltfdata/GltfShader.cpp
        gltfdata/GltfScene.cpp
        gltfdata/GltfSampler.cpp
        gltfdata/GltfRenderer.cpp
        gltfdata/ibl/HDRImageLoader.cpp
        gltfdata/GltfPrimitive.cpp
        gltfdata/GltfNode.cpp
        gltfdata/GltfMesh.cpp
        gltfdata/GltfMaterial.cpp
        gltfdata/GltfLight.cpp
        gltfdata/GltfInterpolator.cpp
        gltfdata/GltfImage.cpp
        gltfdata/GltfEnvironment.cpp
        gltfdata/ibl_sampler.cpp
        gltfdata/GltfConverter.cpp
        gltfdata/GltfCamera.cpp
        gltfdata/GltfBufferView.cpp
        gltfdata/GltfBuffer.cpp
        gltfdata/GltfAsset.cpp
        gltfdata/GltfAnimationSampler.cpp
        gltfdata/GltfAnimationChannel.cpp
        gltfdata/GltfAnimation.cpp
        gltfdata/GltfAccessor.cpp
        gltfdata/Gltf.cpp
        gltfdata/EnvironmentRenderer.cpp
        gltfdata/converter/MaterialConverter.cpp
        gltfdata/converter/GltfLoader.cpp
        #        native-lib.cpp
)

set(KTX_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/third_party/ktx)

# 检查KTX库是否存在
if (NOT EXISTS ${KTX_ROOT_DIR}/include/ktx.h)
    message(FATAL_ERROR "KTX库未找到! 请先下载KTX库到 ${KTX_ROOT_DIR}")
endif ()

# 创建导入的KTX库
add_library(ktx_imported STATIC IMPORTED)
set(KTX_LIB_PATH ${KTX_ROOT_DIR}/lib/libktx.so)
# 检查库文件是否存在
if (NOT EXISTS ${KTX_LIB_PATH})
    message(FATAL_ERROR "KTX库文件未找到: ${KTX_LIB_PATH}")
endif ()

# 配置导入库
set_target_properties(ktx_imported PROPERTIES
        IMPORTED_LOCATION ${KTX_LIB_PATH}
        INTERFACE_INCLUDE_DIRECTORIES ${KTX_ROOT_DIR}/include
)

message(STATUS "KTX库配置成功: ${ANDROID_ABI}")
target_include_directories(lightdigitalhuman PRIVATE
        ${KTX_ROOT_DIR}/include
)
# 查找OpenGL ES库
find_library(
        glesv3-lib
        GLESv3
)

find_library(
        egl-lib
        EGL
)
# 添加包含目录
include_directories(third_party/tinygltf)
include_directories(third_party/glm/glm)  # 添加这行
include_directories(third_party/glm)  # GLM库

add_subdirectory(third_party/tinygltf)

# Specifies libraries CMake should link to your target library. You
# can link libraries from various origins, such as libraries defined in this
# build script, prebuilt third-party libraries, or Android system libraries.
target_link_libraries(${CMAKE_PROJECT_NAME}
        ktx_imported
        # List libraries link to the target library
        ${log-lib}
        ${glesv3-lib}      # 添加这行
        ${egl-lib}         # 添加这行
        android
        tinygltf
        log)


# 只在这里定义STB实现
target_compile_definitions(tinygltf PRIVATE
        TINYGLTF_IMPLEMENTATION
        STB_IMAGE_IMPLEMENTATION
        STB_IMAGE_WRITE_IMPLEMENTATION
        STB_IMAGE_RESIZE_IMPLEMENTATION
)
#
## 完整的 Android 架构优化配置

#
#add_definitions(-DGLM_FORCE_INTRINSICS)
#add_definitions(-DGLM_FORCE_ALIGNED)
#add_definitions(-DGLM_FORCE_SINGLE_ONLY)  # 只使用 float
#add_definitions(-DGLM_FORCE_FAST_MATH)
#
#if (CMAKE_ANDROID_ARCH_ABI STREQUAL "arm64-v8a")
#    message(STATUS "ARM64 优化配置")
#    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=armv8-a+simd")
#    # ARM64 的 NEON 默认启用，不需要额外参数
#
#elseif (CMAKE_ANDROID_ARCH_ABI STREQUAL "armeabi-v7a")
#    message(STATUS "ARM32 优化配置")
#    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=armv7-a -mfpu=neon")
#    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mfloat-abi=softfp")
#
#elseif (CMAKE_ANDROID_ARCH_ABI STREQUAL "x86_64")
#    message(STATUS "x86_64 优化配置")
#    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -msse4.1 -mavx")
#    add_definitions(-DGLM_FORCE_SIMD_AVX2)  # 只对 x86 启用 AVX
#
#elseif (CMAKE_ANDROID_ARCH_ABI STREQUAL "x86")
#    message(STATUS "x86 优化配置")
#    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -msse4.1")
#
#endif ()
#
#
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ffast-math")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -funroll-loops")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")
#
#message(STATUS "🔧 最终编译参数: ${CMAKE_CXX_FLAGS}")
