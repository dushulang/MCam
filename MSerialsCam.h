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


//ö�����,��������������൱�ڳ�ʼ��,�������ṩ���������ʽ��Ĭ��Ϊ�������������
CAMERADLL int			enum_cameras(int method = 0);
//��ȡ�������Ӧ������
CAMERADLL int			get_vendors_num();
//��ȡ�������SDK��Ϣ
CAMERADLL const char*	get_sdk_ver_by_vendor(int _vendor);
//��ȡ������ĸù�Ӧ��Ʒ�����������
CAMERADLL int			get_cameras_num_by_vendor(int _vendor);
//��ȡ��Ӧ������
CAMERADLL const char*	get_vendor_name(int manufactory = 0);
//��ȡ�����Ϣ
CAMERADLL const char*	get_camera_info_by_index(int manufactory = 0, int camera_idx = 0);
//���壬�ӵ�manufactory�������̵ĵ�camera_index������ͼƬ
CAMERADLL int			Snap(int & width, int & height, unsigned char **data, int & ch, int manufactory = 0, int camera_index = 0, int delay = 0);
//�ر�����������ڴ�й©
CAMERADLL void			close_device();

