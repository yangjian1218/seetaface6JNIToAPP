#include <jni.h>
#include <string>
#include <android/asset_manager_jni.h>
#include <android/bitmap.h>
#include <android/log.h>
#include <opencv2/opencv.hpp>
#include <seeta/FaceDetector.h>
#include <seeta/FaceLandmarker.h>
#include <seeta/FaceAntiSpoofing.h>
#include <seeta/Common/Struct.h>
#include "tools.h"
#include "seetaface.hpp"


using namespace seeta::SEETA_FACE_DETECTOR_NAMESPACE_VERSION;

using namespace std;

#define ASSERT(status, ret)     if (!(status)) { return ret; }
#define ASSERT_FALSE(status)    ASSERT(status, false)

/**
 * @param env
 * @param obj_bitmap 安卓传进来的bitmap图片
 * @param matrix 需要转为cv::Mat 图片
 * @return 是否转换成功
 */
static cv::Mat image;
static cv::Mat cimage;
static SeetaImageData simage;
static SeetaRect sbox;
static std::vector<SeetaPointF> points5;
static std::vector<SeetaPointF> points68;
static Seetaface seetaNet;
static int init=1;

bool BitmapToMatrix(JNIEnv * env, jobject obj_bitmap, cv::Mat & matrix) {
    void * bitmapPixels;                                            // 保存图片像素数据
    AndroidBitmapInfo bitmapInfo;                                   // 保存图片参数

    ASSERT_FALSE( AndroidBitmap_getInfo(env, obj_bitmap, &bitmapInfo) >= 0);        // 获取图片参数
    ASSERT_FALSE( bitmapInfo.format == ANDROID_BITMAP_FORMAT_RGBA_8888
                  || bitmapInfo.format == ANDROID_BITMAP_FORMAT_RGB_565 );          // 只支持 ARGB_8888 和 RGB_565
    ASSERT_FALSE( AndroidBitmap_lockPixels(env, obj_bitmap, &bitmapPixels) >= 0 );  // 获取图片像素（锁定内存块）
    ASSERT_FALSE( bitmapPixels );

    if (bitmapInfo.format == ANDROID_BITMAP_FORMAT_RGBA_8888) {
        cv::Mat tmp(bitmapInfo.height, bitmapInfo.width, CV_8UC4, bitmapPixels);    // 建立临时 mat
        tmp.copyTo(matrix);                                                         // 拷贝到目标 matrix
    } else {
        cv::Mat tmp(bitmapInfo.height, bitmapInfo.width, CV_8UC2, bitmapPixels);
        cv::cvtColor(tmp, matrix, cv::COLOR_BGR5652RGB);
    }
    AndroidBitmap_unlockPixels(env, obj_bitmap);            // 解锁
    return true;
}

/**
 * @param env
 * @param matrix cv::Mat图片
 * @param obj_bitmap 转成后的bitmap图片,之后要传给手机
 * @return 是否转化成功
 */
bool MatrixToBitmap(JNIEnv * env, cv::Mat & matrix, jobject obj_bitmap) {
    void * bitmapPixels;                                            // 保存图片像素数据
    AndroidBitmapInfo bitmapInfo;                                   // 保存图片参数

    ASSERT_FALSE( AndroidBitmap_getInfo(env, obj_bitmap, &bitmapInfo) >= 0);        // 获取图片参数
    ASSERT_FALSE( bitmapInfo.format == ANDROID_BITMAP_FORMAT_RGBA_8888
                  || bitmapInfo.format == ANDROID_BITMAP_FORMAT_RGB_565 );          // 只支持 ARGB_8888 和 RGB_565
    ASSERT_FALSE( matrix.dims == 2
                  && bitmapInfo.height == (uint32_t)matrix.rows
                  && bitmapInfo.width == (uint32_t)matrix.cols );                   // 必须是 2 维矩阵，长宽一致
    ASSERT_FALSE( matrix.type() == CV_8UC1 || matrix.type() == CV_8UC3 || matrix.type() == CV_8UC4 );
    ASSERT_FALSE( AndroidBitmap_lockPixels(env, obj_bitmap, &bitmapPixels) >= 0 );  // 获取图片像素（锁定内存块）
    ASSERT_FALSE( bitmapPixels );

    if (bitmapInfo.format == ANDROID_BITMAP_FORMAT_RGBA_8888) {
        cv::Mat tmp(bitmapInfo.height, bitmapInfo.width, CV_8UC4, bitmapPixels);
        switch (matrix.type()) {
            case CV_8UC1:   cv::cvtColor(matrix, tmp, cv::COLOR_GRAY2RGBA);     break;
            case CV_8UC3:   cv::cvtColor(matrix, tmp, cv::COLOR_RGB2RGBA);      break;
            case CV_8UC4:   matrix.copyTo(tmp);                                 break;
            default:        AndroidBitmap_unlockPixels(env, obj_bitmap);        return false;
        }
    } else {
        cv::Mat tmp(bitmapInfo.height, bitmapInfo.width, CV_8UC2, bitmapPixels);
        switch (matrix.type()) {
            case CV_8UC1:   cv::cvtColor(matrix, tmp, cv::COLOR_GRAY2BGR565);   break;
            case CV_8UC3:   cv::cvtColor(matrix, tmp, cv::COLOR_RGB2BGR565);    break;
            case CV_8UC4:   cv::cvtColor(matrix, tmp, cv::COLOR_RGBA2BGR565);   break;
            default:        AndroidBitmap_unlockPixels(env, obj_bitmap);        return false;
        }
    }
    AndroidBitmap_unlockPixels(env, obj_bitmap);                // 解锁
    return true;
}


