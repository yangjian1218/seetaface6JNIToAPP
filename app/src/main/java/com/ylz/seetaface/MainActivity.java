package com.ylz.seetaface;

import androidx.appcompat.app.AppCompatActivity;

import android.content.Context;
import android.content.Intent;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Canvas;  //光栅画布
import android.graphics.Color;  //颜色
import android.graphics.Paint;
import android.net.Uri;  //请求
import android.os.Bundle;   //
import android.util.Log;    //日志
import android.view.View;     //视图
import android.widget.Button;     //按钮
import android.widget.ImageView;   //图片显示
import android.widget.Toast;   //显式页面
import android.widget.TextView;
import java.io.FileNotFoundException;   //异常
import android.content.res.AssetManager;

import com.ylz.seetaface.databinding.ActivityMainBinding;

import java.io.BufferedInputStream;
import java.io.FileOutputStream;
import java.io.File;
import java.io.IOException;
import java.io.InputStream;
import java.util.ArrayList;
import java.util.List;
import java.util.*;



public class MainActivity extends AppCompatActivity {

    private static final int SELECT_IMAGE = 1;

    private ImageView imageView;  //初始化一个图片展示控件
    private Bitmap bitmap = null;   //初始化一个图片
    private Bitmap bitmapOut=null;  //初始化一个输出图片
    private Bitmap yourSelectedImage = null;   //初始化一个图片
    private SeetaFace seetaFace=new SeetaFace(); //初始化一个SeetaFace
    private TextView textview;    //初始化显示框
    private ActivityMainBinding binding;
    private int[] ret_box;

