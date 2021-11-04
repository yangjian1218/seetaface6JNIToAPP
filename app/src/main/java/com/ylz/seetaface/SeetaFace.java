package com.ylz.seetaface;

import android.content.res.AssetManager;
import android.graphics.Bitmap;

public class SeetaFace {
    static{
        System.loadLibrary("seetaface");

    }
    public native boolean loadModel(String datapath,String[] functions);    //加载模型
    public native void InitLiveThreshold(float clarity,float reality);      //初始化活体检测的阈值
    public native int[] detectFace(Bitmap input);                           //人脸检测
    public native boolean landmark(int mark_num);                           //特征点检测并画图
    public native void detectDraw(boolean drawOfbox,boolean drawOfpts5,boolean drawOfpts68,Bitmap output);  //人脸框,pts5,pts68绘制
    public native float liveDetect();                                       //活体检测
    public native String[] detectEyeState();                                 //人眼状态检测
    public native boolean detectMask();                                     //口罩检测
    public native int[] detectFaceMask();                                     //五官遮挡检测
    public native String detectGender();                                    //性别检测
    public native int detectAge();                                           //年龄检测
    public native String evaluateIntergrity();                               //人脸完整度评估
    public native String evaluateClarity();                                  //人脸清晰度评估
    public native String evaluateBright();                                   //人脸明亮度评估
    public native String evaluateResolution();                               //人脸分辨率评估
    public native String evaluatePose();                                    //人脸姿态评估
}
