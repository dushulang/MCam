#include "stdafx.h"
#include "baumer.h"
#include <stdio.h>
#include <iostream>
#include <iomanip>
int Baumer::error_info()
{
	return 0;
}

const char * Baumer::vendor()
{
	return nullptr;
}

const char * Baumer::CamerasInfo()
{
	return nullptr;
}

int Baumer::device_quantity()
{
	return	camera_num;
}



inline int Baumer::Normal_Snap(int & width, int & height, unsigned char ** data, int & ch, int camera_index, int delay) {
	try {
		DeviceList *deviceList = vec_InterfaceList.at(camera_index)->GetDevices();
		Device *pDevice = deviceList->begin()->second;

		DataStream *pDataStream = pDevice->GetDataStreams()->begin()->second;

		pBufferFilled[camera_index] = pDataStream->GetFilledBuffer(delay);
		if (pBufferFilled[camera_index] == NULL) { std::cout << "Error: Buffer Timeout after 1000 msec" << std::endl; }
		else if (pBufferFilled[camera_index]->GetIsIncomplete() == true) { std::cout << "Error: Image is incomplete" << std::endl; pBufferFilled[camera_index]->QueueBuffer(); }
		else {
			//std::cout << " Image " << pBufferFilled[camera_index]->GetFrameID() << " received. " << std::endl;
			width = static_cast<int>(pBufferFilled[camera_index]->GetWidth());
			height = static_cast<int>(pBufferFilled[camera_index]->GetHeight());

			if (nullptr != pImage[camera_index])
			{
				pImage[camera_index]->Release();
			}
			if (nullptr != pTransformImage[camera_index])
			{
				pTransformImage[camera_index]->Release();
			}

			pImage[camera_index] = imgProcessor->CreateImage(
				(bo_uint)pBufferFilled[camera_index]->GetWidth(),
				(bo_uint)(int)pBufferFilled[camera_index]->GetHeight(),
				pBufferFilled[camera_index]->GetPixelFormat(),
				(void*)((unsigned char*)pBufferFilled[camera_index]->GetMemPtr() + pBufferFilled[camera_index]->GetImageOffset()),
				pBufferFilled[camera_index]->GetMemSize()
			);
			pImage[camera_index]->TransformImage("BGR8", &pTransformImage[camera_index]);
			*data = (uchar*)pTransformImage[camera_index]->GetBuffer();

			ch = 3;
			pBufferFilled[camera_index]->QueueBuffer();
		}

	}
	catch (std::out_of_range e)
	{
		std::cout << "camera_index out of range" << std::endl;
		*data = NULL;
		return -1;
	}
	return 0;

}

