// Created by jerry on 2021/9/10.
#include <iostream>
#include <string>
#include <vector>
#include <android/asset_manager_jni.h>
#include <seeta/FaceDetector.h>
#include <seeta/FaceLandmarker.h>
#include <seeta/FaceAntiSpoofing.h>
#include <seeta/Common/Struct.h>
#include <seeta/MaskDetector.h>         //口罩检测
#include <seeta/EyeStateDetector.h>     //眼睛状态检测
#include <seeta/AgePredictor.h>         //年龄检测
#include <seeta/GenderPredictor.h>      //性别检测
#include <seeta/QualityStructure.h>     //遮挡评估
#include <seeta/QualityOfBrightness.h>  //亮度评估
#include <seeta/QualityOfResolution.h>  //分辨率评估
#include <seeta/QualityOfIntegrity.h>   //完整性评估
#include <seeta/QualityOfClarity.h>     //清晰度检测(传统)
#include <seeta/QualityOfPose.h>		//姿态评估(传统)
//#include <seeta/QualityOfLBN.h>         //清晰度评估(深度)
//#include <seeta/QualityOfPoseEx.h>      //姿态评估(深度)
using namespace std;
#define LOG_TAG "YLZ_seetaface"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__) // 定义LOGI类型
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__) // 定义LOGE类型

//人脸检测器
seeta::FaceDetector *new_fd(string path) {
    seeta::ModelSetting setting;
    setting.append(path+"/face_detector.csta");
    return new seeta::FaceDetector(setting);
}

// 特征点5 检测器
seeta::FaceLandmarker *new_ld5(string path){
    seeta::ModelSetting setting;
    setting.append(path+"/face_landmarker_pts5.csta");
    return new seeta::FaceLandmarker(setting);
}

// 特征点68 检测器
seeta::FaceLandmarker *new_ld68(string path){
    seeta::ModelSetting setting;
    setting.append(path+"/face_landmarker_pts68.csta");
    return new seeta::FaceLandmarker(setting);
}
// 活体检测
seeta::FaceAntiSpoofing *new_live(string path){
    seeta::ModelSetting setting;
    setting.append(path+"/fas_first.csta");
    setting.append(path+"/fas_second.csta");
    return new seeta::FaceAntiSpoofing(setting);
}
////五官遮挡检测
seeta::FaceLandmarker *new_facemask(string path){
    seeta::ModelSetting setting;
    setting.append(path+"/face_landmarker_mask_pts5.csta");
    return new seeta::FaceLandmarker(setting);
}
////性别检测
seeta::GenderPredictor* new_genderP(string path){
    seeta::ModelSetting setting;
    setting.append(path+"/gender_predictor.csta");
    return new seeta::GenderPredictor(setting);
};
////年龄检测
seeta::AgePredictor* new_ageP(string path) {
    seeta::ModelSetting setting;
    setting.append(path+"/age_predictor.csta");
    return new seeta::AgePredictor(setting);
}
//////眼睛状态检测
seeta::EyeStateDetector* new_eyeD(string path){
    seeta::ModelSetting setting;
    setting.append(path+"/eye_state.csta");
    return new seeta::EyeStateDetector(setting);
}
////口罩检测
seeta::MaskDetector* new_maskD(string path){
    seeta::ModelSetting setting;
    setting.append(path+"/mask_detector.csta");
    return new seeta::MaskDetector(setting);
}


