//////////////////////////////////////////////////////////////////////////////
// FILE:          OpenFlexure.cpp
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

#include "OpenFlexure.h"
#include "ModuleInterface.h"
#include <sstream>
#include <cstdio>
#include <cstring>
#include <string>
//#include "rapidjson/document.h"
//#include "rapidjson/writer.h"
//#include "rapidjson/stringbuffer.h"
#include <algorithm>
#include <math.h>

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

///////////////////////////////////////////////////////////////////////////////
// Exported MMDevice API
///////////////////////////////////////////////////////////////////////////////
MODULE_API void InitializeModuleData()
{
	RegisterDevice(g_Keyword_DeviceName, MM::XYStageDevice, "OpenFlexure XYStage");
}

MODULE_API MM::Device* CreateDevice(const char* deviceName)
{

	return new OpenFlexure;

}

MODULE_API void DeleteDevice(MM::Device* pDevice)
{
	delete pDevice;
}

///////////////////////////////////////////////////////////////////////////////
// OpenFlexure implementation
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~

/*
* Constructor, called when device is selected
*/
OpenFlexure::OpenFlexure() : initialized_(false), portAvailable_(false), stepSizeUm_(0.07)
{

	// Initialize default error messages
	InitializeDefaultErrorMessages();

	//pre initialization property: port name
	CPropertyAction* pAct = new CPropertyAction(this, &OpenFlexure::OnPort);
	CreateProperty(MM::g_Keyword_Port, "Undefined", MM::String, false, pAct, true);


}

/*
* Destructor
*/
OpenFlexure::~OpenFlexure()
{
	Shutdown();
}


/*
* Initialize the properties and attach the action pointers where needed
*/
int OpenFlexure::Initialize()
{
	if (initialized_) {
		return DEVICE_OK;
	}

	// Manual Command Interface
	CPropertyAction* pCommand = new CPropertyAction(this, &OpenFlexure::OnCommand);
	int ret = CreateProperty(g_Keyword_Command, "", MM::String, false, pCommand);
	assert(DEVICE_OK == ret);

	// Most Recent Serial Response
	ret = CreateProperty(g_Keyword_Response, "", MM::String, false);
	assert(DEVICE_OK == ret);


	// Check status to see if device is ready to start
	int status = UpdateStatus();
	if (status != DEVICE_OK) {
		return status;
	}

	// Use non-blocking moves
	ret = SendSerialCommand(port_.c_str(), "blocking_moves false", "\n");
	GetSerialAnswer(port_.c_str(), "\r", _serial_answer);
	if (_serial_answer.compare("done.") != 0) {
		return DEVICE_ERR;
	}

	

	return DEVICE_OK; 
}



int OpenFlexure::GetPositionUm(double& x, double& y)
{
	long stepsY, stepsX;

	// Get the steps
	int ret = GetPositionSteps(stepsY, stepsX);

	if (ret != DEVICE_OK) {
		return ret;
	}  

	// Convert the steps into um 
	x = stepsX * stepSizeUm_;
	y = stepsY * stepSizeUm_;  

	return DEVICE_OK;
}

int OpenFlexure::SetPositionSteps(long x, long y)
{


	return DEVICE_OK;
}


/**
* Called when device is starting. 
*/
int OpenFlexure::GetPositionSteps(long& x, long& y)
{
	while (Busy()); // Don't show the steps until stage stops moving


	// Query for the current position (x, y, z) of the stage
	int ret = SendSerialCommand(port_.c_str(), "p", "\n");

	if (ret != DEVICE_OK) {
		return ret;
	}

	// Get Answer
	GetSerialAnswer(port_.c_str(), "\r", _serial_answer);

	std::istringstream iss(_serial_answer);

	iss >> x;

	iss >> y;

	return DEVICE_OK;

}