inline int Baumer::Trigger_Snap(int & width, int & height, unsigned char ** data, int & ch, int camera_index, int delay) {
	try {
		DeviceList *deviceList = vec_InterfaceList.at(camera_index)->GetDevices();
		Device *pDevice = deviceList->begin()->second;
		DataStream *pDataStream = pDevice->GetDataStreams()->begin()->second;
		if (pDevice->GetDeviceEvent(pdEvent[camera_index], 1000) == true)
		{
			//std::cout << "EventID " << pdEvent[camera_index]->GetId() << " " << pdEvent[camera_index]->GetName()<<std::endl;	
		}
		else
		{
			std::cout << "Error: GetDeviceEvent Timeout after 1000 msec first" << std::endl;
			return -1;
		}

		// 5. WAIT FOR EVENT EXPOSURE END
		if (pDevice->GetDeviceEvent(pdEvent[camera_index], 1000) == true)
		{
			//std::cout << "EventID " << pdEvent[camera_index]->GetId() << " " << pdEvent[camera_index]->GetName() << std::endl;
		}
		else
		{
			std::cout << "Error: GetDeviceEvent Timeout after 1000 msec second" << std::endl;
			return -2;
		}
		pBufferFilled[camera_index] = pDataStream->GetFilledBuffer(1000);

		if (pBufferFilled[camera_index] == NULL)
		{
			std::cout << "Error: Buffer Timeout after 1000 msec" << std::endl << std::endl;
		}
		else if (pBufferFilled[camera_index]->GetIsIncomplete() == true)
		{
			std::cout << "Error: Image is incomplete" << std::endl << std::endl;
			// queue buffer again
			pBufferFilled[camera_index]->QueueBuffer();
		}
		else
		{
			width = static_cast<int>(pBufferFilled[camera_index]->GetWidth());
			height = static_cast<int>(pBufferFilled[camera_index]->GetHeight());

			if (nullptr != pImage[camera_index])
			{
				pImage[camera_index]->Release();
			}
			if (nullptr != pTransformImage[camera_index])
			{
				pTransformImage[camera_index]->Release();
			}

			pImage[camera_index] = imgProcessor->CreateImage(
				(bo_uint)pBufferFilled[camera_index]->GetWidth(),
				(bo_uint)(int)pBufferFilled[camera_index]->GetHeight(),
				pBufferFilled[camera_index]->GetPixelFormat(),
				(void*)((unsigned char*)pBufferFilled[camera_index]->GetMemPtr() + pBufferFilled[camera_index]->GetImageOffset()),
				pBufferFilled[camera_index]->GetMemSize()
			);
			pImage[camera_index]->TransformImage("BGR8", &pTransformImage[camera_index]);
			*data = (uchar*)pTransformImage[camera_index]->GetBuffer();

			ch = 3;
			// queue buffer again
			pBufferFilled[camera_index]->QueueBuffer();
		}


	}
	catch (std::out_of_range e){
		std::cout << "camera_index out of range" << std::endl;
		*data = NULL;
		return -1;
	
	}
	catch (BGAPI2::Exceptions::IException& ex)
	{
		std::cout << "ExceptionType:    " << ex.GetType() << std::endl;
		std::cout << "ErrorDescription: " << ex.GetErrorDescription() << std::endl;
		std::cout << "in function:      " << ex.GetFunctionName() << std::endl;
		return -2;
	}
	return 0;
}


int Baumer::Snap(int & width, int & height, unsigned char ** data, int & ch, int camera_index, int delay)
{
	static std::mutex mtx[MAX_DEVICE_MUN];
	std::lock_guard<std::mutex> lck(mtx[camera_index]);
	switch (Method) {
	case TRIGGER_UNABLE: return Normal_Snap(width, height, data, ch, camera_index, delay); break;
	case TRIGGER_HARDWARE:return Trigger_Snap(width, height, data, ch, camera_index, delay); break;
	case TRIGGER_SOFTWARE:return Trigger_Snap(width, height, data, ch, camera_index, delay); break;
	default:return -3;
	}
}

void Baumer::set_exposure(double exposure, int camera_index)
{
}

