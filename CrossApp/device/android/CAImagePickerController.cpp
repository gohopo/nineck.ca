//
//  CAImagePickerController.h
//  CrossApp
//
//  Created by 秦乐 on 2017/2/21.
//  Copyright © 2017年 cocos2d-x. All rights reserved.
//

#include "../CAImagePickerController.h"
#include "basics/CAScheduler.h"
#if (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
#include "platform/android/jni/JniHelper.h"
#include <jni.h>

NS_CC_BEGIN

extern "C"
{
    void JAVAOpenCamera(int type)
    {
        JniMethodInfo jmi;
        if(JniHelper::getStaticMethodInfo(jmi , "org/CrossApp/lib/CrossAppDevice" , "CAImageCapture" , "(I)V"))
        {
            jmi.env->CallStaticVoidMethod(jmi.classID , jmi.methodID,type);
            jmi.env->DeleteLocalRef(jmi.classID);
        }
    }
    
    void JAVAOpenAlbum(int type)
    {
        JniMethodInfo jmi;
        if(JniHelper::getStaticMethodInfo(jmi , "org/CrossApp/lib/CrossAppNativeTool" , "CAImageAlbum" , "(I)V"))
        {
            jmi.env->CallStaticVoidMethod(jmi.classID , jmi.methodID,type);
            jmi.env->DeleteLocalRef(jmi.classID);
        }
    }
    
    void JAVAwriteImageToPhoto(CAImage* image, const std::string &imageName)
    {
        JniMethodInfo jmi;
        const char* path;
        std::string savePath;
        if (JniHelper::getStaticMethodInfo(jmi, "org/CrossApp/lib/CrossAppDevice", "getSaveImagePath", "()Ljava/lang/String;"))
        {
            jstring a = (jstring)jmi.env->CallStaticObjectMethod(jmi.classID, jmi.methodID);
            path = jmi.env->GetStringUTFChars(a, 0);
        }
        savePath = path;
        savePath = savePath + imageName + ".jpg";
        image->saveToFile(savePath);
        
        if (JniHelper::getStaticMethodInfo(jmi, "org/CrossApp/lib/CrossAppDevice", "UpdateCamera", "(Ljava/lang/String;)V"))
        {
            jmi.env->CallStaticVoidMethod(jmi.classID, jmi.methodID, jmi.env->NewStringUTF(savePath.c_str()));
            jmi.env->DeleteLocalRef(jmi.classID);
        }
    }
}

#endif

std::function<void(CrossApp::CAImage*)> _imagePickerControllerCallBack;

CAImagePickerController::CAImagePickerController(SourceType type)
: m_eSourceType(type)
{
    
}

CAImagePickerController::~CAImagePickerController()
{
    
}

CAImagePickerController* CAImagePickerController::create(SourceType type)
{
    CAImagePickerController* var = new CAImagePickerController(type);
    if (var && var->init())
    {
        var->autorelease();
        return var;
    }
    CC_SAFE_DELETE(var);
    return nullptr;
}

bool CAImagePickerController::init()
{
    return true;
}

void CAImagePickerController::open(const std::function<void(CrossApp::CAImage*)>& callback)
{
    _imagePickerControllerCallBack = callback;
    
    switch (m_eSourceType)
    {
        case CAImagePickerController::SourceType::PhotoLibrary:
        {
            JAVAOpenAlbum(1);
        }
            break;
        case CAImagePickerController::SourceType::CameraDeviceRear:
        {
            JAVAOpenCamera(1);
        }
            break;
        case CAImagePickerController::SourceType::CameraDeviceFront:
        {
            JAVAOpenCamera(2);
        }
            break;
        case CAImagePickerController::SourceType::SavedPhotosAlbum:
        {
            CCAssert(false, "");
            return;
        }
            break;
        default:
            break;
    }
}

void CAImagePickerController::writeImageToPhoto(CAImage* image, const std::function<void(bool)>& finishCallback, const std::string &imageName)
{
    JAVAwriteImageToPhoto(image, imageName);
}


extern "C"
{
    JNIEXPORT void JNICALL Java_org_CrossApp_lib_CrossAppNativeTool_NativeReturn
    ( JNIEnv* env,jobject thiz ,jstring arg1, jobject arg2)
    {
        const char* str = env->GetStringUTFChars(arg1, false);

        CAScheduler::getScheduler()->performFunctionInUIThread( [=]()
       {
           if (_imagePickerControllerCallBack)
           {
               CAImage *image = new CAImage();
               if (image->initWithImageFile(str))
               {
                   _imagePickerControllerCallBack(CAImage::generateMipmapsWithImage(image));
                   image->release();
               }
           }
       });
    }
}

NS_CC_END