    private static List<String> lst = new ArrayList<String>();  //初始化模型列表
    static {
        lst.add("age_predictor.csta");
        lst.add("eye_state.csta");
        lst.add("face_detector.csta");
        lst.add("face_landmarker_mask_pts5.csta");
        lst.add("face_landmarker_pts5.csta");
        lst.add("face_landmarker_pts68.csta");
        lst.add("fas_first.csta");
        lst.add("fas_second.csta");
        lst.add("gender_predictor.csta");
        lst.add("mask_detector.csta");
        lst.add("post_estimation.csta");
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        for(int i=0;i<lst.size();i++){
            boolean mkdir_model = copyAssetAndWrite(lst.get(i));  //放在了缓存区
            }
        String dataPath = getCacheDir().getPath();
        Log.e("文件目录地址为:", dataPath);
        String[] functions={"landmark5","landmark68","live","age","bright","clarity","eyeState","faceMask","mask","gender","resolution","pose","integrity"};
        boolean ret_init = seetaFace.loadModel(dataPath,functions);
        if (ret_init){
            Log.i("加载模型:", "成功");   //+""即把bool转为字符串
            seetaFace.InitLiveThreshold(0.4f,0.88f);
        }
        else
            Log.e("加载模型:", "失败");   //+""即把bool转为字符串

        imageView = (ImageView) findViewById(R.id.iv_image);  //绑定图片.可以不需要(ImageView)
        textview = findViewById(R.id.output_text);

        //TODO 选取图片
        Button btn_image = findViewById(R.id.btn_select_image);   //绑定选择图片按钮
        btn_image.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {            //绑定按钮的事件
                Intent i = new Intent(Intent.ACTION_PICK);   //
                i.setType("image/*"); //设置图片类型
                startActivityForResult(i, SELECT_IMAGE);  //跳转界面,执行新的操作, SELECT_IMAGE代码在下面
            }
        });

        //TODO 人脸检测+5特征点检测
        Button btn_detect_face = findViewById(R.id.btn_detect_face);  //绑定人脸检测按钮
        btn_detect_face.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                if (yourSelectedImage == null) return;
                bitmapOut = Bitmap.createBitmap(yourSelectedImage.getWidth(), yourSelectedImage.getHeight(), Bitmap.Config.ARGB_8888); //空白图像
                ret_box = seetaFace.detectFace(yourSelectedImage);
                boolean ret5=seetaFace.landmark(5); //特征点检测
                if (ret_box == null) {
                    Toast.makeText(getApplicationContext(), "未检测到人脸", Toast.LENGTH_SHORT).show();   //3.5s显示text
                    return;
                }
                boolean ret68=seetaFace.landmark(68); //特征点检测
                String box = String.valueOf("("+ret_box[0])+", "+String.valueOf(ret_box[1])+")"+", W:"+
                        String.valueOf(ret_box[2])+", H:"+String.valueOf(ret_box[3]);
                Log.v("人脸框信息:", box);  //Log.v为记录VERBOSE,什么信息都会输出,啰嗦的意思,
                textview.setText(" 人脸信息:"+box);
                seetaFace.detectDraw(true,true,false,bitmapOut);  //人脸框画图
                showObjects(bitmapOut);   //调用展示方法

            }
        });
        //todo 68特征点检测
        Button btn_landmark68 = findViewById(R.id.btn_landmark68);  //绑定特征点检测按钮
        btn_landmark68.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                if (yourSelectedImage == null) return;
                if (ret_box == null) {
                    Toast.makeText(getApplicationContext(), "未检测到人脸", Toast.LENGTH_SHORT).show();   //3.5s显示text
                    return;
                }
                boolean ret68=seetaFace.landmark(68); //特征点检测
                seetaFace.detectDraw(false,false,true,bitmapOut);  //人脸框画图
                showObjects(bitmapOut);   //调用展示方法
                textview.setText(" 人脸68特征点检测: "+String.valueOf(ret68));
            }
        });
        //TODO 活体检测
        Button btn_live_face = findViewById(R.id.btn_live_face);
        btn_live_face.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                if (yourSelectedImage == null) return;
                if (ret_box == null) {
                    textview.setText(" -1(未检测到人脸)");
                    return;
                }
                float ret_live = seetaFace.liveDetect();
                float live_threshold = 0.88f;
                String ret_anti=" 攻击图片";
                if(ret_live>live_threshold){
                    ret_anti=" 真人";
                }
                textview.setText(ret_anti + "   score=" + String.valueOf(ret_live));
            }
        });
        //TODO 眼睛状态检测
        Button btn_eyeState = findViewById(R.id.btn_eyeState);
        btn_eyeState.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {

                if (yourSelectedImage == null) return;
                if (ret_box == null) {
                    textview.setText(" -1(未检测到人脸)");
                    return;
                }
                 String[] ret_eyeState = seetaFace.detectEyeState();
                textview.setText(" 左眼:"+ret_eyeState[0]+"   右眼:"+ret_eyeState[1]);
            }
        });
        //TODO 口罩状态检测
        Button btn_mask = findViewById(R.id.btn_mask);
        btn_mask.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {

                if (yourSelectedImage == null) return;
                if (ret_box == null) {
                    textview.setText(" -1(未检测到人脸)");
                    return;
                }
                boolean ret_mask = seetaFace.detectMask();
                textview.setText(" 是否戴口罩:"+String.valueOf(ret_mask));
            }
        });
        //TODO 五官遮挡检测
        Button btn_faceMask = findViewById(R.id.btn_faceMask);
        btn_faceMask.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                if (yourSelectedImage == null) return;
                if (ret_box == null) {
                    textview.setText(" -1(未检测到人脸)");
                    return;
                }
                int[] ret_mask = seetaFace.detectFaceMask();
                String face[]={" leftEye "," rightEye "," nose "," mouth "};
                String facemask="";
                String temp;
                for(int i=0;i<ret_mask.length;i++){
                    if(ret_mask[i]==1){
                        temp = facemask;
                        facemask = temp+face[i];
                    }
                }
                textview.setText(" 遮挡部分:"+facemask);
            }
        });
        //TODO 性别/年龄检测
        Button btn_gender_age = findViewById(R.id.btn_gender_age);
        btn_gender_age.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                if (yourSelectedImage == null) return;
                if (ret_box == null) {
                    textview.setText(" -1(未检测到人脸)");
                    return;
                }
                String ret_gender = seetaFace.detectGender();  //性别检测
                int ret_age = seetaFace.detectAge();     //年龄检测

                textview.setText(" 性别:"+ret_gender+"   年龄:"+String.valueOf(ret_age));
            }
        });
        //TODO 完整性评估
        Button btn_qualityIntegrity = findViewById(R.id.btn_qualityIntegrity);
        btn_qualityIntegrity.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                if (yourSelectedImage == null) return;
                if (ret_box == null) {
                    textview.setText(" -1(未检测到人脸)");
                    return;
                }
                String ret_integrity = seetaFace.evaluateIntergrity();  // 完整性评估
                textview.setText(" 人脸完整度质量:"+ret_integrity);
            }
        });
        //TODO 清晰/亮度评估
        Button btn_clarity_bright = findViewById(R.id.btn_clarity_bright);
        btn_clarity_bright.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                if (yourSelectedImage == null) return;
                if (ret_box == null) {
                    textview.setText(" -1(未检测到人脸)");
                    return;
                }
                String ret_clarity = seetaFace.evaluateClarity();     //清晰度评估
                String ret_bright = seetaFace.evaluateBright();     //清晰度评估
                textview.setText(" 清晰度:"+ret_clarity+"   明亮度:"+ret_bright);
            }
        });
        //TODO 分辨率评估
        Button btn_resolution = findViewById(R.id.btn_resolution);
        btn_resolution.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                if (yourSelectedImage == null) return;
                if (ret_box == null) {
                    textview.setText(" -1(未检测到人脸)");
                    return;
                }
                String ret_resolution = seetaFace.evaluateResolution();  // 分辨率评估
                textview.setText(" 分辨率质量:"+ret_resolution);
            }
        });
        //TODO 人脸姿态评估
        Button btn_qulityPose = findViewById(R.id.btn_qulityPose);
        btn_qulityPose.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                if (yourSelectedImage == null) return;
                if (ret_box == null) {
                    textview.setText(" -1(未检测到人脸)");
                    return;
                }
                String ret_pose = seetaFace.evaluatePose();
                textview.setText(" 分辨率质量:"+ret_pose);
            }
        });
    }


    //TODO 从相册中选取图片
    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);
        if (resultCode == RESULT_OK && null != data) {
            Uri selectedImage = data.getData();
            try {
                if (requestCode == SELECT_IMAGE) {
                    bitmap = decodeUri(selectedImage);
                    yourSelectedImage = bitmap.copy(Bitmap.Config.ARGB_8888, true);
                    imageView.setImageBitmap(bitmap);
                }
            } catch (FileNotFoundException e) {
                Log.e("MainActivity", "FileNotFoundException");
                return;
            }
        }
    }

    private Bitmap decodeUri(Uri selectedImage) throws FileNotFoundException {
        // Decode image size
        BitmapFactory.Options o = new BitmapFactory.Options();
        o.inJustDecodeBounds = true;
        BitmapFactory.decodeStream(getContentResolver().openInputStream(selectedImage), null, o);

        // The new size we want to scale to
        final int REQUIRED_SIZE = 400;

        // Find the correct scale value. It should be the power of 2.
        int width_tmp = o.outWidth, height_tmp = o.outHeight;
        int scale = 1;
        while (true) {
            if (width_tmp / 2 < REQUIRED_SIZE
                    || height_tmp / 2 < REQUIRED_SIZE) {
                break;
            }
            width_tmp /= 2;
            height_tmp /= 2;
            scale *= 2;
        }

        // Decode with inSampleSize
        BitmapFactory.Options o2 = new BitmapFactory.Options();
        o2.inSampleSize = scale;
        return BitmapFactory.decodeStream(getContentResolver().openInputStream(selectedImage), null, o2);
    }
    private void showObjects(Bitmap Out) {
        imageView.setImageBitmap(Out);  //设置imageView组件的图片显示
    }

    //TODO 把assert文件写入系统中
    private boolean copyAssetAndWrite(String fileName){
        try {
            File cacheDir=getCacheDir();
            if (!cacheDir.exists()){
                cacheDir.mkdirs();
            }
            File outFile =new File(cacheDir,fileName);
            if (!outFile.exists()){
                boolean res=outFile.createNewFile();
                if (!res){
                    return false;
                }
            }else {
                if (outFile.length()>10){//表示已经写入一次
                    return true;
                }
            }
            InputStream is=getAssets().open(fileName);
            FileOutputStream fos = new FileOutputStream(outFile);
            byte[] buffer = new byte[1024];
            int byteCount;
            while ((byteCount = is.read(buffer)) != -1) {
                fos.write(buffer, 0, byteCount);
            }
            fos.flush();
            is.close();
            fos.close();
            return true;
        } catch (IOException e) {
            e.printStackTrace();
        }

        return false;
    }

}