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
const char* g_XYStageDeviceName = "OpenFlexure";
const char* g_HubDeviceName = "SangaboardHub";
const char* g_Keyword_Response = "SerialResponse";
const char* g_Keyword_Command = "SerialCommand";
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

class OpenFlexure : public  CXYStageBase<OpenFlexure>
{
public:
	OpenFlexure();
	~OpenFlexure();

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


	bool Busy() { return false; };
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


#endif 


