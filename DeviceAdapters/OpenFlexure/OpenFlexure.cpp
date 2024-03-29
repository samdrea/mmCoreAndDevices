//////////////////////////////////////////////////////////////////////////////
// FILE:          OpenFlexure.cpp
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
	RegisterDevice(g_XYStageDeviceName, MM::XYStageDevice, "OpenFlexure XYStage");
	RegisterDevice(g_HubDeviceName, MM::HubDevice, "Sangaboard Hub");
}

MODULE_API MM::Device* CreateDevice(const char* deviceName)
{
	if (deviceName == 0)
		return 0;

	if (strcmp(deviceName, g_XYStageDeviceName) == 0)
	{
		return new OpenFlexure; // Create xy stage
	}
	else if (strcmp(deviceName, g_HubDeviceName) == 0)
	{
		return new SangaBoardHub; // Create hub
	}
	
	return 0; // Device name not recognized

}

MODULE_API void DeleteDevice(MM::Device* pDevice)
{
	delete pDevice;
}

///////////////////////////////////////////////////////////////////////////////
// Sangaboard hub implementation
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~

/*
* Constructor
*/
SangaBoardHub::SangaBoardHub() : initialized_(false), port_("Undefined"), portAvailable_(false), busy_(false)
{
	// Initialize default error messages
	InitializeDefaultErrorMessages();

	// Pre-initialization property: port name
	CPropertyAction* pAct = new CPropertyAction(this, &SangaBoardHub::OnPort);
	CreateProperty(MM::g_Keyword_Port, "Undefined", MM::String, false, pAct, true);
}

/*
* Destructor
*/
SangaBoardHub::~SangaBoardHub()
{
	Shutdown();
}



int SangaBoardHub::Initialize()
{
	initialized_ = true;

	// TODO: Add some properties??

	return DEVICE_OK;

}

/*
* Shutdown the hub
*/
int SangaBoardHub::Shutdown()
{
	initialized_ = false;
	return DEVICE_OK;
}

/** Comment from MMDevice.h
* 
* Instantiate all available child peripheral devices.
*
* The implementation must instantiate all available child devices and
* register them by calling AddInstalledDevice() (currently in HubBase).
*
* Instantiated peripherals are owned by the Core, and will be destroyed
* by calling the usual ModuleInterface DeleteDevice() function.
*
* The result of calling this function more than once for a given hub
* instance is undefined.
*/
int SangaBoardHub::DetectInstalledDevices()
{

	ClearInstalledDevices();

	// make sure this method is called before we look for available devices
	//InitializeModuleData();

	char hubName[MM::MaxStrLength];
	GetName(hubName); // this device name
	for (unsigned i = 0; i < GetNumberOfDevices(); i++)
	{
		char deviceName[MM::MaxStrLength];
		bool success = GetDeviceName(i, deviceName, MM::MaxStrLength);
		if (success && (strcmp(hubName, deviceName) != 0))
		{
			MM::Device* pDev = CreateDevice(deviceName);
			AddInstalledDevice(pDev);
		}
	}

	return DEVICE_OK;
}


void SangaBoardHub::GetName(char* name) const
{
	CDeviceUtils::CopyLimitedString(name, g_HubDeviceName);
}

bool SangaBoardHub::Busy()
{
	return false;
}


///////////////////////////////////////////////////////////////////////////////
// Action handlers
///////////////////////////////////////////////////////////////////////////////
/*
 * Sets the Serial Port to be used.
 * Should be called before initialization
 */
int SangaBoardHub::OnPort(MM::PropertyBase* pProp, MM::ActionType eAct)
{
	if (eAct == MM::BeforeGet)
	{
		pProp->Set(port_.c_str());
	}
	else if (eAct == MM::AfterSet)
	{
		pProp->Get(port_);
		portAvailable_ = true;
	}

	return DEVICE_OK;
}



/////// Helper Functions

void SangaBoardHub::GetPort(std::string& port)
{
	port = this->port_;

}



///////////////////////////////////////////////////////////////////////////////
// OpenFlexure implementation
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~

/*
* Constructor, called when device is selected
*/
OpenFlexure::OpenFlexure() : initialized_(false), portAvailable_(false), stepSizeUm_(0.07), stepsX_(0), stepsY_(0)
{
	// Parent ID display
	CreateHubIDProperty();

	// Initialize default error messages
	InitializeDefaultErrorMessages();

	//pre initialization property: port name
	//CPropertyAction* pAct = new CPropertyAction(this, &OpenFlexure::OnPort);
	//CreateProperty(MM::g_Keyword_Port, "Undefined", MM::String, false, pAct, true);


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


	// Create pointer to parent hub (sangaboard)
	SangaBoardHub* pHub = static_cast<SangaBoardHub*>(GetParentHub());
	if (pHub)
	{
		char hubLabel[MM::MaxStrLength];
		pHub->GetLabel(hubLabel);
		SetParentID(hubLabel); // for backward comp.
	}
	else
		LogMessage(NoHubError);

	// Set port name
	pHub->GetPort(this->port_);

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

	// Set the current stagePosition
	ret = SyncState();
	assert(DEVICE_OK == ret);

	// Device is now initialized
	initialized_ = true;
	

	return DEVICE_OK; 
}

