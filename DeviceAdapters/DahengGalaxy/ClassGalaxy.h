#pragma once
#define _AFXDLL
//#include <afxdlgs.h>
//#include "stdafx.h"

#include	"GalaxyException.h"
#include	"GalaxyIncludes.h"
//#include	"GXBitmap.h"
//#include	"FileVersion.h"
//#include	"DxImageProc.h"
	//---------------------------------------------------------------------------------
/**
\brief   用户继承采集事件处理类，回调函数，重点关注 类中有回调类，供采集图像
*/
//----------------------------------------------------------------------------------
#include <sstream>
#include <math.h>
#include "ModuleInterface.h"
#include "DeviceUtils.h"
#include <vector>
#include "DeviceBase.h"
#include "DeviceThreads.h"
#include <string>
#include <vector>
#include <map>
#include "ImageMetadata.h"
#include "ImgBuffer.h"
#include <iostream>



class CircularBufferInserter;

class MODULE_API ClassGalaxy : public CCameraBase<ClassGalaxy>
{

public:

	ClassGalaxy();
	//函数的意义-析构函数 delete时，启用;
	~ClassGalaxy(void);

	// MMDevice API
	// ------------
	int Initialize();
	int Shutdown();

	void GetName(char* name) const;
	bool Busy() { return false; }


	// MMCamera API
	// ------------
	int SnapImage();

	unsigned char* GetImageBufferFromCallBack(CImageDataPointer& objCImageDataPointer);

	const unsigned char* GetImageBuffer();

	// void* Buffer4ContinuesShot;

	unsigned GetNumberOfComponents() const;
	unsigned GetImageWidth() const;
	unsigned GetImageHeight() const;

	unsigned GetImageBytesPerPixel() const;
	long GetImageSizeLarge()const;
	unsigned GetBitDepth() const;

	long GetImageBufferSize() const;

	double GetExposure() const;
	void SetExposure(double exp);
	int SetROI(unsigned x, unsigned y, unsigned xSize, unsigned ySize);
	int GetROI(unsigned& x, unsigned& y, unsigned& xSize, unsigned& ySize);
	int ClearROI();
	void ReduceImageSize(int64_t Width, int64_t Height);
	int GetBinning() const;
	int SetBinning(int binSize);
	int IsExposureSequenceable(bool& seq) const { seq = false; return DEVICE_OK; }

	void CoverToRGB(GX_PIXEL_FORMAT_ENTRY emDstFormat,void* DstBuffer, CImageDataPointer pObjSrcImageData);

	////int SetProperty(const char* name, const char* value);
	int CheckForBinningMode(CPropertyAction* pAct);
	void AddToLog(std::string msg);
	
	GX_VALID_BIT_LIST GetBestValudBit(GX_PIXEL_FORMAT_ENTRY emPixelFormatEntry);
	void CopyToImageBuffer(CImageDataPointer& objImageDataPointer);
	//CImageFormatConverter* converter;
	CircularBufferInserter* ImageHandler_;
	//std::string EnumToString(EDeviceAccessiblityInfo DeviceAccessiblityInfo);
	//void UpdateTemperature();

	/**
	* Starts continuous acquisition. live模式，但是以下函数没有写也会开启live模式，开启该函数之后，软件会卡死，内部有函数
	*/
	int StartSequenceAcquisition(long numImages, double interval_ms, bool stopOnOverflow) final ;
	int StartSequenceAcquisition(double interval_ms) final;
	int StopSequenceAcquisition() final;
	int PrepareSequenceAcqusition() final;

	///**
	//* Flag to indicate whether Sequence Acquisition is currently running.
	//* Return true when Sequence acquisition is active, false otherwise
	//*/
	//bool IsCapturing();

	////Genicam Callback
	////void ResultingFramerateCallback(GenApi::INode* pNode);


	//// action interface
	//// ----------------
	//int OnAcqFramerate(MM::PropertyBase* pProp, MM::ActionType eAct);
	//int OnAcqFramerateEnable(MM::PropertyBase* pProp, MM::ActionType eAct);
	//int OnAutoExpore(MM::PropertyBase* pProp, MM::ActionType eAct);
	//int OnAutoGain(MM::PropertyBase* pProp, MM::ActionType eAct);
	int OnBinning(MM::PropertyBase* pProp, MM::ActionType eAct);
	int OnBinningMode(MM::PropertyBase* pProp, MM::ActionType eAct);
	int OnDeviceLinkThroughputLimit(MM::PropertyBase* pProp, MM::ActionType eAct);
	int OnExposure(MM::PropertyBase* pProp, MM::ActionType eAct);
	int OnGain(MM::PropertyBase* pProp, MM::ActionType eAct);
	int OnHeight(MM::PropertyBase* pProp, MM::ActionType eAct);
	int OnInterPacketDelay(MM::PropertyBase* pProp, MM::ActionType eAct);
	//int OnLightSourcePreset(MM::PropertyBase* pProp, MM::ActionType eAct);
	//int OnOffset(MM::PropertyBase* pProp, MM::ActionType eAct);
	int OnPixelType(MM::PropertyBase* pProp, MM::ActionType eAct);
	//int OnResultingFramerate(MM::PropertyBase* pProp, MM::ActionType eAct);
	//int OnReverseX(MM::PropertyBase* pProp, MM::ActionType eAct);
	//int OnReverseY(MM::PropertyBase* pProp, MM::ActionType eAct);
	//int OnSensorReadoutMode(MM::PropertyBase* pProp, MM::ActionType eAct);
	//int OnShutterMode(MM::PropertyBase* pProp, MM::ActionType eAct);
	//int OnTemperature(MM::PropertyBase* pProp, MM::ActionType eAct);
	//int OnTemperatureState(MM::PropertyBase* pProp, MM::ActionType eAct);
	int OnTriggerMode(MM::PropertyBase* pProp, MM::ActionType eAct);
	int OnTriggerActivation(MM::PropertyBase* pProp, MM::ActionType eAct);
	int OnAdjFrameRateMode(MM::PropertyBase* pProp, MM::ActionType eAct);
	int OnAcquisitionFrameRate(MM::PropertyBase* pProp, MM::ActionType eAct);
	int OnTriggerSource(MM::PropertyBase* pProp, MM::ActionType eAct);
	int OnWidth(MM::PropertyBase* pProp, MM::ActionType eAct);