extern "C" JNIEXPORT jboolean
Java_com_ylz_seetaface_SeetaFace_loadModel(JNIEnv *env,jobject thiz,jstring cstapath,jobjectArray string_array) {
    //模型初始化
    const char *modelpath = env->GetStringUTFChars(cstapath, 0); //jstring 转char*
    seetaNet.Init_face(modelpath);  //加载模型
    jint strlength = env->GetArrayLength(string_array);   //string_array 为要加载模型名称的字符串数组
    for (int i = 0; i < strlength; ++i) {
        jstring str = static_cast<jstring>(env->GetObjectArrayElement(string_array, i));
        const char* func = env->GetStringUTFChars(str,NULL);
        LOGI("获取java的参数:%s",func);
        string res = seetaNet.Init(modelpath,func);
        if(res !="ok"){
            LOGE("输入模型:%s名称不对",func);
            return JNI_FALSE;
        }
        else{
            LOGI("%s初始化成功",func);
        }
        env->ReleaseStringUTFChars(str,func);
    }

    env->ReleaseStringUTFChars(cstapath, modelpath);
    return JNI_TRUE;
}

extern "C" JNIEXPORT void JNICALL
Java_com_ylz_seetaface_SeetaFace_InitLiveThreshold(JNIEnv *env,jobject thiz,jfloat clarity,jfloat reality){
    seetaNet.Init_liveThreshold(clarity,reality);
    LOGI("活体检测初始化成功,clarity=%f,reality=%f",clarity,reality);
}

extern "C" JNIEXPORT jintArray JNICALL
Java_com_ylz_seetaface_SeetaFace_detectFace(JNIEnv *env, jobject thiz, jobject input) {

    bool bit_cv = BitmapToMatrix(env,input,image);  //bitmap转cvMat,格式还为RGBA
    cv::cvtColor(image,cimage,COLOR_RGBA2BGR);  //
    //cv图片转Seeta图片
    simage.width = cimage.cols;
    simage.height = cimage.rows;
    simage.channels = cimage.channels();
    simage.data = cimage.data;
    SeetaFaceInfoArray faces = seetaNet.detect_face(simage); //调用人脸检测
    if (faces.size==0){
        return nullptr;
    }
    if(faces.size>0) {
        auto face = faces.data[0];
        sbox = face.pos;
//        float score = face.score;@
        int x1 = int(sbox.x), y1 = int(sbox.y), width = int(sbox.width), height = int(sbox.height);
        int bbox[4] = {x1, y1, width, height};
        jintArray boxInfo = env->NewIntArray(4);
        env->SetIntArrayRegion(boxInfo, 0, 4, bbox);
        return boxInfo;
    }
}


extern "C" JNIEXPORT jboolean JNICALL
Java_com_ylz_seetaface_SeetaFace_landmark(JNIEnv *env, jobject thiz, jint mark_num) {
    if(mark_num==5)
        points5 = seetaNet.detect_land5(simage, sbox);
    else if(mark_num==68){
        points68 = seetaNet.detect_land68(simage, sbox);
    }
    else{
        return JNI_FALSE;
    }

    return JNI_TRUE;
}

