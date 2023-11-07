//////////////////////////////////////////////////////////////////////////////
// FILE:          LedArray.cpp
// PROJECT:       Micro-Manager
// SUBSYSTEM:     DeviceAdapters
//-----------------------------------------------------------------------------
// DESCRIPTION:   Adapter for illuminate LED controller firmware
//                Needs accompanying firmware to be installed on the LED Array:
//                https://github.com/zfphil/illuminate
//
// COPYRIGHT:     Regents of the University of California
// LICENSE:       LGPL
//
// AUTHOR:        Samdrea Hsu, samdreahsu@berkeley.edu, 8/9/2023
//
//////////////////////////////////////////////////////////////////////////////

#include "LEDArray.h"
#include "ModuleInterface.h"
#include <sstream>
#include <cstdio>
#include <cstring>
#include <string>
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
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
	RegisterDevice(g_Keyword_DeviceName, MM::GenericDevice, "Sams test Adapter LED Array");
}
 
MODULE_API MM::Device* CreateDevice(const char* deviceName)
{

		return new LedArray;

}

MODULE_API void DeleteDevice(MM::Device* pDevice)
{
	delete pDevice;
}

///////////////////////////////////////////////////////////////////////////////
// LedArray implementation
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~

/*
* Constructor
*/
LedArray::LedArray() : _initialized(false), color_r(10), color_g(10), color_b(10), _portAvailable(false), brightness(10), _pattern(g_Pattern_None)
{

	// Initialize default error messages
	InitializeDefaultErrorMessages();

	//pre initialization property: port name
	CPropertyAction* pAct = new CPropertyAction(this, &LedArray::OnPort);
	CreateProperty(MM::g_Keyword_Port, "Undefined", MM::String, false, pAct, true);


}

/*
* Destructor
*/
LedArray::~LedArray()
{
	Shutdown();
}


/*
* Initialize the properties and attach the action pointers where needed
*/
int LedArray::Initialize()
{
	if (_initialized) {
		return DEVICE_OK;
	}

	// TODO: Change the reset property to a button
	CPropertyAction* pActreset = new CPropertyAction(this, &LedArray::OnReset);
	CreateProperty(g_Keyword_Reset, g_Pattern_None, MM::String, false, pActreset);
	AddAllowedValue(g_Keyword_Reset, g_Pattern_None);
	AddAllowedValue(g_Keyword_Reset, g_Keyword_Reset);

	// The illumination pattern property with drop down menu
	CPropertyAction* pActpat = new CPropertyAction(this, &LedArray::OnPattern);
	CreateProperty(g_Keyword_Pattern, g_Pattern_None, MM::String, false, pActpat);
	AddAllowedValue(g_Keyword_Pattern, g_Pattern_None);
	AddAllowedValue(g_Keyword_Pattern, g_Pattern_SmileyFace);

	// The brightness property with slider
	CPropertyAction* pActbr = new CPropertyAction(this, &LedArray::OnBrightness);
	CreateProperty(g_Keyword_Brightness, std::to_string((long long)brightness).c_str(), MM::Float, false, pActbr);
	SetPropertyLimits(g_Keyword_Brightness, 0, 255);

	// The property for the response from the Arduino
	CreateProperty(g_Keyword_Response, "", MM::String, false);

	// Make sure parameters in sync with Arduino settings
	//SyncState();

	// Reset the LED array
	Reset();

	// Check status to see if device is ready to start
	int status = UpdateStatus();
	if (status != DEVICE_OK) {
		return status;
	}

	return DEVICE_OK;
}

//////////////// Action functions

/*
* Set the brightness
*/
int LedArray::OnBrightness(MM::PropertyBase* pProp, MM::ActionType pAct)
{
	if (pAct == MM::BeforeGet)
	{
		pProp->Set(brightness); // set the property display in mm to the brightness variable stored on cache
	}
	else if (pAct == MM::AfterSet)
	{
		pProp->Get(brightness); // get value from mm property display and store as brightness variable on cache
		SetBrightness(brightness); // actually send command to set brightness of the LedArray
		UpdatePattern(); // reflect the changes in brightness immediately
	}
	return DEVICE_OK;
}

