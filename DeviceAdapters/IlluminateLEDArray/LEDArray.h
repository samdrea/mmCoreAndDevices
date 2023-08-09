//////////////////////////////////////////////////////////////////////////////
// FILE:          LedArray.h
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


#ifndef _ILLUMINATE_H_
#define _ILLUMINATE_H_

#include "MMDevice.h"
#include "DeviceBase.h"
#include <string>
#include <map>

// Global keywords to be called multiple times
const char* g_Keyword_DeviceName = "SamsLights";
//const char* g_Keyword_RedValue = "RedValue";
//const char* g_Keyword_GreenValue = "GreenValue";
//const char* g_Keyword_BlueValue = "BlueValue";
const char* g_Keyword_Brightness = "Brightness";
const char* g_Keyword_Pattern = "IlluminationPattern";
const char* g_Keyword_Reset = "Reset";
const char* g_Keyword_Response = "SerialResponse";
const char* g_Keyword_Command = "SerialCommand";
const char* g_Pattern_None = "None";
const char* g_Pattern_SmileyFace = "SamsFace";


class LedArray: public  CGenericBase<LedArray>
{
public:
   LedArray();
   ~LedArray();
  
   // MMDevice API
   // ------------
   int Initialize();
   int Shutdown();

   bool Busy();
   void GetName(char *) const;

   // Action Interface
   int OnPort(MM::PropertyBase* pPropt, MM::ActionType eAct);
   // int OnCommand(MM::PropertyBase* pPropt, MM::ActionType eAct);
   int OnPattern(MM::PropertyBase* pPropt, MM::ActionType eAct);
   int OnBrightness(MM::PropertyBase* pPropt, MM::ActionType eAct);
   int OnReset(MM::PropertyBase* pPropt, MM::ActionType eAct);

   // Action functions
   int SetBrightness(long brightness);
   int UpdatePattern();
   int SyncState();
   int Reset();
   int SendCommand(const char* command, bool get_response);
   int GetResponse();

private:
	
	// Variables for manipulating the LED array
	bool _initialized;
	bool _portAvailable;
	std::string _port;
	MMThreadLock _lock;
	std::string _pattern;
	std::string _command;
	std::string _serial_answer;
	long color_r, color_g, color_b, brightness;
	
	bool IsPortAvailable() { return _portAvailable; }

};


#endif 