extern "C" JNIEXPORT void JNICALL
Java_com_ylz_seetaface_SeetaFace_detectDraw(JNIEnv *env, jobject thiz,jboolean drawOfbox,jboolean drawOfpts5,jboolean drawOfpts68,jobject bitmapOut) {
    cv::Mat imageOut;
    cimage.copyTo(imageOut);
    if(drawOfbox){
        draw_box(imageOut, sbox);  //人脸框与五官画图

    }
    if(drawOfpts5){
        draw_points(imageOut, points5);  //五官画图
    }
    if(drawOfpts68){
        draw_points(imageOut, points68);  //五官画图
    }
    cv::cvtColor(imageOut,imageOut,COLOR_BGR2RGBA);
    bool cv_bit = MatrixToBitmap(env,imageOut,bitmapOut);
}
//todo 活体检测
extern "C" JNIEXPORT jfloat JNICALL
Java_com_ylz_seetaface_SeetaFace_liveDetect(JNIEnv *env, jobject thiz) {
        float score = seetaNet.detect_live(simage,sbox,points5);
        return (jfloat)score;
}

//todo 眼睛状态检测
extern "C" JNIEXPORT jobjectArray JNICALL
Java_com_ylz_seetaface_SeetaFace_detectEyeState(JNIEnv *env, jobject thiz) {
    vector<string> eyeState = seetaNet.detect_eyestate(simage,points5);
    const char* leftState = eyeState[0].c_str();
    const char* rightState = eyeState[1].data();
    const char* State[]={leftState,rightState};
    jstring str;
    jobjectArray args=0;
    jsize  len = eyeState.size();
    args =(env)->NewObjectArray(len,(env)->FindClass("java/lang/String"),0);
    for(int i=0;i<len;i++){
        str=(env)->NewStringUTF(State[i]);
        (env)->SetObjectArrayElement(args,i,str);
    }
    return args;

//    jintArray returnArray = env->NewIntArray(2);
//    env->SetIntArrayRegion(returnArray,0,2,eyeState);
//    return returnArray;

}
//todo 戴口罩检测
extern "C" JNIEXPORT jboolean JNICALL
Java_com_ylz_seetaface_SeetaFace_detectMask(JNIEnv *env, jobject thiz) {
    bool isMask = seetaNet.detect_mask(simage,sbox);
    return (jboolean)isMask;
}

//todo 五官遮挡检测
extern "C" JNIEXPORT jintArray JNICALL
Java_com_ylz_seetaface_SeetaFace_detectFaceMask(JNIEnv *env, jobject thiz) {
    int *isfaceMask= seetaNet.detect_facemask(simage,sbox);
    jintArray returnArray = env->NewIntArray(4);
    env->SetIntArrayRegion(returnArray,0,4,isfaceMask);
    return returnArray;
}
//todo 性别预测
extern "C" JNIEXPORT jstring JNICALL
Java_com_ylz_seetaface_SeetaFace_detectGender(JNIEnv *env, jobject thiz) {
    char* gender= seetaNet.predict_gender(simage,points5);
    return env->NewStringUTF(gender);
}
//todo 年龄预测
extern "C" JNIEXPORT jint JNICALL
Java_com_ylz_seetaface_SeetaFace_detectAge(JNIEnv *env, jobject thiz) {
    int age= seetaNet.predict_age(simage,points5);
    return (jint)age;
}
//todo 人脸完整度评估
extern "C" JNIEXPORT jstring JNICALL
Java_com_ylz_seetaface_SeetaFace_evaluateIntergrity(JNIEnv *env, jobject thiz) {
    char* gender= seetaNet.evaluate_integrity(simage,sbox,points5);
    return env->NewStringUTF(gender);
}
//todo 人脸清晰度评估
extern "C" JNIEXPORT jstring JNICALL
Java_com_ylz_seetaface_SeetaFace_evaluateClarity(JNIEnv *env, jobject thiz) {
    char* clarity= seetaNet.evaluate_clarity(simage,sbox,points5);
    return env->NewStringUTF(clarity);
}
//todo 人脸明亮度评估
extern "C" JNIEXPORT jstring JNICALL
Java_com_ylz_seetaface_SeetaFace_evaluateBright(JNIEnv *env, jobject thiz) {
    char* bright= seetaNet.evaluate_bright(simage,sbox,points5);
    return env->NewStringUTF(bright);
}
//todo 人脸分辨率评估
extern "C" JNIEXPORT jstring JNICALL
Java_com_ylz_seetaface_SeetaFace_evaluateResolution(JNIEnv *env, jobject thiz) {
    char* resolution= seetaNet.evaluate_resolution(simage,sbox,points5);
    return env->NewStringUTF(resolution);
}
//todo 人脸姿态评估
extern "C" JNIEXPORT jstring JNICALL
Java_com_ylz_seetaface_SeetaFace_evaluatePose(JNIEnv *env, jobject thiz) {
    char* pose= seetaNet.evaluate_pose(simage,sbox,points5);
    return env->NewStringUTF(pose);
}