/*
* Set the port to be used
*/
int LedArray::OnPort(MM::PropertyBase* pProp, MM::ActionType pAct)
{
	if (pAct == MM::BeforeGet)
	{
		pProp->Set(_port.c_str());
	}
	else if (pAct == MM::AfterSet)
	{
		pProp->Get(_port);
		_portAvailable = true;
	}

	return DEVICE_OK;
}

/*
* Reset the LED array if the user chooses to reset
*/
int LedArray::OnReset(MM::PropertyBase* pProp, MM::ActionType pAct)
{
	if (pAct == MM::BeforeGet)
	{
		pProp->Set("None");
	}
	else if (pAct == MM::AfterSet)
	{
		Reset(); 
	}

	return DEVICE_OK;
}

/*
* Change pattern if pattern property changes
*/
int LedArray::OnPattern(MM::PropertyBase * pProp, MM::ActionType pAct)
{
	if (pAct == MM::BeforeGet)
	{
		pProp->Set(_pattern.c_str());
	}
	else if (pAct == MM::AfterSet)
	{
		pProp->Get(_pattern);
		return UpdatePattern();
	}
	return DEVICE_OK;
}


//////////// Helper Functions
/*
*  Query the Arduino for its current values and reflect those values in property
*/
int LedArray::SyncState()
{
	// Get current brightness
	SendCommand("sb", true);
	std::string brightness_str("SB."); // this is the start and end of the response that Arduino will send back?
	//brightness = (long)atoi(_serial_answer.substr(_serial_answer.find(brightness_str) + brightness_str.length(), _serial_answer.length() - brightness_str.length()).c_str());
	
	// Show the current brightness in property interface
	SetProperty(g_Keyword_Brightness, std::to_string((long long)brightness).c_str());

	return DEVICE_OK;
}


/*
* Allow other functions to send commands to the Arduino
*/
int LedArray::SendCommand(const char* command, bool get_response)
{
	// Purge COM port
	PurgeComPort(_port.c_str());

	// Covert command to std::string and store in the command var
	std::string _command(command);

	// Second command to device
	_command += "\n";
	WriteToComPort(_port.c_str(), &((unsigned char*)_command.c_str())[0], (unsigned int)_command.length());

	// Impose a small delay to prevent overloading buffer
	Sleep(30);

	// Check response
	if (get_response)
		return GetResponse();
	else
		return DEVICE_OK;

}

/*
* Recieve a response from the Arduino and display it in properties
*/
int LedArray::GetResponse()
{
	// Answer
	GetSerialAnswer(_port.c_str(), "-==-", _serial_answer);

	// Set Property
	SetProperty(g_Keyword_Response, _serial_answer.c_str());

	// Search for error
	if (_serial_answer.find("ERROR") != std::string::npos)
		return DEVICE_ERR;
	else
		return DEVICE_OK;
}

/*
*  Helps the OnBrightness function to send the command to Arduino
*/
int LedArray::SetBrightness(long brightness)
{
	// create a temporary string
	std::string command("sb.");

	// Apped the brightness value to it
	command += std::to_string((long long)brightness);

	// Send command and get the response
	SendCommand(command.c_str(), true);

	return DEVICE_OK;
}

/*
* Send the pattern selected to the Arduino
*/
int LedArray::UpdatePattern()
{
	if (_pattern == g_Pattern_SmileyFace)
	{
		// No need to update color because we never changed it
		SendCommand("l.24.21.22.29.27.31.23", true);
	}
	else
	{
		// if pattern is none
		SendCommand("x", true);
	}

	// return
	return DEVICE_OK;
}

/*
* Send the reset command to Arduino
*/
int LedArray::Reset()
{
	// Send the reset command and get response
	SendCommand("reset", true);

	// Return
	return DEVICE_OK;
}

/*
* Make LedArray available?
*/
bool LedArray::Busy()
{
	return false;
}

/*
* Pass a copy of the device name to the location given in the parameters
*/
void LedArray::GetName(char* name) const
{
	CDeviceUtils::CopyLimitedString(name, "g_Keyword_DeviceName");
}

/*
* Shutdown the LedArray
*/
int LedArray::Shutdown()
{
	_initialized = false;
	return DEVICE_OK;
}

 