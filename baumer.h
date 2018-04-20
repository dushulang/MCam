#pragma once
#include "Camera.h"
#include "atlstr.h"
#include <iostream>
#include <vector>
#include <mutex>
#include "UtilDef.h"
#include "../third_party/baumer/Inc/bgapi2_genicam.hpp"


#define BUFFERCOUNT 4
using namespace BGAPI2;
using namespace MSerials;
#pragma comment(lib,"../third_party/baumer/Lib/x64/bgapi2_genicam.lib")


class Baumer :public Camera
{
private:
	int Method;
	int error = 0;
	int camera_num = 0;

	Buffer * pBufferFilled[MAX_DEVICE_MUN];// = NULL;
	SystemList *systemList = nullptr;
	System *pSystem = nullptr;
	InterfaceList *interfaceList = nullptr;
	BGAPI2::BufferList *bufferList = nullptr;
	Buffer * pBuffer = nullptr;
	BGAPI2::ImageProcessor *imgProcessor = nullptr;
	Buffer *pUserBufferPixeltransformation = nullptr;

	BGAPI2::Image * pTransformImage[MAX_DEVICE_MUN];
	BGAPI2::Image * pImage[MAX_DEVICE_MUN];
	BGAPI2::Events::DeviceEvent* pdEvent[MAX_DEVICE_MUN];// = new BGAPI2::Events::DeviceEvent();


	bool bIsAvailableEventSelector[MAX_DEVICE_MUN];// = false;
	bool bIsAvaliableEventSelectorExposureStart[MAX_DEVICE_MUN];// = false;
	bool bIsAvaliableEventSelectorExposureEnd[MAX_DEVICE_MUN];// = false;
	bool bIsAvaliableEventSelectorFrameTransferEnd[MAX_DEVICE_MUN];// = false;

	std::vector<Device*>	m_vec_DeviceList;
	std::vector<Interface*> vec_InterfaceList;

public:
	Baumer(int method = 0) {
		Method = method;
		for (int i = 0; i < MAX_DEVICE_MUN; i++) {
			pBufferFilled[i] = nullptr;
			pTransformImage[i] = nullptr;
			pImage[i] = nullptr;
			bIsAvailableEventSelector[i] = false;
			bIsAvaliableEventSelectorExposureStart[i] = false;
			bIsAvaliableEventSelectorExposureEnd[i] = false;
			bIsAvaliableEventSelectorFrameTransferEnd[i] = false;
			pdEvent[i] = new BGAPI2::Events::DeviceEvent();
		}
		imgProcessor = BGAPI2::ImageProcessor::GetInstance();
	}


	virtual ~Baumer() {
		std::cout << "free baumer camera" << std::endl;

		try {
			//关闭device
			for (auto bm_interface : vec_InterfaceList) {
				DeviceList *deviceList = bm_interface->GetDevices();
				Device *pDevice = deviceList->begin()->second;
				pDevice->UnregisterDeviceEvent();

				if (pDevice->GetRemoteNodeList()->GetNodePresent("AcquisitionAbort"))
				{
					pDevice->GetRemoteNode("AcquisitionAbort")->Execute();
					std::cout << "5.1.12   " << pDevice->GetModel() << " aborted " << std::endl;
				}

				pDevice->GetRemoteNode("AcquisitionStop")->Execute();
				std::cout << "5.1.12   " << pDevice->GetModel() << " stopped " << std::endl;
				std::cout << std::endl;

				if (pDevice->GetTLType() == "GEV")
				{
					if (pDevice->GetRemoteNodeList()->GetNodePresent("DeviceStreamChannelPacketSize"))
						std::cout << "         DeviceStreamChannelPacketSize:  " << pDevice->GetRemoteNode("DeviceStreamChannelPacketSize")->GetInt() << " [bytes]" << std::endl;
					else
						std::cout << "         GevSCPSPacketSize:              " << pDevice->GetRemoteNode("GevSCPSPacketSize")->GetInt() << " [bytes]" << std::endl;
					std::cout << "         GevSCPD (PacketDelay):          " << pDevice->GetRemoteNode("GevSCPD")->GetInt() << " [tics]" << std::endl;
				}

				DataStream *pDataStream = pDevice->GetDataStreams()->begin()->second;
				pDataStream->StopAcquisition();
				std::cout << "5.1.12   DataStream stopped " << std::endl;
				bufferList = pDataStream->GetBufferList();
				bufferList->DiscardAllBuffers();
				while (bufferList->size() > 0)
				{
					pBuffer = bufferList->begin()->second;
					bufferList->RevokeBuffer(pBuffer);
					delete pBuffer;
				}
				std::cout << "         buffers after revoke:    " << bufferList->size() << std::endl;
				pDataStream->Close();
				pDevice->Close();
				bm_interface->Close();
				pSystem->Close();
				BGAPI2::SystemList::ReleaseInstance();
			}

		}	
		catch (BGAPI2::Exceptions::IException& ex)
		{
			std::cout << "ExceptionType:    " << ex.GetType() << std::endl;
			std::cout << "ErrorDescription: " << ex.GetErrorDescription() << std::endl;
			std::cout << "in function:      " << ex.GetFunctionName() << std::endl;
		}


		//有bug存在
		for (int i = 0; i < MAX_DEVICE_MUN; i++)
		{
		//	if(nullptr != pBufferFilled[i])
		//		delete pBufferFilled[i];
			
			if(nullptr != pdEvent[i])
				delete pdEvent[i];	
		}	
	}