int OpenFlexure::SetRelativePositionSteps(long x, long y)
{

	// sending two commands sequentially
	std::ostringstream cmd;
	cmd << "mrx " << x << "\nmry " << y; // move in x first then y (arbitrary choice)
	int ret = SendSerialCommand(port_.c_str(), cmd.str().c_str(), "\n");


	//std::ostringstream move_x;
	//std::ostringstream move_y;

	//move_x << "mrx " << x;
	//move_y << "mry " << y;

	//// move in x first (arbitrary choice)
	//int ret = SendSerialCommand(port_.c_str(), move_x.str().c_str(), "\n");

	//if (ret != DEVICE_OK) {
	//	return ret;
	//}

	//// move in y second
	//ret = SendSerialCommand(port_.c_str(), move_y.str().c_str(), "\n");

	// Get Answer
	std::string answer;
	GetSerialAnswer(port_.c_str(), "\r", answer);

	if (ret != DEVICE_OK) {
		return ret;
	}


	return DEVICE_OK;
}


int OpenFlexure::SetOrigin()
{
	while (Busy()); // make sure stage is not busy

	// Set current position as origin (all motor positions set to 0)
	int ret = SendSerialCommand(port_.c_str(), "zero", "\n");

	if (ret != DEVICE_OK) {
		return ret;
	}

	return DEVICE_OK;
}


int OpenFlexure::SetAdapterOrigin()
{
	return DEVICE_OK;
}


int OpenFlexure::Home()
{
	//TODO: Query the position steps and set the number of steps to opposite that
	return DEVICE_OK;
}


int OpenFlexure::Stop()
{
	// send the stop command to the stage
	int ret = SendSerialCommand(port_.c_str(), "stop", "\n");

	if (ret != DEVICE_OK) {
		return ret;
	}

	// TODO: maybe check for a response?

	return DEVICE_OK;

}


int OpenFlexure::GetStepLimits(long& xMin, long& xMax, long& yMin, long& yMax)
{
	return DEVICE_OK;
}
int OpenFlexure::GetLimitsUm(double& xMin, double& xMax, double& yMin, double& yMax)
{
	return DEVICE_OK;
}

//////////////// Action functions

/*
* Set the port to be used
*/
int OpenFlexure::OnPort(MM::PropertyBase* pProp, MM::ActionType pAct)
{
	if (pAct == MM::BeforeGet)
	{
		pProp->Set(port_.c_str());
	}
	else if (pAct == MM::AfterSet)
	{
		pProp->Get(port_);
		portAvailable_ = true;
	}

	return DEVICE_OK;
}

/*
Send a command directly to the stage
*/
int OpenFlexure::OnCommand(MM::PropertyBase* pProp, MM::ActionType pAct)
{
	if (pAct == MM::BeforeGet)
	{
		pProp->Set(_command.c_str());
	}
	else if (pAct == MM::AfterSet)
	{
		// Get command string
		pProp->Get(_command);

		// Append terminator
		_command += "\n";

		// Purge COM Port
		PurgeComPort(port_.c_str());

		// Send command
		WriteToComPort(port_.c_str(), (unsigned char*)_command.c_str(), (unsigned int)_command.length());

		// Get Answer
		std::string answer;
		GetSerialAnswer(port_.c_str(), "\r", answer);

		// Set property
		SetProperty(g_Keyword_Response, answer.c_str());
		//SetProperty(g_Keyword_Response, std::to_string((long long)answer.length()).c_str());
	}

	// Return
	// Search for error
	std::string error_flag("ERROR");
	if (_serial_answer.find(error_flag) != std::string::npos)
		return DEVICE_ERR;
	else
		return DEVICE_OK;

}



//////////// Helper Functions


/*
* Make sure no moves are in progress
*/
bool OpenFlexure::Busy()
{

	MM::MMTime timeout(0, 1000000); // wait for 1sec

	// Send a query to check if stage is moving
	int ret = SendSerialCommand(port_.c_str(), "moving?", "\n");

	// Check response
	GetSerialAnswer(port_.c_str(), "\r", _serial_answer);

	if (_serial_answer.compare("true") == 0) {
		return true;
	}
	else {
		return false;
	}
}

/*
* Pass a copy of the device name to the location given in the parameters
*/
void OpenFlexure::GetName(char* name) const
{
	CDeviceUtils::CopyLimitedString(name, "g_Keyword_DeviceName");
}

/*
* Shutdown the OpenFlexure
*/
int OpenFlexure::Shutdown()
{
	initialized_ = false;
	return DEVICE_OK;
}