class Seetaface
{
public:
    Seetaface(){

    }
    // 模型加载
    void Init_face(string path){
        faceDetector = new_fd(path);                        //人脸框检测
    }
    void Init_land5(string path){
        landDetector5 = new_ld5(path);                      //5特征点检测
    }
    void Init_land68(string path){
        landDetector68 = new_ld68(path);                    //68特征点检测
    }
    void Init_live(string path){
        liveDetector = new_live(path);                      //活体检测
    }
    void Init_faceMask(string path){
        faceMaskDetector = new_facemask(path);              //五官遮挡检测
    }
    void Init_age(string path){
        agePredictor = new_ageP(path);                      //年龄检测
    }
    void Init_gender(string path){
        genderPredictor = new_genderP(path);                //性别检测
    }
    void Init_mask(string path){
        maskDetector = new_maskD(path);                     //口罩检测
    }
    void Init_eyeState(string path){
        eyeStateDetector = new_eyeD(path);                  //眼睛状态检测
    }
    void Init_clarity(){
        qualityClarity = new seeta::QualityOfClarity();     //清晰度评估(传统)
    }
    void Init_bright(){
        qualityBright = new seeta::QualityOfBrightness();   //明亮度评估(传统)
    }
    void Init_resolution(){
        qualityResolution = new seeta::QualityOfResolution(); //分辨率评估(传统)
    }
    void Init_pose(){
        qualityPose = new seeta::QualityOfPose();           //姿态评估(传统)
    }
    void Init_integrity(){
        qualityIntegrity = new seeta::QualityOfIntegrity(); //人脸完整性评估
    }
    //活体清晰度跟活体阈值修改
    void Init_liveThreshold(float clarity,float threshold){
        liveDetector->SetThreshold(clarity,threshold);
    }

    string Init(string path, string func){
        if(func == "landmark5"){
            Init_land5(path);                  //成功
            return "ok";
        }
        if(func == "landmark68"){
            Init_land68(path);                  //成功
            return "ok";
        }
        else if(func == "live") {
            Init_live(path);                    //成功
            return "ok";
        }
        else if(func == "faceMask") {
            Init_faceMask(path);                //成功
            return "ok";
        }
        else if(func == "age") {
            Init_age(path);                     //成功
            return "ok";
        }
        else if(func == "gender") {
            Init_gender(path);                  //成功
            return "ok";
        }
        else if(func == "mask") {
            Init_mask(path);                    //成功
            return "ok";
        }
        else if(func == "eyeState") {
            Init_eyeState(path);                //成功
            return "ok";
        }
        else if(func =="clarity") {
            Init_clarity();                     //成功
            return "ok";
        }
        else if(func == "bright"){
            Init_bright();                      //成功
            return "ok";
        }
        else if(func == "resolution"){
            Init_resolution();                  //成功
            return "ok";
        }
        else if(func == "pose"){
            Init_pose();                        //成功
            return "ok";
        }
        else if(func == "integrity"){
            Init_integrity();                  //成功
            return "ok";
        }
        else
            return func;

    }

    // 人脸检测
    SeetaFaceInfoArray detect_face(const SeetaImageData &simage){
        SeetaFaceInfoArray faces = faceDetector->detect(simage);
        return faces;
    }

    //5特征点检测
    std::vector<SeetaPointF> detect_land5(const SeetaImageData &simage,const SeetaRect &box){
        std::vector<SeetaPointF> points = landDetector5->mark(simage, box);
        return points;
    }

    //68特征点检测
    std::vector<SeetaPointF> detect_land68(const SeetaImageData &simage,const SeetaRect &box){
        std::vector<SeetaPointF> points = landDetector68->mark(simage, box);
        return points;
    }

