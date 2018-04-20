/*

需要几种相机的驱动

大恒与basler的在官网上下即可
京航相机的下载网址为 链接: https://pan.baidu.com/s/1o9Ewf82 密码: d6sn

2018 SunKun All Rights Reserved
*/




#pragma once

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


#if 0
#include "UtilDef.h"
#include "../MSerialsCore/Core/MCPPUtil.h"
#include "../MSerialsCore/cv_module/hal/interface.h"

namespace MSerials
{
	class Buffer : public MRootObject {
	public:
		virtual const char*ClassName(void) { return "Buffer"; }
		/*
		GetBuffer will return the image data address
		*/
		virtual void* GetBuffer() {
			return nullptr;
		}
		/*
		GetWidth() will return the image width
		*/
		virtual int GetWidth() {
			return 0;
		};
		/*
		GetHeight() will reutrn the image height
		*/
		virtual int GetHeight() {
			return 0;
		};

		/*
		return the image format
		*/
		virtual int GetPixFormat() {
			return IMAGE_HAL_ERROR_UNKNOWN;
		};

		virtual int GetDeviceID(){
			return 0;
		}
	};
};
#endif

	class Camera
	{
	public:
		Camera();
		virtual ~Camera();

		virtual int error_info() { return 0; }

		virtual const char * vendor() { return "none"; }

		virtual const char * sdk_version() { return "sdk version none"; }

		virtual const char * CamerasInfo() { return "none"; }

		virtual int device_quantity() = 0;
		//
		virtual int Snap(int &width, int &height, unsigned char **data, int &ch, int camera_index = 0, int delay = 0) { width = 0; height = 0; ch = 1; *data = nullptr; return -1; }

		virtual void set_exposure(double exposure = 2000.0, int camera_index = 0) { return; }
		//virtual void Grap,子类必须实现
		virtual int refresh_list() = 0;// { return 0; }
		 
		//子类实现关闭相机
		virtual void close() { return; }
	};


