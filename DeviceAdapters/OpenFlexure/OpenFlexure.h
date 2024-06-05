//////////////////////////////////////////////////////////////////////////////
// FILE:          OpenFlexure.h
// PROJECT:       Micro-Manager
// SUBSYSTEM:     DeviceAdapters
//-----------------------------------------------------------------------------
// DESCRIPTION:   Adapter for the OpenFlexure Microscope. This adapter is used on the v5 Sangaboard.
//
// COPYRIGHT:     Samdrea Hsu
//
// AUTHOR:        Samdrea Hsu, samdreahsu@gmail.edu, 02/28/2024
//
//////////////////////////////////////////////////////////////////////////////


#ifndef _OpenFlexure_H_
#define _OpenFlexure_H_

#include "MMDevice.h"
#include "DeviceBase.h"
#include <string>
#include <map>

// Global keywords 
const char* g_XYStageDeviceName = "XY Stage";
const char* g_HubDeviceName = "SangaboardHub";
const char* g_ZStageDeviceName = "Z Stage";
const char* g_ShutterDeviceName = "LED illumination";

const char* g_Keyword_Response = "SerialResponse";
const char* g_Keyword_Command = "SerialCommand";
const char* g_Keyword_Brightness = "Brightness";
const char* g_Keyword_StepDelay = "Stage Step Delay";
const char* g_Keyword_MinStepDelay = "Minimum Step Delay";
const char* g_Keyword_RampTime = "Ramp Time";
const char* g_ExtraCommand_Stop = "Stop";
const char* g_ExtraCommand_Zero = "Zero";
const char* g_ExtraCommand_Release = "Release";
const char* NoHubError = "Parent Hub not defined.";

// Custom Error texts
#define DEVICE_STAGE_STILL_MOVING     42
const char* const g_Msg_DEVICE_STAGE_STILL_MOVING = "Stage is still moving. Current move aborted.";


class SangaBoardHub : public HubBase<SangaBoardHub>
{
public:
	SangaBoardHub();
	~SangaBoardHub();

	// MMDevice API
	// ------------
	int Initialize();
	int Shutdown();
	void GetName(char* pszName) const;
	bool Busy();

	// Hub API
	// --------
	int DetectInstalledDevices();

	// Action Interface
	int OnPort(MM::PropertyBase* pPropt, MM::ActionType eAct);
	int OnManualCommand(MM::PropertyBase* pPropt, MM::ActionType eAct);
	int OnStepDelay(MM::PropertyBase* pPropt, MM::ActionType eAct);
	int OnMinStepDelay(MM::PropertyBase* pPropt, MM::ActionType eAct);
	int OnRampTime(MM::PropertyBase* pPropt, MM::ActionType eAct);
	int OnExtraFunctions(MM::PropertyBase* pPropt, MM::ActionType eAct);

	// Helper Functions
	void GetPort(std::string& port);
	int SendCommand(std::string cmd, std::string& res);


private:
	//void GetPeripheralInventory();
	//std::vector<std::string> peripherals_;
	bool initialized_;
	bool busy_;
	bool portAvailable_;
	std::string port_;
	std::string _command;
	std::string _serial_answer;
	MMThreadLock serial_lock_;


	bool IsPortAvailable() { return portAvailable_; }

};

class XYStage : public  CXYStageBase<XYStage>
{
public:
	XYStage();
	~XYStage();

	// MMDevice API
	// ------------
	int Initialize();
	int Shutdown();

	// XYStage API
	// -----------
	int SetPositionSteps(long x, long y);
	int GetPositionSteps(long& x, long& y);
	int SetRelativePositionSteps(long x, long y);
	int GetPositionUm(double& posX, double& posY);
	int SetPositionUm(double posX, double posY);
	int SetRelativePositionUm(double posX, double posY);
	int SetOrigin();
	int SetAdapterOrigin();
	int Home();
	int Stop();
	double GetStepSizeXUm() { return stepSizeUm_; }
	double GetStepSizeYUm() { return stepSizeUm_; }
	int GetStepLimits(long& xMin, long& xMax, long& yMin, long& yMax);
	int GetLimitsUm(double& xMin, double& xMax, double& yMin, double& yMax);
	int IsXYStageSequenceable(bool& isSequenceable) const { isSequenceable = false; return DEVICE_OK; }


	bool Busy() { return false; }
	void GetName(char*) const;

	// Action Interface
	//int OnPort(MM::PropertyBase* pPropt, MM::ActionType eAct);
	//int OnCommand(MM::PropertyBase* pPropt, MM::ActionType eAct);
	
	// Action functions
	int SyncState();


private:

	// Variables for manipulating the LED array
	long stepsX_;
	long stepsY_;
	bool initialized_;
	bool portAvailable_;
	double stepSizeUm_;
	std::string port_;
	//MMThreadLock lock_;
	//std::string _command;
	std::string _serial_answer;
	SangaBoardHub* pHub;


	bool IsPortAvailable() { return portAvailable_; }

};



class ZStage : public CStageBase<ZStage>
{
public:
	ZStage();
	~ZStage();


	// MMDevice API
	// ------------
	int Initialize();
	int Shutdown() { initialized_ = false; return DEVICE_OK; }
	void GetName(char* name) const;

	// ZStage API
	// -----------
	int SetPositionUm(double pos) { return DEVICE_UNSUPPORTED_COMMAND;}
	int SetPositionSteps(long steps) { return DEVICE_UNSUPPORTED_COMMAND;}
	int SetRelativePositionUm(double d);
	int SetRelativePositionSteps(long z);
	int Stop() {return DEVICE_UNSUPPORTED_COMMAND;} 
	int GetPositionUm(double& pos);
	int GetPositionSteps(long& steps);
	int SetOrigin();
	int GetLimits(double& lower, double& upper) { return DEVICE_UNSUPPORTED_COMMAND;} // nah 
	int IsStageSequenceable(bool& isSequenceable) const { isSequenceable = false;  return DEVICE_OK;}
	bool IsContinuousFocusDrive() const  { return false; }
	bool Busy() { return false; }

	// Action functions
	int SyncState();


private:
	long stepsZ_;
	bool initialized_;
	double stepSizeUm_;
	std::string _serial_answer;
	SangaBoardHub* pHub;
};


class LEDIllumination : public CShutterBase<LEDIllumination>
{
public:
	LEDIllumination() : state_(false), initialized_(false), changedTime_(0.0), brightness_(1.0)
	{
		EnableDelay(); // signals that the delay setting will be used
		// parent ID display
		CreateHubIDProperty();
	}
	~LEDIllumination() {}

	int Initialize();
	int Shutdown() { initialized_ = false; return DEVICE_OK; }
	void GetName(char* name) const;


	// Shutter API
	int SetOpen(bool open);
	int GetOpen(bool& open);
	int Fire(double deltaT) { return DEVICE_UNSUPPORTED_COMMAND;}
	bool Busy() { return false; }

	// action interface
	int OnState(MM::PropertyBase* pProp, MM::ActionType eAct);
	int OnBrightness(MM::PropertyBase* pPropt, MM::ActionType eAct);

	// Action functions
	int SyncState();
	int SetBrightness();

private:
	bool state_;
	bool initialized_;
	double brightness_;
	MM::MMTime changedTime_;
	std::string _serial_answer;
	SangaBoardHub* pHub;

};



#endif 