/**
* Sync the starting position of the stage to the cached values in the adapter
*/
int OpenFlexure::SyncState()
{
	// Make sure stage has stopped moving
	while (Busy());

	// Query for the current position (x, y, z) of the stage
	int ret = SendSerialCommand(port_.c_str(), "p", "\n");

	if (ret != DEVICE_OK) {
		return ret;
	}

	// Get Answer
	GetSerialAnswer(port_.c_str(), "\r", _serial_answer);

	std::istringstream iss(_serial_answer);

	iss >> stepsX_;

	iss >> stepsY_;

	// Reflect the synch-ed state in display
	ret = OnXYStagePositionChanged(stepsX_ * stepSizeUm_, stepsY_ * stepSizeUm_);

	/**
	if (ret != DEVICE_OK) {
		return ret;
	}
	*/

	return DEVICE_OK;
}



/**
* Called by SetPositionUm()
* Probably best to call SetRelativePosition() to complete this function
*/
int OpenFlexure::SetPositionSteps(long x, long y)
{
	return DEVICE_OK;
}

/**
* Rewriting a function that I shouldn't have messed with...
* According to DeviceBase.h, updates cached xPos and yPos and then calls OnXYStagePositionChanged
* Calls SetPositionSteps, which is the function users should implement to actuate change in physical xy stage device
*/
int OpenFlexure::SetPositionUm(double posX, double posY)
{
	// Manually change the xPos and yPos
	// Not sure where this function is actually called...
	return DEVICE_OK;
}


/**
* Called when stage control GUI is opened
*/
int OpenFlexure::GetPositionUm(double& posX, double& posY)
{
	// Make sure stage isn't moving
	while (Busy());

	posX = stepsX_ * stepSizeUm_;
	posY = stepsY_ * stepSizeUm_;

	return DEVICE_OK;
}


/**
* Should be called by GetPositionUm(), but probably redundant, because I'm keeping stepsX_ and stepsY_ global variables
*/
int OpenFlexure::GetPositionSteps(long& x, long& y)
{
	// Make sure stage isn't moving
	while (Busy());

	x = stepsX_;
	y = stepsY_;

	return DEVICE_OK;
}

/**
* Rewriting a function that I shouldn't have messed with...
* Called when arrow key is pressed
* Calls on SetRelativePositionSteps to actuate change in physical device
* Updates OnXYStagePositionChanged with xPos and yPos
*/
int OpenFlexure::SetRelativePositionUm(double dx, double dy)
{
	// Don't allow simultaneous moving
	while (Busy());

	long dxSteps = nint(dx / stepSizeUm_);
	long dySteps = nint(dy / stepSizeUm_);
	int ret = SetRelativePositionSteps(dxSteps, dySteps); // Stage starts moving after this step

	if (ret == DEVICE_OK) {
		stepsX_ += dxSteps;
		stepsY_ += dySteps;
		this->OnXYStagePositionChanged(stepsX_ * stepSizeUm_, stepsY_ * stepSizeUm_);
	}

	return DEVICE_OK;
}

/**
* According to DeviceBase.h, it uses GetPositionSteps() and then SetPositionSteps()
* Since OpenFlexure has a specific function for setting relative position, I'm actualizing the xy stage device here
*/
int OpenFlexure::SetRelativePositionSteps(long x, long y)
{
	// Don't allow simultaneous moving
	while (Busy());

	// Sending two commands sequentially
	std::ostringstream cmd;
	cmd << "mrx " << x << "\nmry " << y; // move in x first then y (arbitrary choice)
	int ret = SendSerialCommand(port_.c_str(), cmd.str().c_str(), "\n");

	if (ret != DEVICE_OK) {
		return ret;
	}

	return DEVICE_OK;
}


int OpenFlexure::SetOrigin()
{
	// Wait for stage to stop moving
	while (Busy());

	// Set current position as origin (all motor positions set to 0)
	int ret = SendSerialCommand(port_.c_str(), "zero", "\n");

	if (ret != DEVICE_OK) {
		return ret;
	}

	return DEVICE_OK;
}


/**
* Could be the function to sync adapter to the stage's actual positions... if fails, I'll implement a sync() myself
*/
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

	// Make sure current position is synched
	SyncState();

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
//int OpenFlexure::OnPort(MM::PropertyBase* pProp, MM::ActionType pAct)
//{
//	if (pAct == MM::BeforeGet)
//	{
//		pProp->Set(port_.c_str());
//	}
//	else if (pAct == MM::AfterSet)
//	{
//		pProp->Get(port_);
//		portAvailable_ = true;
//	}
//
//	return DEVICE_OK;
//}

/*
Send a command directly to the stage
*/
int OpenFlexure::OnCommand(MM::PropertyBase* pProp, MM::ActionType pAct)
{
	if (pAct == MM::BeforeGet)
	{
		pProp->Set(_command.c_str());
		
		// Sync the position
		//SyncState();
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

		// Sync the position
		SyncState();
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

	MM::MMTime timeout(0, 500000); // wait for 0.5 sec

	// Send a query to check if stage is moving
	int ret = SendSerialCommand(port_.c_str(), "moving?", "\n");

	// Check response
	GetSerialAnswer(port_.c_str(), "\r", _serial_answer); // Should return "\ntrue" or "\nfalse"

	if (_serial_answer.find("true") != -1) { // find() will return index of substring
		return true;
	} else {
		return false;
	}
}

/*
* Pass a copy of the device name to the location given in the parameters
*/
void OpenFlexure::GetName(char* name) const
{
	CDeviceUtils::CopyLimitedString(name, g_XYStageDeviceName);
}

/*
* Shutdown the OpenFlexure
*/
int OpenFlexure::Shutdown()
{
	initialized_ = false;
	return DEVICE_OK;
}



