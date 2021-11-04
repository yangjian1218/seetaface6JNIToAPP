cmake_minimum_required(VERSION 3.10.2)

project("seetaface")

# 添加opencv
set(OpenCV_DIR ${CMAKE_SOURCE_DIR}/opencv-mobile-4.5.1-android/sdk/native/jni)
find_package(OpenCV REQUIRED core imgproc)

#添加ncnn
#set(ncnn_DIR ${CMAKE_SOURCE_DIR}/ncnn-20210525-android-vulkan/${ANDROID_ABI}/lib/cmake/ncnn)
#find_package(ncnn REQUIRED)


aux_source_directory(. DIR_SRCS)  #定义DIR_SRCS 为当前目录下所有源代码文件
add_compile_options(-std=c++11 -O2)
# openmp
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fopenmp -static-openmp")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fopenmp -static-openmp")

find_library(log-lib log)
#添加头文件搜索路径
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/seetaface6/include)
#添加动态库搜索文件
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -L${CMAKE_SOURCE_DIR}/seetaface6/jniLibs/${CMAKE_ANDROID_ARCH_ABI}")
link_directories(${CMAKE_CURRENT_SOURCE_DIR}/../jniLibs/${ANDROID_ABI})
add_library( seetaface SHARED ${DIR_SRCS} tools.cpp tools.h)
target_link_libraries(seetaface
#        ncnn
        ${log-lib}
        android
        jnigraphics
        ${android-lib}
        ${OpenCV_LIBS}
#        PoseEstimation600          #提示SeetaPoseEstimation600找不到
        QualityAssessor300          #Seeta
        SeetaAgePredictor600
        SeetaAuthorize
        SeetaEyeStateDetector600    # 600<->200
        SeetaFaceAntiSpoofingX600
        SeetaFaceDetector600
        SeetaFaceLandmarker600
        SeetaGenderPredictor600
        SeetaMaskDetector200
        TenniS
        )