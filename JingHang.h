#pragma once
#include "Camera.h"
#ifndef __JING_HANG__
#define __JING_HANG__

/*
���Ķ�camera����ĵ�������ע�������lib�ļ�·��λ��

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
	//��ȡͼƬ
	int Snap(int &width, int &height, unsigned char **data, int &ch, int camera_index = 0, int delay = 35);

	void set_exposure(double exposure = 2000.0, int camera_index = 0);
};

#endif