	int OnTriggerDelay(MM::PropertyBase* pProp, MM::ActionType eAct);

	int OnTriggerFilterRaisingEdge(MM::PropertyBase* pProp, MM::ActionType eAct);
	//为了实现在采集类中使用
	bool  colorCamera_;
	CGXFeatureControlPointer          m_objFeatureControlPtr;     ///< 属性控制器
		 //格式转换函数
	void RG8ToRGB24Packed(void* destbuffer, CImageDataPointer& objImageDataPointer);
	void CoverRGB16ToRGBA16(unsigned short int* Desbuffer, unsigned short int* Srcbuffer);
	void RG10ToRGB24Packed(void* destbuffer, CImageDataPointer& objImageDataPointer);
	void RGB24PackedToRGBA(void* destbuffer, void* srcbuffer, CImageDataPointer& objImageDataPointer);
	void ResizeSnapBuffer();
	void* imgBuffer_;
	bool __IsPixelFormat8(GX_PIXEL_FORMAT_ENTRY emPixelFormatEntry);

private:
	GxIAPICPP::gxdeviceinfo_vector vectorDeviceInfo;
	int nComponents_;
	unsigned bitDepth_;
	unsigned bytesPerPixel_;

	bool IsByerFormat = false;

	/*mutable*/ unsigned Width_, Height_;
	long imageBufferSize_;
	void GetImageSize();

	unsigned maxWidth_, maxHeight_;
	int64_t DeviceLinkThroughputLimit_;
	// double ResultingFrameRatePrevious;
	// double acqFramerate_, acqFramerateMax_, acqFramerateMin_;
	double exposure_us_, exposureMax_, exposureMin_;
	double gain_, gainMax_, gainMin_;
	double offset_, offsetMin_, offsetMax_;

	std::string binningFactor_;
	std::string pixelType_;
	std::string reverseX_, reverseY_;
	std::string sensorReadoutMode_;
	std::string setAcqFrm_;
	std::string shutterMode_;
	std::string temperature_;
	std::string temperatureState_;
	std::string TriggerMode_;
	std::string AcquisitionFrameRateMode_;
	std::string AcquisitionFrameRate_;
	std::string TriggerActivation_;
	std::string TriggerDelay_;
	std::string TriggerFilterRaisingEdge_;

	BITMAPINFO* m_pBmpInfo;	                     ///<BITMAPINFO 结构指针，显示图像时使用
	char               m_chBmpBuf[2048];	     ///<BIMTAPINFO 存储缓冲区，m_pBmpInfo即指向此缓冲区
	void __ColorPrepareForShowImg();
	void __UpdateBitmap(CImageDataPointer& objCImageDataPointer);
	bool __IsCompatible(BITMAPINFO* pBmpInfo, uint64_t nWidth, uint64_t nHeight);

	long imgBufferSize_;
	ImgBuffer img_;
	bool initialized_;

	//图像转换
	CGXImageFormatConvertPointer TestFormatConvertPtr;

	int64_t __GetStride(int64_t nWidth, bool bIsColor);
	void SaveBmp(CImageDataPointer& objCImageDataPointer, const std::string& strFilePath);
	void SaveBmp(CImageDataPointer& objCImageDataPointer, void* buffer, const std::string& strFilePath);
	void SaveRaw(CImageDataPointer& objCImageDataPointer, const std::string& strFilePath);
	unsigned char* imgBuffer_2;
	unsigned char* m_pImageBuffer;

	CGXDevicePointer                  m_objDevicePtr;             ///< 设备句柄
	CGXStreamPointer                  m_objStreamPtr;             ///< 设备流
	CGXFeatureControlPointer          m_objStreamFeatureControlPtr; ///< 流层控制器对象

	bool m_bIsOpen = false;

	//MM::MMTime startTime_;


};

class CircularBufferInserter : public ICaptureEventHandler {
private:
	ClassGalaxy* dev_;

public:
	CircularBufferInserter(ClassGalaxy* dev);

	virtual void DoOnImageCaptured(CImageDataPointer& objImageDataPointer, void* pUserParam);
};