	int refresh_list();
#if 0
	{ 
		
		int returncode = 0;
		try
		{
			systemList = SystemList::GetInstance();
			systemList->Refresh();

			pSystem = systemList->begin()->second;      //gige: begin(); 
																//System *pSystem = systemList->begin()++->second;  //usb3: begin()++
			pSystem->Open();
			interfaceList = pSystem->GetInterfaces();
			interfaceList->Refresh(100);

			//找到相机的数量，最多支持8个
			bo_uint64 DeviceNumber = min(interfaceList->size(), MAX_DEVICE_MUN);
			camera_num = DeviceNumber;
			int idx = 0;
			for (BGAPI2::InterfaceList::iterator it = interfaceList->begin(); it != interfaceList->end() && idx < DeviceNumber; it++, idx++) {
				Interface *pInterface = it->second;
				pInterface->Open();
				DeviceList *deviceList = pInterface->GetDevices();
				deviceList->Refresh(100);
				if (deviceList->size() > 0) {

					Device *pdevice = deviceList->begin()->second;
					pdevice->Open();
					std::cout << pdevice->GetModel() << "(" << pdevice->GetSerialNumber() << ")" << std::endl;

					//设置触发模式
					if (Method == TRIGGER_SOFTWARE)
					{
						pdevice->GetRemoteNode("TriggerSource")->SetString("SoftwareTrigger");
						pdevice->GetRemoteNode("TriggerMode")->SetString("On");
					}
					else if (Method == TRIGGER_HARDWARE) {
						pdevice->GetRemoteNode("TriggerSource")->SetString("Line0");
						pdevice->GetRemoteNode("TriggerMode")->SetString("On");

					}
					else if(Method == TRIGGER_UNABLE){
						pdevice->GetRemoteNode("TriggerMode")->SetString("Off");
					}

					BGAPI2::String sExposureNodeName = "";
					if (pdevice->GetRemoteNodeList()->GetNodePresent("ExposureTime")) {
						sExposureNodeName = "ExposureTime";
					}
					else if (pdevice->GetRemoteNodeList()->GetNodePresent("ExposureTimeAbs")) {
						sExposureNodeName = "ExposureTimeAbs";
					}
					//设置曝光时间
					pdevice->GetRemoteNode(sExposureNodeName)->SetDouble(10000.0);
					//pdevice->GetRemoteNode("EventSelector")->SetString("ExposureStart");
					//std::cout << "        EventSelector:            " << pdevice->GetRemoteNode("EventSelector")->GetValue() << std::endl;
					//pdevice->GetRemoteNode("EventSelector")->SetValue("GigeVisionEvent");
					//std::cout << " " << pdevice->GetRemoteNode("EventNotification")->GetValue() << std::endl;

					pdevice->RegisterDeviceEvent(BGAPI2::Events::EVENTMODE_POLLING);
					DataStreamList *datastreamList = pdevice->GetDataStreams();
					datastreamList->Refresh();
					DataStream *pDataStream = datastreamList->begin()->second;
					pDataStream->Open();

					bufferList = pDataStream->GetBufferList();

					for (int i = 0; i < 4; i++)
					{
						pBuffer = new BGAPI2::Buffer();
						bufferList->Add(pBuffer);
						pBuffer->QueueBuffer();
					}

					pDataStream->StartAcquisitionContinuous();
					pdevice->GetRemoteNode("AcquisitionStart")->Execute();
					vec_InterfaceList.push_back(it->second);
				}
				else {
					std::cout << "no camera found on first system and first interface." << std::endl;
					std::cout << "please modify source code according your camera system , see code line 19." << std::endl;
				}
			}
			return 0;
		}
		catch (BGAPI2::Exceptions::IException& ex)
		{
			returncode = 0 == returncode ? 1 : returncode;
			std::cout << "Error in function: " << ex.GetFunctionName() << std::endl << "Error description: " << ex.GetErrorDescription() << std::endl << std::endl;
			BGAPI2::SystemList::ReleaseInstance();
		}
		//std::cout << "Input any number to close the program:";
		//int endKey = 0;
		//std::cin >> endKey;
		return returncode;
	}
#endif


	int error_info();

	const char * vendor();

	const char * CamerasInfo();

	int device_quantity();
	int Trigger_Snap(int & width, int & height, unsigned char ** data, int & ch, int camera_index, int delay);
	int Normal_Snap(int & width, int & height, unsigned char ** data, int & ch, int camera_index, int delay);
	//
	int Snap(int &width, int &height, unsigned char **data, int &ch, int camera_index = 0, int delay = 35);

	void set_exposure(double exposure = 2000.0, int camera_index = 0);



	//以下函数非所有相机公有
#if 0
	GX_STATUS SetPixelFormat8bit(GX_DEV_HANDLE hDevice);

	void StartAcquisition(int OperateID);

	void OpenDevice(int OperateID = 0);

	//virtual void Grap
	int refresh_list();

	CString Get_Error_Info();

	CString Get_Camera_Info();
#endif
};
