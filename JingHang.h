#pragma once
#include "Camera.h"
#ifndef __JING_HANG__
#define __JING_HANG__

/*
请阅读camera里的文档，并且注意下面的lib文件路径位置

*/


#include "../third_party/jinghang/JHCap.h"
#pragma comment(lib,"../third_party/jinghang/JHCap2.lib")

class JingHang:public Camera
{
private:
	unsigned char *inBuf[MAX_DEVICE_MUN];
private:
	int Method;
	int error = 0;
	int camera_num = 0;

public:
	JingHang(int method = 0) {
		Method = method;
		for (size_t i = 0; i < MAX_DEVICE_MUN; i++) {
			inBuf[i] = nullptr;
		}
	}

	virtual ~JingHang();// { std::cout << "freeing basler camera" << std::endl;  close();  }//To CLose camera};

	int refresh_list();

	void _close();

	int error_info();

	const char * vendor();

	const char * sdk_version();

	const char * CamerasInfo();

	int device_quantity();
	//获取图片
	int Snap(int &width, int &height, unsigned char **data, int &ch, int camera_index = 0, int delay = 35);

	void set_exposure(double exposure = 2000.0, int camera_index = 0);
};

#endif