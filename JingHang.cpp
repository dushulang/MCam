
#include "stdafx.h"
#include "JingHang.h"
#include <mutex>
#include <iostream>



int JingHang::refresh_list()
{
	_close();
	int count;
	CameraGetCount(&count);
	camera_num = min(count, MAX_DEVICE_MUN);
	int len = 0;
	for (int i = 0 ; i < camera_num; i++){
		CameraInit(i);
		CameraGetImageBufferSize(i, &len, CAMERA_IMAGE_BMP);
		inBuf[i] = new unsigned char[len];
		CameraSetSnapMode(i, CAMERA_SNAP_CONTINUATION);
	}
	if (0 == count)
	{
		puts("no JingHang Camera found");
		return NO_CAMERA;
	}
	return 0;
}

JingHang::~JingHang()
{
	puts("Freeing JingHang Camera\n");
	_close();
}

void JingHang::_close()
{
	for (int i = 0; i < camera_num; i++){
		CameraFree(i);
	}
	//释放内存
	for (int i = 0; i < MAX_DEVICE_MUN; i++)
	{
		if (nullptr != inBuf[i])
		{
			delete inBuf[i];
			inBuf[i] = nullptr;
		}
	}
}

int JingHang::error_info()
{
	return 0;
}

const char * JingHang::vendor()
{
	return "JingHang";
}

const char * JingHang::sdk_version()
{
	return "2016.09.09";
}

const char * JingHang::CamerasInfo()
{
	return nullptr;
}

int JingHang::device_quantity()
{
	return camera_num;
}

int JingHang::Snap(int & width, int & height, unsigned char ** data, int & ch, int camera_index, int delay)
{
	if (camera_index >= device_quantity() || camera_index<0) return -1;
	static std::mutex m_mtx[MAX_DEVICE_MUN];
	//每一个相机一个锁，保证不相互锁住，只锁相同的相机
	//有bug 不知道像素通道
	std::lock_guard<std::mutex> lck(m_mtx[camera_index]);
	int len = 0;
	CameraGetImageBufferSize(camera_index, &len, CAMERA_IMAGE_BMP);
	Sleep(delay);
	CameraGetImageSize(camera_index, &width, &height);
	CameraQueryImage(camera_index, inBuf[camera_index], &len, CAMERA_IMAGE_BMP);
	ch = len / (width*height);
	*data = inBuf[camera_index];
	return 0;
}


void JingHang::set_exposure(double exposure, int camera_index)
{
}