int Baumer::refresh_list()
{
	int returncode = 0;
	try {
		bo_double fExposureValue = 300;
		bo_double fTimestampTickFrequency = 1.0;
		bo_double fTimestampExposureStart = 0.0;
		bo_double fTimestampExposureEnd = 0.0;
		bo_double fTimestampDiff = 0.0;

		systemList = SystemList::GetInstance();
		systemList->Refresh();
		pSystem = systemList->begin()->second;      
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
				BGAPI2::Events::DeviceEvent* pdEvent = new BGAPI2::Events::DeviceEvent();
				Device *pDevice = deviceList->begin()->second;
				pDevice->Open();


				BGAPI2::NodeMap * pEnumNodes = pDevice->GetRemoteNode("TriggerSource")->GetEnumNodeList();
				if (pDevice->GetRemoteNodeList()->GetNodePresent("GevTimestampTickFrequency"))
				{
					fTimestampTickFrequency = (bo_double)pDevice->GetRemoteNode
					("GevTimestampTickFrequency")->GetInt();
				}
				else
				{
					fTimestampTickFrequency = 1000000000.0;
				}
				// 2. SET EXPOSURE TIME
				pDevice->GetRemoteNode("ExposureTime")->SetDouble(fExposureValue);
				std::cout << "Set Exposure to: ";
				std::cout << pDevice->GetRemoteNode("ExposureTime")->GetDouble();
				std::cout << " [usec]" << std::endl;

				std::cout << "Tigger Mode List:" << std::endl;
				for (BGAPI2::NodeMap::iterator nit = pEnumNodes->begin(); nit != pEnumNodes->end(); nit++)
				{
					std::cout <<"	"<< nit->first << std::endl;
				}
				std::cout << "	" << std::endl;
				String sTriggerSourceNodeName = "", Tigger_Switch="";
				pDevice->GetRemoteNode("LineSelector")->SetString("Line0");

				if (pEnumNodes->GetNodePresent("SoftwareTrigger")) {
					sTriggerSourceNodeName = "SoftwareTrigger";
				}
				else if (pEnumNodes->GetNodePresent("Software")) {
					sTriggerSourceNodeName = "Software";
				}
				else if (pEnumNodes->GetNodePresent("Line0")) {
					sTriggerSourceNodeName = "Line0";
				}


				//Method = TRIGGER_SOFTWARE;
				if (TRIGGER_HARDWARE == Method)
				{
					sTriggerSourceNodeName = "Line0";
					Tigger_Switch = "On";
				}
				else if (TRIGGER_SOFTWARE == Method)
				{
					sTriggerSourceNodeName = "Software";
					Tigger_Switch = "On";
				}
				else
				{
					Tigger_Switch = "Off";
				}


				//SET TRIGGER SOURCE "SOFTWARE"
				pDevice->GetRemoteNode("TriggerSource")->SetString(sTriggerSourceNodeName);
				std::cout << "        TriggerSource:            " << pDevice->GetRemoteNode("TriggerSource")->GetValue() << std::endl;

				//SET TRIGGER MODE ON
				pDevice->GetRemoteNode("TriggerMode")->SetString(Tigger_Switch);
				std::cout << "        TriggerMode:              " << pDevice->GetRemoteNode("TriggerMode")->GetValue() << std::endl << std::endl;







				pDevice->GetRemoteNode("EventSelector")->SetString("ExposureStart");
				std::cout << "        EventSelector:            " << pDevice->GetRemoteNode("EventSelector")->GetValue() << std::endl;

				if ((pDevice->GetRemoteNode("EventNotification")->GetEnumNodeList()->GetNodePresent("On")) == true)
				{
					pDevice->GetRemoteNode("EventNotification")->SetString("On");  // standard: MXG, VisiLine,...
					std::cout << "        EventNotification:        " << pDevice->GetRemoteNode("EventNotification")->GetValue() << std::endl << std::endl;
				}
				else
				{
					pDevice->GetRemoteNode("EventNotification")->SetString("GigEVisionEvent");  // TXG, SXG
					std::cout << "        EventNotification:        " << pDevice->GetRemoteNode("EventNotification")->GetValue() << std::endl << std::endl;
				}


				// MESSAGE DEVEICE EVENT "ExposureEnd"
				//====================================

				pDevice->GetRemoteNode("EventSelector")->SetString("ExposureEnd");
				std::cout << "        EventSelector:            " << pDevice->GetRemoteNode("EventSelector")->GetValue() << std::endl;

				if ((pDevice->GetRemoteNode("EventNotification")->GetEnumNodeList()->GetNodePresent("On")) == true)
				{
					pDevice->GetRemoteNode("EventNotification")->SetString("On");  // standard: MXG, VisiLine,...
					std::cout << "        EventNotification:        " << pDevice->GetRemoteNode("EventNotification")->GetValue() << std::endl << std::endl;
				}
				else
				{
					pDevice->GetRemoteNode("EventNotification")->SetString("GigEVisionEvent");  // TXG, SXG
					std::cout << "        EventNotification:        " << pDevice->GetRemoteNode("EventNotification")->GetValue() << std::endl << std::endl;
				}
				std::cout << std::endl;




				//可能没设置造成的硬触发超时
				BGAPI2::String sEventSelectorItem;

				for (bo_uint64 i = 0; bIsAvailableEventSelector && i < pDevice->GetRemoteNode("EventSelector")->GetEnumNodeList()->GetNodeCount(); i++)
				{
					BGAPI2::Node *pNode = pDevice->GetRemoteNode("EventSelector")->GetEnumNodeList()->GetNodeByIndex(i);
					if ((pNode->GetImplemented() == true) && (pNode->GetAvailable() == true) &&
						(pNode->GetLocked() == false) && (pNode->GetVisibility() != "Invisible"))
					{
						std::cout << "                            " << std::setw(3) << i << ":  ";
						sEventSelectorItem = pNode->GetValue();
						std::cout << std::left << std::setw(28) << sEventSelectorItem << std::right;

						// check event notification is enabled ("On" or "GigeVisionEvent") or not ("Off")
						pDevice->GetRemoteNode("EventSelector")->SetValue(sEventSelectorItem);
						std::cout << " " << pDevice->GetRemoteNode("EventNotification")->GetValue() << std::endl;
					}
				}
				std::cout << std::endl;


				pDevice->RegisterDeviceEvent(BGAPI2::Events::EVENTMODE_POLLING);
				try
				{
					std::cout << "5.1.12   " << pDevice->GetModel() << " started " << std::endl;
					pDevice->GetRemoteNode("AcquisitionStart")->Execute();
				}
				catch (BGAPI2::Exceptions::IException& ex)
				{
					returncode = 0 == returncode ? 1 : returncode;
					std::cout << "ExceptionType:    " << ex.GetType() << std::endl;
					std::cout << "ErrorDescription: " << ex.GetErrorDescription() << std::endl;
					std::cout << "in function:      " << ex.GetFunctionName() << std::endl;
				}


				BGAPI2::DataStreamList *datastreamList = pDevice->GetDataStreams();
				datastreamList->Refresh();
	
				DataStream *pDataStream = datastreamList->begin()->second;
				pDataStream->Open();

				bufferList = pDataStream->GetBufferList();
				// 4 buffers using internal buffer mode
				for (int i = 0; i<4; i++)
				{
					pBuffer = new BGAPI2::Buffer();
					bufferList->Add(pBuffer);
				}
				try
				{
					for (BufferList::iterator bufIterator = bufferList->begin(); bufIterator != bufferList->end(); bufIterator++)
					{
						bufIterator->second->QueueBuffer();
					}
					std::cout << "5.1.11   Queued buffers:          " << bufferList->GetQueuedCount() << std::endl;
				}
				catch (BGAPI2::Exceptions::IException& ex)
				{
					returncode = 0 == returncode ? 1 : returncode;
					std::cout << "ExceptionType:    " << ex.GetType() << std::endl;
					std::cout << "ErrorDescription: " << ex.GetErrorDescription() << std::endl;
					std::cout << "in function:      " << ex.GetFunctionName() << std::endl;
				}

				pDataStream->StartAcquisitionContinuous();
				pDevice->GetRemoteNode("AcquisitionStart")->Execute();

				vec_InterfaceList.push_back(it->second);

//测试硬触发模式
#if 0
				std::cout << "Execute TriggerSoftware " << std::endl;
				pDevice->GetRemoteNode("TriggerSoftware")->Execute();

				if (pDevice->GetDeviceEvent(pdEvent, 1000) == true)
				{
					fTimestampExposureStart = (bo_double)pdEvent->GetTimeStamp() / fTimestampTickFrequency;
					fTimestampExposureStart = fTimestampExposureStart * 1000000.0;
					std::cout << "EventID " << pdEvent->GetId() << " " << pdEvent->GetName();
					std::cout << " Timestamp " << fTimestampExposureStart << " [usec]" << std::endl;
				}
				else
				{
					std::cout << "Error: GetDeviceEvent Timeout after 1000 msec" << std::endl;
				}

				// 5. WAIT FOR EVENT EXPOSURE END
				if (pDevice->GetDeviceEvent(pdEvent, 1000) == true)
				{
					fTimestampExposureEnd = (bo_double)pdEvent->GetTimeStamp() / fTimestampTickFrequency;
					fTimestampExposureEnd = fTimestampExposureEnd * 1000000.0;
					std::cout << "EventID " << pdEvent->GetId() << " " << pdEvent->GetName();
					std::cout << " Timestamp " << fTimestampExposureEnd << " [usec]" << std::endl;
				}
				else
				{
					std::cout << "Error: GetDeviceEvent Timeout after 1000 msec" << std::endl;
				}
#endif

				
			}
		
		}
	}
	catch (BGAPI2::Exceptions::IException& ex)
	{
		returncode = 0 == returncode ? 1 : returncode;
		std::cout << "Error in function: " << ex.GetFunctionName() << std::endl << "Error description: " << ex.GetErrorDescription() << std::endl << std::endl;
		BGAPI2::SystemList::ReleaseInstance();
	}
	return 0;
}
