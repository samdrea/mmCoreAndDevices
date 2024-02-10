//////////////////////////////////////////////////////////////////////////////
// FILE:          OpenFlexure.h
// PROJECT:       Micro-Manager
// SUBSYSTEM:     DeviceAdapters
//-----------------------------------------------------------------------------
// DESCRIPTION:   Adapter for illuminate LED controller firmware
//                Needs accompanying firmware to be installed on the LED Array:
//                https://github.com/zfphil/illuminate
//
// COPYRIGHT:     Samdrea Hsu
// LICENSE:       BSD3
//
// AUTHOR:        Samdrea Hsu, samdreahsu@berkeley.edu, 8/9/2023
//
//////////////////////////////////////////////////////////////////////////////


#ifndef _ILLUMINATE_H_
#define _ILLUMINATE_H_

#include "MMDevice.h"
#include "DeviceBase.h"
#include <string>
#include <map>

// Global keywords to be called multiple times
const char* g_Keyword_DeviceName = "OpenFlexure";
//const char* g_Keyword_Reset = "Reset";
const char* g_Keyword_Response = "SerialResponse";
const char* g_Keyword_Command = "SerialCommand";


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
	//int SetPositionUm(double x, double y);
	int GetPositionUm(double& x, double& y);
	int SetRelativePositionSteps(long x, long y);
	int SetOrigin();
	int SetAdapterOrigin();
	int Home();
	int Stop();
	int GetStepLimits(long& xMin, long& xMax, long& yMin, long& yMax);
	int GetLimitsUm(double& xMin, double& xMax, double& yMin, double& yMax);
	double GetStepSizeXUm() { return stepSizeUm_; }
	double GetStepSizeYUm() { return stepSizeUm_; }
	int IsXYStageSequenceable(bool& isSequenceable) const { isSequenceable = false; return DEVICE_OK; }


	bool Busy();
	void GetName(char*) const;

	// Action Interface
	int OnPort(MM::PropertyBase* pPropt, MM::ActionType eAct);
	int OnCommand(MM::PropertyBase* pPropt, MM::ActionType eAct);
	//int OnReset(MM::PropertyBase* pPropt, MM::ActionType eAct);

	// Action functions
	//int Reset();
	int SendCommand(const char* command, bool get_response);
	int GetResponse();

private:

	// Variables for manipulating the LED array
	bool initialized_;
	bool portAvailable_;
	double stepSizeUm_;
	std::string port_;
	MMThreadLock lock_;
	std::string _command;
	std::string _serial_answer;


	bool IsPortAvailable() { return portAvailable_; }

};


#endif 


