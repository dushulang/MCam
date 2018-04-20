#pragma once
#pragma once
#include <stdarg.h>


#define DLL_EXPORT

#ifdef DLL_EXPORT
#define CAMERADLL __declspec(dllexport)
#else
#define CAMERADLL __declspec(dllimport)
#endif


#ifndef ___TRIGGER_METHOD___
#define ___TRIGGER_METHOD___

#define TRIGGER_MASK		0xF

#define _TRIGGER_UNABLE		0x0
#define _TRIGGER_SOFTWARE	0x1
#define _TRIGGER_HARDWARE	0x2

#define TRIGGER_UNABLE		(TRIGGER_MASK&_TRIGGER_UNABLE)
#define TRIGGER_SOFTWARE	(TRIGGER_MASK&_TRIGGER_SOFTWARE)
#define TRIGGER_HARDWARE	(TRIGGER_MASK&_TRIGGER_HARDWARE)

#endif


//枚举相机,返回相机数量，相当于初始化,参数是提供了相机排序方式，默认为按相机名称排列
CAMERADLL int			enum_cameras(int method = 0);
//获取供相机的应商数量
CAMERADLL int			get_vendors_num();
//获取供相机的SDK信息
CAMERADLL const char*	get_sdk_ver_by_vendor(int _vendor);
//获取供相机的该供应商品牌相机的数量
CAMERADLL int			get_cameras_num_by_vendor(int _vendor);
//获取供应商名字
CAMERADLL const char*	get_vendor_name(int manufactory = 0);
//获取相机信息
CAMERADLL const char*	get_camera_info_by_index(int manufactory = 0, int camera_idx = 0);
//含义，从第manufactory的制造商的第camera_index相机获得图片
CAMERADLL int			Snap(int & width, int & height, unsigned char **data, int & ch, int manufactory = 0, int camera_index = 0, int delay = 0);
//关闭相机，以免内存泄漏
CAMERADLL void			close_device();