    //活体检测
    float detect_live(const SeetaImageData &simage,const SeetaRect &box,const std::vector<SeetaPointF> &points5){
        LOGI("标记1");
        auto status = liveDetector->Predict(simage, box, points5.data());
        float clarity_value, reality_value;
        liveDetector->GetPreFrameScore(&clarity_value, &reality_value);
        return reality_value;
    }
    //五官遮挡检测
    int* detect_facemask(const SeetaImageData& simage,const SeetaRect& box){
        int masks[4];
        auto point_masks = faceMaskDetector->mark_v2(simage, box);
        for (int i = 0; i < point_masks.size()-2; i++) {
            masks[i]=point_masks[i].mask;
        }
        if(point_masks[3].mask==0 && point_masks[4].mask==0){
            masks[3]=0;
        }
        else{
            masks[3]=1;
        }
        return masks;
    }
    //年龄预测
    int predict_age(const SeetaImageData& simage,const std::vector<SeetaPointF>& points5){
        int age = 0;
        agePredictor->PredictAgeWithCrop(simage, points5.data(), age);
        return age;
    }
    //性别预测
    char* predict_gender(const SeetaImageData& simage,const std::vector<SeetaPointF>& points5){
        seeta::GenderPredictor::GENDER gender;
        char* genderOut="male";
        genderPredictor->PredictGenderWithCrop(simage, points5.data(), gender);
        if(gender == seeta::GenderPredictor::FEMALE)
            genderOut="female";
//        gender == seeta::GenderPredictor::FEMALE ? "female" : "male";
        return genderOut;
    }
    //口罩检测
    bool detect_mask(const SeetaImageData &simage,const SeetaRect &box){
        float score = 0;
        bool mask = maskDetector->detect(simage, box, &score);
        return mask;
    }
    //眼睛状态检测
    vector<string> detect_eyestate(const SeetaImageData &simage,const std::vector<SeetaPointF>& points5){
        seeta::EyeStateDetector::EYE_STATE left_eye, right_eye;
        vector<string> EYE_STATE_STR{ "close", "open", "random", "unknown" };
        eyeStateDetector->Detect(simage, points5.data(), left_eye, right_eye);
        vector<string> eyestate;                                 //以字符串数组传递
        eyestate.push_back(EYE_STATE_STR[left_eye]);
        eyestate.push_back(EYE_STATE_STR[right_eye]);
//        int eyestate[2] = {left_eye,right_eye};           //以int数组传递

        return eyestate;
    }
    //清晰度评估(传统)
    char* evaluate_clarity(const SeetaImageData &simage,const SeetaRect &box,const std::vector<SeetaPointF>&points5){
        vector<char*> level_string = { "LOW", "MEDIUM", "HIGH" };
        seeta::QualityResult result = qualityClarity->check(simage, box, points5.data(), int(points5.size()));
        char* level=level_string[result.level];
        return level;
    }
    //亮度度评估(传统)
    char* evaluate_bright(const SeetaImageData &simage,const SeetaRect &box,const std::vector<SeetaPointF>&points5){
        vector<char*> level_string = { "LOW", "MEDIUM", "HIGH" };
        seeta::QualityResult result = qualityBright->check(simage, box, points5.data(), int(points5.size()));
        char* level=level_string[result.level];
        return level;
    }
    //分辨率评估
    char* evaluate_resolution(const SeetaImageData &simage,const SeetaRect &box,const std::vector<SeetaPointF>&points5){
        vector<char*> level_string = { "LOW", "MEDIUM", "HIGH" };
        seeta::QualityResult result = qualityResolution->check(simage, box, points5.data(), int(points5.size()));
        char* level=level_string[result.level];
        return level;
    }
    //姿态评估
    char* evaluate_pose(const SeetaImageData &simage,const SeetaRect &box,const std::vector<SeetaPointF>&points5){
        vector<char*> level_string = { "LOW", "MEDIUM", "HIGH" };
        seeta::QualityResult result = qualityPose->check(simage, box, points5.data(), int(points5.size()));
        char* level=level_string[result.level];
        return level;
    }
    //完整性评估
    char* evaluate_integrity(const SeetaImageData &simage,const SeetaRect &box,const std::vector<SeetaPointF>&points5){
        vector<char*> level_string = { "LOW", "MEDIUM", "HIGH" };
        seeta::QualityResult result = qualityIntegrity->check(simage, box, points5.data(), int(points5.size()));
        char* level=level_string[result.level];
        return level;
    }
private:
    seeta::FaceDetector *faceDetector;              //人脸框
    seeta::FaceLandmarker *landDetector5;           //5特征点
    seeta::FaceLandmarker *landDetector68;          //68特征点
    seeta::FaceAntiSpoofing *liveDetector;          //活体检测
    seeta::FaceLandmarker *faceMaskDetector;        //五官遮挡检测
    seeta::AgePredictor *agePredictor;              //年龄预测
    seeta::GenderPredictor*genderPredictor;         //性别评估
    seeta::MaskDetector* maskDetector;              //口罩检测
    seeta::EyeStateDetector* eyeStateDetector;      //眼睛状态检测
    seeta::QualityRule* qualityClarity;             //清晰度评估(传统)
    seeta::QualityRule* qualityBright;              //明亮度评估(传统)
    seeta::QualityRule* qualityResolution;          //分辨率评估
    seeta::QualityOfPose* qualityPose;              //姿态评估(传统)
    seeta::QualityOfIntegrity* qualityIntegrity;    //完整性评估

};