///////////////////////////////////////////////////////////////////////////////
// FILE:          DemoCamera.cpp
// PROJECT:       Micro-Manager
// SUBSYSTEM:     DeviceAdapters
//-----------------------------------------------------------------------------
// DESCRIPTION:   The example implementation of the demo camera.
//                Simulates generic digital camera and associated automated
//                microscope devices and enables testing of the rest of the
//                system without the need to connect to the actual hardware. 
//                
// AUTHOR:        fandayu, fandayu@tucsen.com 2024
//
// COPYRIGHT:     Tucsen Photonics Co., Ltd., 2024
// LICENSE:       This file is distributed under the BSD license.
//                License text is included with the source distribution.
//
//                This file is distributed in the hope that it will be useful,
//                but WITHOUT ANY WARRANTY; without even the implied warranty
//                of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
//
//                IN NO EVENT SHALL THE COPYRIGHT OWNER OR
//                CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
//                INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES.

#include "MMTUCam.h"
#include <cstdio>
#include <string>
#include <math.h>
#include "ModuleInterface.h"
#include <sstream>
#include <algorithm>
#include "WriteCompactTiffRGB.h"
#include <iostream>
#include <process.h>

#include <shlwapi.h>
#pragma comment(lib, "shlwapi.lib")

using namespace std;
const double CMMTUCam::nominalPixelSizeUm_ = 1.0;
double g_IntensityFactor_ = 1.0;

// External names used used by the rest of the system
// to load particular device from the "DemoCamera.dll" library
const char* g_TUDeviceName  = "TUCam";
const char* g_PropNameFan   = "Fan";
const char* g_PropNamePCLK  = "PixelClock";
const char* g_PropNameBODP  = "BitDepth";
const char* g_PropNameGain  = "Gain";
const char* g_PropNameMode  = "Mode";
const char* g_PropNameFLPH  = "FlipH";
const char* g_PropNameFLPV  = "FlipV";
const char* g_PropNameGAMM  = "Image Adjustment Gamma";
const char* g_PropNameCONT  = "Image Adjustment Contrast";
const char* g_PropNameSATU  = "Image Adjustment Saturation";
const char* g_PropNameRGAN  = "Image Adjustment Channel R";
const char* g_PropNameGGAN  = "Image Adjustment Channel G";
const char* g_PropNameBGAN  = "Image Adjustment Channel B";
const char* g_PropNameATWB  = "Image Adjustment Auto White Balance";
const char* g_PropNameONWB  = "Image Adjustment Once White Balance";
const char* g_PropNameCLRTEMP = "Image Adjustment Color Temperature";
const char* g_PropNameATEXP = "Exposure_Auto Adjustment";
const char* g_PropNameTEMP  = "Temperature";
const char* g_PropNameLLev  = "Image Adjustment Left  Levels";
const char* g_PropNameRLev  = "Image Adjustment Right Levels";
const char* g_PropNameIFMT  = "SaveImage";
const char* g_PropNameReset = "Reset";
const char* g_PropNameCMS   = "CMSMode";  
const char* g_PropNameLED   = "LEDMode";  
const char* g_PropNameTEC   = "TECMode";
const char* g_PropNamePI    = "PIMode";
const char* g_PropNameDepth = "DepthMode"; 
const char* g_PropNameShutter = "Shutter Mode";
const char* g_PropNameMdTgr = "Trigger Mode"; 
const char* g_PropNameMdExp = "Trigger Exposure Mode";
const char* g_PropNameMdEdg = "Trigger Edge Mode"; 
const char* g_PropNameMdDly = "Trigger Delay";
const char* g_PropNameFilter= "Signal Filter";
const char* g_PropNameMdFrames = "Trigger Frames";
const char* g_PropNameMdTFrames = "Trigger Total Frames";
const char* g_PropNameDoSFW = "Trigger Software Do";
const char* g_PropNameSharp = "Image Adjustment Sharpness";
const char* g_PropNameDPC   = "Image Adjustment DPC";
const char* g_PropNameOffset= "Image Adjustment Offset";
const char* g_PropNameOTEnable = "Output Trigger Enable";
const char* g_PropNamePort  = "Output Trigger Port"; 
const char* g_PropNameKind  = "Output Trigger Kind";
const char* g_PropNameEdge  = "Output Trigger Edge Mode"; 
const char* g_PropNameDelay = "Output Trigger Delay";
const char* g_PropNameWidth = "Output Trigger Width";

const char* g_PropNameRSMode  = "Rollingshutter Status";
const char* g_PropNameRSLtd   = "Rollingshutter Line Time Delay";
const char* g_PropNameRSSlit  = "Rollingshutter Slit Height";
const char* g_PropNameRSDir   = "Rollingshutter Readout Direction";
const char* g_PropNameRSReset = "Rollingshutter Readout Direction Reset";
const char* g_PropNameRSLITm  = "Rollingshutter Rolling Speed";

const char* g_PropNameFrameRate = "Frame Rate";

const char* g_PropNameTestImg = "Test Image";

const char* g_PropNameBrightness = "Targeting Level"; //"Brightness";
const char* g_PropNamePixelRatio = "Metering Level";  //"Pixel Ratio";
const char* g_PropNameImgMetadata= "Image Metadata";  
const char* g_PropNameATExpMode  = "ATExposure Mode";

const char* g_PropNameBinningSum = "Binning Sum";

const char* g_DeviceName = "Dhyana";   //"400Li"
const char* g_SdkName = "TUCam";
//const char* g_SdkName = "CamCore";

//const char* g_DeviceName = "Dhyana400A";
//const char* g_SdkName = "CamKernel";

const char* g_Color = "Color Mode";
const char* g_Gray  = "Gray Mode";

const char* g_WB  = "Click WhiteBalance";

const char* g_AE_ON  = "On";
const char* g_AE_OFF = "Off";

const char* g_CMS_ON  = "On";
const char* g_CMS_OFF = "Off";

const char* g_LED_ON  = "On";
const char* g_LED_OFF = "Off";

const char* g_TEC_ON  = "On";
const char* g_TEC_OFF = "Off";

const char* g_PI_ON  = "On";
const char* g_PI_OFF = "Off";

const char* g_FAN_ON  = "On";
const char* g_FAN_OFF = "Off";

const char* g_OT_ON = "On";
const char* g_OT_OFF = "Off";

const char* g_CMSBIT_ON  = "CMS";
const char* g_HDRBIT_ON  = "HDR";
const char* g_HIGHBIT_ON = "HIGH";
const char* g_LOWBIT_ON  = "LOW";
const char* g_GRHIGH_ON  = "GLOBALRESETHIGH";
const char* g_GRLOW_ON   = "GLOBALRESETLOW";
const char* g_HSHIGH_ON  = "HIGHSPEEDHG";
const char* g_HSLOW_ON   = "HIGHSPEEDLG";
const char* g_STDHIGH_ON = "STDHIGH";
const char* g_STDLOW_ON  = "STDLOW";

const char* g_HIGHDYNAMIC_ON  = "High Dynamic";      /// HDR
const char* g_HIGHSPEED_ON    = "High Speed";        /// HighSpeedHg
const char* g_HIGHSENSITY_ON  = "High Sensitivity";  /// CMS
const char* g_GLOBALRESET_ON  = "Global Reset";      /// GlobalReset Hg

const char* g_TRIGGER_OFF = "Off";
const char* g_TRIGGER_STD = "Standard";
const char* g_TRIGGER_STDOVERLAP = "Standard(Overlap)";
const char* g_TRIGGER_STDNONOVERLAP = "Standard(Non-Overlap)";
const char* g_TRIGGER_CC1 = "CC1";
const char* g_TRIGGER_SYN = "Synchronization";
const char* g_TRIGGER_GLB = "Global";
const char* g_TRIGGER_SWF = "Software";

const char* g_TRIGGER_PORT1 = "1";
const char* g_TRIGGER_PORT2 = "2";
const char* g_TRIGGER_PORT3 = "3";

const char* g_TRIGGER_EXPSTART = "Exposure Start";
const char* g_TRIGGER_READEND  = "Readout End";
const char* g_TRIGGER_GLBEXP   = "Global Exposure";
const char* g_TRIGGER_TRIREADY = "Trigger Ready";
const char* g_TRIGGER_LOW      = "Low";
const char* g_TRIGGER_HIGH     = "High";

const char* g_TRIGGER_EXP_EXPTM    = "Timed";
const char* g_TRIGGER_EXP_WIDTH    = "Width";
const char* g_TRIGGER_EDGE_RISING  = "Rising";
const char* g_TRIGGER_EDGE_FALLING = "Falling";

const char* g_TRIGGER_DO_SOFTWARE  = "Exec";

const char* g_DPC_OFF    = "Off";
const char* g_DPC_LOW    = "Low";
const char* g_DPC_MEDIUM = "Medium";
const char* g_DPC_HIGH   = "High";

const char* g_Format_PNG = "PNG";
const char* g_Format_TIF = "TIF";
const char* g_Format_JPG = "JPG";
const char* g_Format_BMP = "BMP";
const char* g_Format_RAW = "RAW";
const char* g_FileName   = "\\Image";

// constants for naming pixel types (allowed values of the "PixelType" property)
const char* g_PixelType_8bit     = "8bit";
const char* g_PixelType_16bit    = "16bit";
const char* g_PixelType_32bitRGB = "32bitRGB";
const char* g_PixelType_64bitRGB = "64bitRGB";
const char* g_PixelType_32bit    = "32bit";  // floating point greyscale

///////////////////////////////////////////////////////////////////////////////
// Exported MMDevice API
///////////////////////////////////////////////////////////////////////////////

MODULE_API void InitializeModuleData()
{
   RegisterDevice(g_TUDeviceName, MM::CameraDevice, "TUCSEN Camera");
}

MODULE_API MM::Device* CreateDevice(const char* deviceName)
{
   if (deviceName == 0)
      return 0;

   // decide which device class to create based on the deviceName parameter
   if (strcmp(deviceName, g_TUDeviceName) == 0)
   {
      // create camera
      return new CMMTUCam();
   }

   // ...supplied name not recognized
   return 0;
}

MODULE_API void DeleteDevice(MM::Device* pDevice)
{
   delete pDevice;
}


int CMMTUCam::s_nNumCam  = 0;
int CMMTUCam::s_nCntCam  = 0;
///////////////////////////////////////////////////////////////////////////////
// CMMTUCam implementation
// ~~~~~~~~~~~~~~~~~~~~~~~~~~

/**
* CMMTUCam constructor.
* Setup default all variables and create device properties required to exist
* before intialization. In this case, no such properties were required. All
* properties will be created in the Initialize() method.
*
* As a general guideline Micro-Manager devices do not access hardware in the
* the constructor. We should do as little as possible in the constructor and
* perform most of the initialization in the Initialize() method.
*/
CMMTUCam::CMMTUCam() :
    CCameraBase<CMMTUCam> (),
    exposureMaximum_(10000.0),     
    exposureMinimum_(0.0),
    dPhase_(0),
    initialized_(false),
    readoutUs_(0.0),
    scanMode_(1),
    bitDepth_(8),
    roiX_(0),
    roiY_(0),
    sequenceStartTime_(0),
    isSequenceable_(false),
    sequenceMaxLength_(100),
    sequenceRunning_(false),
    sequenceIndex_(0),
    binSize_(1),
    cameraCCDXSize_(512),
    cameraCCDYSize_(512),
    ccdT_ (0.0),
    triggerDevice_(""),
    stopOnOverflow_(false),
    dropPixels_(false),
    fastImage_(false),
    saturatePixels_(false),
    fractionOfPixelsToDropOrSaturate_(0.002),
    shouldRotateImages_(false),
    shouldDisplayImageNumber_(false),
    stripeWidth_(1.0),
    nComponents_(1),
    returnToSoftwareTriggers_(false)
{
    memset(testProperty_,0,sizeof(testProperty_));

    // call the base class method to set-up default error codes/messages
    InitializeDefaultErrorMessages();
    readoutStartTime_ = GetCurrentMMTime();
    thd_ = new CTUCamThread(this);

    // parent ID display
//   CreateHubIDProperty();

    m_fCurTemp    = 0.0f;
    m_fValTemp    = 0.0f;

    m_bROI        = false;
    m_bSaving     = false;
    m_bLiving     = false;
    m_bTemping    = false;
    m_hThdWaitEvt = NULL;
    m_hThdTempEvt = NULL;

    m_frame.uiRsdSize   = 1;
    m_frame.ucFormatGet = TUFRM_FMT_USUAl;
    m_frame.pBuffer     = NULL;

	m_nZeroTemp = 50;
	m_nPID     = 0;
	m_nBCD     = 0;
    m_nIdxGain = 0;
	m_tgrOutAttr.nTgrOutPort = 0;   
	m_tgrOutAttr.nTgrOutMode = 5;  
	m_tgrOutAttr.nEdgeMode   = 0;   
	m_tgrOutAttr.nDelayTm    = 0;   
	m_tgrOutAttr.nWidth      = 5000;  

	m_tgrOutPara.nTgrOutPort= 0;
	m_tgrOutPara.TgrPort1.nTgrOutMode = 5;
	m_tgrOutPara.TgrPort1.nEdgeMode = 0;
	m_tgrOutPara.TgrPort1.nDelayTm = 0;
	m_tgrOutPara.TgrPort1.nWidth = 5000;


	m_tgrOutPara.TgrPort2.nTgrOutMode = 4;
	m_tgrOutPara.TgrPort2.nEdgeMode = 0;
	m_tgrOutPara.TgrPort2.nDelayTm = 0;
	m_tgrOutPara.TgrPort2.nWidth = 5000;

	m_tgrOutPara.TgrPort3.nTgrOutMode = 3;
	m_tgrOutPara.TgrPort3.nEdgeMode = 0;
	m_tgrOutPara.TgrPort3.nDelayTm = 0;
	m_tgrOutPara.TgrPort3.nWidth = 5000;

	m_rsPara.nMode         = 0;
	m_rsPara.nLTDelay      = 1;
	m_rsPara.nLTDelayMax   = 1;
	m_rsPara.nLTDelayMin   = 1;
	m_rsPara.nLTDelayStep  = 1;
	m_rsPara.nSlitHeight   = 1;
	m_rsPara.nSlitHeightMax = 1;
	m_rsPara.nSlitHeightMin = 1;
	m_rsPara.nSlitHeightStep = 1;
	m_rsPara.dbLineInvalTm = 1.0;
	m_nDriverType = 0;
	m_bCC1Support = false;
	m_nTriType    = TRITYPE_SMA;
	m_bTempEn     = true;
	m_bTriEn      = true;
	m_bOffsetEn   = false;
	m_bAcquisition = false;
}

/**
* CMMTUCam destructor.
* If this device used as intended within the Micro-Manager system,
* Shutdown() will be always called before the destructor. But in any case
* we need to make sure that all resources are properly released even if
* Shutdown() was not called.
*/
CMMTUCam::~CMMTUCam()
{
	if (m_hThdTempEvt != NULL)
	{
        m_bTemping = false;
		WaitForSingleObject(m_hThdTempEvt, INFINITE);	
		CloseHandle(m_hThdTempEvt);
		m_hThdTempEvt = NULL;
	}

    StopSequenceAcquisition();
	StopCapture();
	s_nCntCam--;

	if (s_nCntCam <= 0)
	{
		s_nCntCam = 0;
		UninitTUCamApi();
	}

    delete thd_;   
}

/**
* Obtains device name.
* Required by the MM::Device API.
*/
void CMMTUCam::GetName(char* name) const
{
    // Return the name used to referr to this device adapte
    CDeviceUtils::CopyLimitedString(name, g_TUDeviceName);
}

/**
* Intializes the hardware.
* Required by the MM::Device API.
* Typically we access and initialize hardware at this point.
* Device properties are typically created here as well, except
* the ones we need to use for defining initialization parameters.
* Such pre-initialization properties are created in the constructor.
* (This device does not have any pre-initialization properties)
*/
int CMMTUCam::Initialize()
{
    OutputDebugString("[Initialize]:Enter!\n");

    if (initialized_)
        return DEVICE_OK;

    DemoHub* pHub = static_cast<DemoHub*>(GetParentHub());

    if (pHub)
    {
        char hubLabel[MM::MaxStrLength];
        pHub->GetLabel(hubLabel);
        SetParentID(hubLabel); // for backward comp.
    }
    else
        LogMessage(NoHubError);

    // init camera api
    // -----------------
    int nRet = InitTUCamApi();
    if (DEVICE_OK != nRet)
    {
        return nRet;
    }

    TUCAM_Capa_SetValue(m_opCam.hIdxTUCam, TUIDC_VERCORRECTION, 0);

    // set property list
    // -----------------

    TUCAM_CAPA_ATTR  capaAttr;
    TUCAM_PROP_ATTR  propAttr;
    TUCAM_VALUE_INFO valInfo;

    // Name
    nRet = CreateStringProperty(MM::g_Keyword_Name, g_TUDeviceName, true);
    if (DEVICE_OK != nRet)
        return nRet;

    // Description
    nRet = CreateStringProperty(MM::g_Keyword_Description, "TUCSEN Camera Device Adapter", true);
    if (DEVICE_OK != nRet)
        return nRet;

    // CameraName
    // Get camera type
    valInfo.nID = TUIDI_CAMERA_MODEL;
    if (TUCAMRET_SUCCESS == TUCAM_Dev_GetInfo(m_opCam.hIdxTUCam, &valInfo))
    {
        nRet = CreateProperty(MM::g_Keyword_CameraName, valInfo.pText, MM::String, true);
        assert(nRet == DEVICE_OK);
    }
    else
        return DEVICE_NOT_SUPPORTED;


    // CameraID
    nRet = CreateProperty(MM::g_Keyword_CameraID, "V1.0", MM::String, true);
    assert(nRet == DEVICE_OK);

	// Get BCD 
	valInfo.nID = TUIDI_BCDDEVICE;
	if (TUCAMRET_SUCCESS == TUCAM_Dev_GetInfo(m_opCam.hIdxTUCam, &valInfo))
	{
		m_nBCD   = valInfo.nValue;
	}

	// Get Zero Temperature Value
	valInfo.nID = TUIDI_ZEROTEMPERATURE_VALUE;
	if (TUCAMRET_SUCCESS == TUCAM_Dev_GetInfo(m_opCam.hIdxTUCam, &valInfo))
	{
		m_nZeroTemp = valInfo.nValue;
	}

	// Get camera type
	valInfo.nID = TUIDI_PRODUCT;
	if (TUCAMRET_SUCCESS == TUCAM_Dev_GetInfo(m_opCam.hIdxTUCam, &valInfo))
	{
		m_nPID   = valInfo.nValue;
	}

	if (DHYANA_201D == m_nPID || DHYANA_401D == m_nPID)
	{
		m_bTempEn = false;
		m_nTriType = TRITYPE_HR;
	}

	if (PID_FL_20BW == m_nPID)
	{
		m_nTriType = TRITYPE_HR;
	}

	if (DHYANA_400D_X45 == m_nPID || DHYANA_D95_X45 == m_nPID || DHYANA_400DC_X45 == m_nPID || DHYANA_400DC_X100 == m_nPID)
	{
		m_bTriEn = false;
	}

    if (PID_FL_9BW == m_nPID || PID_FL_9BW_LT == m_nPID || PID_FL_26BW == m_nPID || PID_FL_20BW == m_nPID || DHYANA_4040V2 == m_nPID || DHYANA_4040BSI == m_nPID || DHYANA_XF4040BSI == m_nPID)
	{
		m_bOffsetEn = true;
	}

	// Get driver type
	valInfo.nID = TUIDI_BUS;
	if (TUCAMRET_SUCCESS == TUCAM_Dev_GetInfo(m_opCam.hIdxTUCam, &valInfo))
	{
		if (0x200 == valInfo.nValue || 0x210 == valInfo.nValue)  /// USB2.0
		{
			m_nDriverType = TU_USB2_DRIVER;
		}
		else if (0x03 == valInfo.nValue)  /// FireBird
		{
			m_nDriverType = TU_PHXCAMERALINK;
		}
		else if (0x04 == valInfo.nValue)  /// Euresys
		{
			m_nDriverType = TU_EURESYSCAMERALINK;
		}
		else    /// USB3.0
		{
			m_nDriverType = TU_USB3_DRIVER;
		}
	}

	if (TRITYPE_HR == m_nTriType)
	{
		m_tgrOutPara.TgrPort1.nTgrOutMode = 3;
		m_tgrOutPara.TgrPort2.nTgrOutMode = 5;
	}

    // binning
    CPropertyAction *pAct = new CPropertyAction (this, &CMMTUCam::OnBinning);
    nRet = CreateProperty(MM::g_Keyword_Binning, "", MM::String, false, pAct);
    assert(nRet == DEVICE_OK);

    nRet = SetAllowedBinning();
    if (nRet != DEVICE_OK)
        return nRet;

    if (PID_FL_26BW == m_nPID)
    {
        // binning sum
        pAct = new CPropertyAction(this, &CMMTUCam::OnBinningSum);
        nRet = CreateProperty(g_PropNameBinningSum, "", MM::String, false, pAct);
        assert(nRet == DEVICE_OK);

        nRet = SetAllowedBinningSum();
        if (nRet != DEVICE_OK)
            return nRet;
    }

    // Bit depth
    capaAttr.idCapa = TUIDC_BITOFDEPTH;
    if (TUCAMRET_SUCCESS == TUCAM_Capa_GetAttr(m_opCam.hIdxTUCam, &capaAttr))
    {
		if(capaAttr.nValMax != capaAttr.nValMin && m_nPID != DHYANA_D95_X100)
		{
			if (capaAttr.nValMax > 8)
			{
				if (DHYANA_400DC_X100 == m_nPID || DHYANA_400DC_X45 == m_nPID)
				{
					TUCAM_Capa_SetValue(m_opCam.hIdxTUCam, TUIDC_BITOFDEPTH, 8);
				}

			pAct = new CPropertyAction (this, &CMMTUCam::OnBitDepth);
			nRet = CreateProperty(g_PropNameBODP, "8", MM::String, false, pAct);
			assert(nRet == DEVICE_OK);

			vector<string> bitDepths;
			bitDepths.push_back("8");
			bitDepths.push_back("16");

				nRet = SetAllowedValues(g_PropNameBODP, bitDepths);
				if (nRet != DEVICE_OK)
					return nRet;
			}
			else
			{
				pAct = new CPropertyAction(this, &CMMTUCam::OnBitDepthEum);
				nRet = CreateProperty(g_PropNameBODP, "", MM::String, false, pAct);
				SetAllowedDepth();
				if (nRet != DEVICE_OK)
					return nRet;
			}
		}
    } 

    // Pixels clock
    capaAttr.idCapa = TUIDC_PIXELCLOCK;
    if (TUCAMRET_SUCCESS == TUCAM_Capa_GetAttr(m_opCam.hIdxTUCam, &capaAttr))
    {
        pAct = new CPropertyAction (this, &CMMTUCam::OnPixelClock);
        nRet = CreateProperty(g_PropNamePCLK, "High", MM::String, false, pAct);
        assert(nRet == DEVICE_OK);

        nRet = SetAllowedPixelClock();
        if (nRet != DEVICE_OK)
            return nRet;

        SetProperty(g_PropNamePCLK, "High");
    }

    // Exposure
	propAttr.nIdxChn = 0;
	propAttr.idProp = TUIDP_EXPOSURETM;
	if (TUCAMRET_SUCCESS == TUCAM_Prop_GetAttr(m_opCam.hIdxTUCam, &propAttr))
	{
		pAct = new CPropertyAction(this, &CMMTUCam::OnExposure);
		nRet = CreateProperty(MM::g_Keyword_Exposure, "10.0", MM::Float, false, pAct);
		assert(nRet == DEVICE_OK);

		UpdateExpRange();
	}

	// Brightness
	propAttr.nIdxChn = 0;
	propAttr.idProp = TUIDP_BRIGHTNESS;
	if (TUCAMRET_SUCCESS == TUCAM_Prop_GetAttr(m_opCam.hIdxTUCam, &propAttr))
	{
		pAct = new CPropertyAction(this, &CMMTUCam::OnBrightness);
		nRet = CreateProperty(g_PropNameBrightness, "0", MM::Integer, false, pAct);
		assert(nRet == DEVICE_OK);

		SetPropertyLimits(g_PropNameBrightness, propAttr.dbValMin, propAttr.dbValMax);
	}

	// PixelRatio
	propAttr.nIdxChn = 0;
	propAttr.idProp = TUIDP_PIXELRATIO;
	if (TUCAMRET_SUCCESS == TUCAM_Prop_GetAttr(m_opCam.hIdxTUCam, &propAttr))
	{
		pAct = new CPropertyAction(this, &CMMTUCam::OnPixelRatio);
		nRet = CreateProperty(g_PropNamePixelRatio, "0", MM::Integer, false, pAct);
		assert(nRet == DEVICE_OK);

		SetPropertyLimits(g_PropNamePixelRatio, propAttr.dbValMin, propAttr.dbValMax);
	}

    // Global Gain
    propAttr.nIdxChn= 0;
    propAttr.idProp = TUIDP_GLOBALGAIN;
    if (TUCAMRET_SUCCESS == TUCAM_Prop_GetAttr(m_opCam.hIdxTUCam, &propAttr))
    {
        if (propAttr.dbValMax > 5)
        {
            pAct = new CPropertyAction (this, &CMMTUCam::OnGlobalGain);
            nRet = CreateProperty(g_PropNameGain, "1", MM::Integer, false, pAct);
            assert(nRet == DEVICE_OK);

            SetPropertyLimits(g_PropNameGain, propAttr.dbValMin, propAttr.dbValMax);
        }
        else
        {
			if (m_nPID == PID_FL_9BW || PID_FL_9BW_LT == m_nPID || PID_FL_26BW == m_nPID || IsSupportAries16())
            {
                int nCnt = (int)(propAttr.dbValMax - propAttr.dbValMin + 1);

                char szBuf[64] = { 0 };
                TUCAM_VALUE_TEXT valText;
                valText.nTextSize = 64;
                valText.pText = &szBuf[0];
                valText.nID = TUIDP_GLOBALGAIN;

                vector<string>  gainValues;

                for (int i = 0; i<nCnt; ++i)
                {
                    valText.dbValue = i;
                    TUCAM_Prop_GetValueText(m_opCam.hIdxTUCam, &valText);

                    gainValues.push_back(string(valText.pText));
                }
                
                pAct = new CPropertyAction(this, &CMMTUCam::OnGlobalGainMode);
                nRet = CreateProperty(g_PropNameGain, gainValues.at(0).c_str(), MM::String, false, pAct);
                assert(nRet == DEVICE_OK);

                nRet = SetAllowedValues(g_PropNameGain, gainValues);
            }
            else
            {
                nRet = SetAllowedGainMode();
                if (nRet != DEVICE_OK)
                    return nRet;
            }
        }
    }

	// FrameRate
	propAttr.nIdxChn = 0;
	propAttr.idProp = TUIDP_FRAME_RATE;
	if (TUCAMRET_SUCCESS == TUCAM_Prop_GetAttr(m_opCam.hIdxTUCam, &propAttr))
	{
		pAct = new CPropertyAction(this, &CMMTUCam::OnFrameRate);
		nRet = CreateProperty(g_PropNameFrameRate, "100.0", MM::Float, false, pAct);
		assert(nRet == DEVICE_OK);
	}

	// Auto Exposure
	capaAttr.idCapa = TUIDC_ENABLETIMESTAMP;
	if (TUCAMRET_SUCCESS == TUCAM_Capa_GetAttr(m_opCam.hIdxTUCam, &capaAttr))
	{
		if (IsSupport401DNew() || IsSupport95V2New() || IsSupport400BSIV3New())
		{
			pAct = new CPropertyAction(this, &CMMTUCam::OnTimeStamp);
			nRet = CreateProperty(g_PropNameImgMetadata, "FALSE", MM::String, false, pAct);
			assert(nRet == DEVICE_OK);

			vector<string> StampValues;
			StampValues.push_back("FALSE");
			StampValues.push_back("TRUE");

			nRet = SetAllowedValues(g_PropNameImgMetadata, StampValues);
			if (nRet != DEVICE_OK)
				return nRet;
		}
	}

	// Sensor reset
	capaAttr.idCapa = TUIDC_SENSORRESET;
	if (TUCAMRET_SUCCESS == TUCAM_Capa_GetAttr(m_opCam.hIdxTUCam, &capaAttr))
	{
		pAct = new CPropertyAction(this, &CMMTUCam::OnSensorReset);
		nRet = CreateStringProperty(g_PropNameReset, "Reset", false, pAct);
		assert(nRet == DEVICE_OK);
		AddAllowedValue(g_PropNameReset, g_PropNameReset);
	}

	// Auto Exposure Mode
	capaAttr.idCapa = TUIDC_ATEXPOSURE_MODE;
	if (TUCAMRET_SUCCESS == TUCAM_Capa_GetAttr(m_opCam.hIdxTUCam, &capaAttr))
	{
		pAct = new CPropertyAction(this, &CMMTUCam::OnATExpMode);
		nRet = CreateProperty(g_PropNameATExpMode, "0", MM::Integer, false, pAct);
		assert(nRet == DEVICE_OK);

		SetPropertyLimits(g_PropNameATExpMode, capaAttr.nValMin, capaAttr.nValMax);
	}

    // Auto Exposure
    capaAttr.idCapa = TUIDC_ATEXPOSURE;
    if (TUCAMRET_SUCCESS == TUCAM_Capa_GetAttr(m_opCam.hIdxTUCam, &capaAttr))
    {
        pAct = new CPropertyAction (this, &CMMTUCam::OnATExposure);
        nRet = CreateProperty(g_PropNameATEXP, "FALSE", MM::String, false, pAct);
        assert(nRet == DEVICE_OK);

        vector<string> ATExpValues;
        ATExpValues.push_back("FALSE");
        ATExpValues.push_back("TRUE");

        nRet = SetAllowedValues(g_PropNameATEXP, ATExpValues);
        if (nRet != DEVICE_OK)
            return nRet;
    }

    // Flip Horizontal
    capaAttr.idCapa = TUIDC_HORIZONTAL;
    if (TUCAMRET_SUCCESS == TUCAM_Capa_GetAttr(m_opCam.hIdxTUCam, &capaAttr))
    {
        pAct = new CPropertyAction (this, &CMMTUCam::OnFlipH);
        nRet = CreateProperty(g_PropNameFLPH, "FALSE", MM::String, false, pAct);
        assert(nRet == DEVICE_OK);

        vector<string> HFlipValues;
        HFlipValues.push_back("FALSE");
        HFlipValues.push_back("TRUE");

        nRet = SetAllowedValues(g_PropNameFLPH, HFlipValues);
        if (nRet != DEVICE_OK)
            return nRet;
    }

    // Flip Vertical
    capaAttr.idCapa = TUIDC_VERTICAL;
    if (TUCAMRET_SUCCESS == TUCAM_Capa_GetAttr(m_opCam.hIdxTUCam, &capaAttr))
    {
        pAct = new CPropertyAction (this, &CMMTUCam::OnFlipV);
        nRet = CreateProperty(g_PropNameFLPV, "FALSE", MM::String, false, pAct);
        assert(nRet == DEVICE_OK);

        vector<string> VFlipValues;
        VFlipValues.push_back("FALSE");
        VFlipValues.push_back("TRUE");

        nRet = SetAllowedValues(g_PropNameFLPV, VFlipValues);
        if (nRet != DEVICE_OK)
            return nRet;
    }

    // Shutter
    if (PID_FL_26BW == m_nPID)
    {
        pAct = new CPropertyAction(this, &CMMTUCam::OnShutterMode);
        nRet = CreateProperty(g_PropNameShutter, "", MM::String, false, pAct);
        assert(nRet == DEVICE_OK);

        nRet = SetAllowedShutterMode();
        if (nRet != DEVICE_OK)
            return nRet;
    }              

    // Gamma
    propAttr.nIdxChn= 0;
    propAttr.idProp = TUIDP_GAMMA;
    if (TUCAMRET_SUCCESS == TUCAM_Prop_GetAttr(m_opCam.hIdxTUCam, &propAttr))
    {
        pAct = new CPropertyAction (this, &CMMTUCam::OnGamma);
        nRet = CreateProperty(g_PropNameGAMM, "100", MM::Integer, false, pAct);
        assert(nRet == DEVICE_OK);

        SetPropertyLimits(g_PropNameGAMM, propAttr.dbValMin, propAttr.dbValMax);
    }

    // Contrast
    propAttr.nIdxChn= 0;
    propAttr.idProp = TUIDP_CONTRAST;
    if (TUCAMRET_SUCCESS == TUCAM_Prop_GetAttr(m_opCam.hIdxTUCam, &propAttr))
    {
        pAct = new CPropertyAction (this, &CMMTUCam::OnContrast);
        nRet = CreateProperty(g_PropNameCONT, "128", MM::Integer, false, pAct);
        assert(nRet == DEVICE_OK);

        SetPropertyLimits(g_PropNameCONT, (int)propAttr.dbValMin, (int)propAttr.dbValMax);
    }

    // Saturation
    propAttr.nIdxChn= 0;
    propAttr.idProp = TUIDP_SATURATION;
    if (TUCAMRET_SUCCESS == TUCAM_Prop_GetAttr(m_opCam.hIdxTUCam, &propAttr))
    {
        pAct = new CPropertyAction (this, &CMMTUCam::OnSaturation);
        nRet = CreateProperty(g_PropNameSATU, "128", MM::Integer, false, pAct);
        assert(nRet == DEVICE_OK);

        SetPropertyLimits(g_PropNameSATU, propAttr.dbValMin, propAttr.dbValMax);
    }

    // White Balance
    capaAttr.idCapa = TUIDC_ATWBALANCE;
    if (TUCAMRET_SUCCESS == TUCAM_Capa_GetAttr(m_opCam.hIdxTUCam, &capaAttr))
    {
        pAct = new CPropertyAction (this, &CMMTUCam::OnWhiteBalance);

        if (2 == capaAttr.nValMax)
        {
            nRet = CreateProperty(g_PropNameATWB, "FALSE", MM::String, false, pAct);
            assert(nRet == DEVICE_OK);

            vector<string> WBLNValues;
            WBLNValues.push_back("FALSE");
            WBLNValues.push_back("TRUE");

            nRet = SetAllowedValues(g_PropNameATWB, WBLNValues);
            if (nRet != DEVICE_OK)
                return nRet;
        }
        else
        {
            nRet = CreateProperty(g_PropNameONWB, "Click", MM::String, false, pAct);
            assert(nRet == DEVICE_OK);

            vector<string> WBLNValues;
            WBLNValues.push_back("Click");

            nRet = SetAllowedValues(g_PropNameONWB, WBLNValues);
            if (nRet != DEVICE_OK)
                return nRet;
        }
    }
 
	// Color Temperature
	propAttr.nIdxChn = 0;
	propAttr.idProp = TUIDP_CLRTEMPERATURE;
	if (TUCAMRET_SUCCESS == TUCAM_Prop_GetAttr(m_opCam.hIdxTUCam, &propAttr))
	{
		CPropertyAction *pAct = new CPropertyAction(this, &CMMTUCam::OnClrTemp);
		nRet = CreateProperty(g_PropNameCLRTEMP, "2000K", MM::String, false, pAct);
		assert(nRet == DEVICE_OK);

		nRet = SetAllowedClrTemp();
		if (nRet != DEVICE_OK)
			return nRet;
	}

    // Red Channel Gain
    propAttr.nIdxChn= 1;
    propAttr.idProp = TUIDP_CHNLGAIN;
    if (TUCAMRET_SUCCESS == TUCAM_Prop_GetAttr(m_opCam.hIdxTUCam, &propAttr))
    {
        pAct = new CPropertyAction (this, &CMMTUCam::OnRedGain);
        nRet = CreateProperty(g_PropNameRGAN, "256", MM::Integer, false, pAct);
        assert(nRet == DEVICE_OK);

        SetPropertyLimits(g_PropNameRGAN, propAttr.dbValMin, propAttr.dbValMax);
    }

    // Green Channel Gain
    propAttr.nIdxChn= 2;
    propAttr.idProp = TUIDP_CHNLGAIN;
    if (TUCAMRET_SUCCESS == TUCAM_Prop_GetAttr(m_opCam.hIdxTUCam, &propAttr))
    {
        pAct = new CPropertyAction (this, &CMMTUCam::OnGreenGain);
        nRet = CreateProperty(g_PropNameGGAN, "256", MM::Integer, false, pAct);
        assert(nRet == DEVICE_OK);

        SetPropertyLimits(g_PropNameGGAN, propAttr.dbValMin, propAttr.dbValMax);
    }

    // Blue Channel Gain
    propAttr.nIdxChn= 3;
    propAttr.idProp = TUIDP_CHNLGAIN;
    if (TUCAMRET_SUCCESS == TUCAM_Prop_GetAttr(m_opCam.hIdxTUCam, &propAttr))
    {
        pAct = new CPropertyAction (this, &CMMTUCam::OnBlueGain);
        nRet = CreateProperty(g_PropNameBGAN, "256", MM::Integer, false, pAct);
        assert(nRet == DEVICE_OK);

        SetPropertyLimits(g_PropNameBGAN, propAttr.dbValMin, propAttr.dbValMax);
    }

	// Temperature
	propAttr.nIdxChn= 0;
	propAttr.idProp = TUIDP_TEMPERATURE;
	if (TUCAMRET_SUCCESS == TUCAM_Prop_GetAttr(m_opCam.hIdxTUCam, &propAttr) && m_bTempEn)  // For 401D 201D disable temperature
	{
		pAct = new CPropertyAction (this, &CMMTUCam::OnTemperature);

        if (propAttr.dbValMax > 100)
        {
            m_fScaTemp = 10;
            nRet = CreateProperty(g_PropNameTEMP, "0.0", MM::Float, false, pAct);
        }
        else
        {
            m_fScaTemp = 1;
            nRet = CreateProperty(g_PropNameTEMP, "0", MM::Integer, false, pAct);
        }	

		m_nMidTemp = m_nZeroTemp;
        if (PID_FL_9BW_LT == m_nPID)
        {
			///m_nMidTemp = 500;
            SetPropertyLimits(g_PropNameTEMP, (propAttr.dbValMin - m_nMidTemp) / m_fScaTemp, (propAttr.dbValMax - m_nMidTemp) / m_fScaTemp);
        }
        else
        {
            SetPropertyLimits(g_PropNameTEMP, -m_nMidTemp / m_fScaTemp, m_nMidTemp / m_fScaTemp);
        }
		
        char sz[64] = { 0 };
		switch (m_nPID)
		{
		case PID_FL_20BW:
			// Set default temperature
			if (TUCAMRET_SUCCESS == TUCAM_Prop_SetValue(m_opCam.hIdxTUCam, TUIDP_TEMPERATURE, 0))
			{
				sprintf(sz, "%d", -50);
				SetProperty(g_PropNameTEMP, sz);
			}
			break;
//		case DHYANA_400BSIV2:
//		case DHYANA_400BSIV3:
		case DHYANA_D95_V2:
		case DHYANA_4040V2:
		case DHYANA_4040BSI:
		case DHYANA_XF4040BSI:
			// Set default temperature
			sprintf(sz, "%d", ((int)propAttr.dbValDft - m_nMidTemp));
			SetProperty(g_PropNameTEMP, sz);
			break;
		default:
			// Set default temperature
            double dblTemp;
            if (TUCAMRET_SUCCESS == TUCAM_Prop_GetValue(m_opCam.hIdxTUCam, TUIDP_TEMPERATURE_TARGET, &dblTemp))
            {
                sprintf(sz, "%.1f", (dblTemp - m_nMidTemp) / m_fScaTemp);
                SetProperty(g_PropNameTEMP, sz);
            }
            else
            {
                if (TUCAMRET_SUCCESS == TUCAM_Prop_SetValue(m_opCam.hIdxTUCam, TUIDP_TEMPERATURE, 40.0f))
                {
				    sprintf(sz, "%d", -10);
                    SetProperty(g_PropNameTEMP, sz);
                }
            }

			break;
		}

		if (NULL == m_hThdTempEvt)
		{
			m_bTemping = true;
			m_hThdTempEvt = CreateEvent(NULL, TRUE, FALSE, NULL);
			_beginthread(GetTemperatureThread, 0, this);            // Start the get value of temperature thread
		}
	}

    // Left Levels
    propAttr.nIdxChn= 0;
    propAttr.idProp = TUIDP_LFTLEVELS;
    if (TUCAMRET_SUCCESS == TUCAM_Prop_GetAttr(m_opCam.hIdxTUCam, &propAttr))
    {
        pAct = new CPropertyAction (this, &CMMTUCam::OnLeftLevels);
        nRet = CreateProperty(g_PropNameLLev, "0", MM::Integer, false, pAct);
        assert(nRet == DEVICE_OK);

        SetPropertyLimits(g_PropNameLLev, propAttr.dbValMin, propAttr.dbValMax);
    }

    // Right Levels
    propAttr.nIdxChn= 0;
    propAttr.idProp = TUIDP_RGTLEVELS;
    if (TUCAMRET_SUCCESS == TUCAM_Prop_GetAttr(m_opCam.hIdxTUCam, &propAttr))
    {
        pAct = new CPropertyAction (this, &CMMTUCam::OnRightLevels);
        nRet = CreateProperty(g_PropNameRLev, "0", MM::Integer, false, pAct);
        assert(nRet == DEVICE_OK);

        SetPropertyLimits(g_PropNameRLev, propAttr.dbValMin, propAttr.dbValMax);
    }

    // Image format
    pAct = new CPropertyAction (this, &CMMTUCam::OnImageFormat);
	nRet = CreateStringProperty(g_PropNameIFMT, "RAW", false, pAct);
	assert(nRet == DEVICE_OK);

	AddAllowedValue(g_PropNameIFMT, g_Format_RAW);

    // CMS / CL Mode
    capaAttr.idCapa = TUIDC_IMGMODESELECT;
    if (TUCAMRET_SUCCESS == TUCAM_Capa_GetAttr(m_opCam.hIdxTUCam, &capaAttr))
    {
        if (PID_FL_9BW == m_nPID || PID_FL_9BW_LT == m_nPID)
        {
            nRet = SetAllowedGainMode();
        }
        else
        {
            int nImgMode = capaAttr.nValMax - capaAttr.nValMin + 1;

            if ((nImgMode < 0x3) && (capaAttr.nValMax < 0x02) && PID_FL_9BW != m_nPID && PID_FL_9BW_LT != m_nPID)
            {
                pAct = new CPropertyAction(this, &CMMTUCam::OnCMSMode);

			nRet = CreateProperty(g_PropNameCMS, g_CMS_ON, MM::String, false, pAct);
			assert(nRet == DEVICE_OK);
			vector<string>CMSValues;

			CMSValues.push_back(g_CMS_OFF);
			CMSValues.push_back(g_CMS_ON);

                nRet = SetAllowedValues(g_PropNameCMS, CMSValues);

            }
        }

        if (nRet != DEVICE_OK)
            return nRet;
    }

	// LED
	capaAttr.idCapa = TUIDC_LEDENBALE;
	if (TUCAMRET_SUCCESS == TUCAM_Capa_GetAttr(m_opCam.hIdxTUCam, &capaAttr))
	{
		pAct = new CPropertyAction (this, &CMMTUCam::OnLEDMode);

		nRet = CreateProperty(g_PropNameLED, g_LED_ON, MM::String, false, pAct);
		assert(nRet == DEVICE_OK);
		vector<string>LEDValues;

		LEDValues.push_back(g_LED_OFF);
		LEDValues.push_back(g_LED_ON);

		nRet = SetAllowedValues(g_PropNameLED, LEDValues);

		if (nRet != DEVICE_OK)
			return nRet;
	}

	// PI
	capaAttr.idCapa = TUIDC_ENABLEPI;
	if (TUCAMRET_SUCCESS == TUCAM_Capa_GetAttr(m_opCam.hIdxTUCam, &capaAttr))
	{
		pAct = new CPropertyAction(this, &CMMTUCam::OnPIMode);

		nRet = CreateProperty(g_PropNamePI, g_PI_ON, MM::String, false, pAct);
		assert(nRet == DEVICE_OK);
		vector<string>Values;

		Values.push_back(g_PI_OFF);
		Values.push_back(g_PI_ON);

		nRet = SetAllowedValues(g_PropNamePI, Values);

		if (nRet != DEVICE_OK)
			return nRet;
	}

	// Rolling scan Mode
	capaAttr.idCapa = TUIDC_ROLLINGSCANMODE;
	if (TUCAMRET_SUCCESS == TUCAM_Capa_GetAttr(m_opCam.hIdxTUCam, &capaAttr))
	{
		pAct = new CPropertyAction(this, &CMMTUCam::OnRollingScanMode);
		nRet = CreateProperty(g_PropNameRSMode, "", MM::String, false, pAct);
		assert(nRet == DEVICE_OK);

		nRet = SetAllowedRSMode();
		if (nRet != DEVICE_OK)
			return nRet;

		TUCAM_Capa_GetValue(m_opCam.hIdxTUCam, TUIDC_ROLLINGSCANMODE, &m_rsPara.nMode);
	}

	// Rolling scan line time delay
	capaAttr.idCapa = TUIDC_ROLLINGSCANLTD;
	if (TUCAMRET_SUCCESS == TUCAM_Capa_GetAttr(m_opCam.hIdxTUCam, &capaAttr))
	{
		pAct = new CPropertyAction(this, &CMMTUCam::OnRollingScanLtd);
		nRet = CreateProperty(g_PropNameRSLtd, "1", MM::Integer, false, pAct);
		assert(nRet == DEVICE_OK);

		//m_rsPara.nLTDelayMin = 1;
		m_rsPara.nLTDelayMax = capaAttr.nValMax;
		SetPropertyLimits(g_PropNameRSLtd, m_rsPara.nLTDelayMin, m_rsPara.nLTDelayMax);
		TUCAM_Capa_GetValue(m_opCam.hIdxTUCam, TUIDC_ROLLINGSCANLTD, &m_rsPara.nLTDelay);
	}

	// Rolling scan slit height
	capaAttr.idCapa = TUIDC_ROLLINGSCANSLIT;
	if (TUCAMRET_SUCCESS == TUCAM_Capa_GetAttr(m_opCam.hIdxTUCam, &capaAttr))
	{
		pAct = new CPropertyAction(this, &CMMTUCam::OnRollinScanSlit);
		nRet = CreateProperty(g_PropNameRSSlit, "1", MM::Integer, false, pAct);
		assert(nRet == DEVICE_OK);

		//m_rsPara.nSlitHeightMin = 1;
		m_rsPara.nSlitHeightMax = capaAttr.nValMax;
		SetPropertyLimits(g_PropNameRSSlit, m_rsPara.nSlitHeightMin, m_rsPara.nSlitHeightMax);
		TUCAM_Capa_GetValue(m_opCam.hIdxTUCam, TUIDC_ROLLINGSCANSLIT, &m_rsPara.nSlitHeight);
		m_rsPara.dbLineInvalTm = LineIntervalTime(0 == m_rsPara.nMode ? 0 : m_rsPara.nLTDelay);

		/// Line interval time
		pAct = new CPropertyAction(this, &CMMTUCam::OnRollinScanLITm);
		nRet = CreateProperty(g_PropNameRSLITm, "10.00", MM::String, false, pAct);
		assert(nRet == DEVICE_OK);
	}

	// Rolling scan dir
	capaAttr.idCapa = TUIDC_ROLLINGSCANDIR;
	if (TUCAMRET_SUCCESS == TUCAM_Capa_GetAttr(m_opCam.hIdxTUCam, &capaAttr))
	{
		pAct = new CPropertyAction(this, &CMMTUCam::OnRollingScanDir);
		nRet = CreateProperty(g_PropNameRSDir, "", MM::String, false, pAct);
		assert(nRet == DEVICE_OK);

		nRet = SetAllowedRSDir();
		if (nRet != DEVICE_OK)
			return nRet;
	}

	// Rolling scan dir reset
	capaAttr.idCapa = TUIDC_ROLLINGSCANRESET;
	if (TUCAMRET_SUCCESS == TUCAM_Capa_GetAttr(m_opCam.hIdxTUCam, &capaAttr))
	{
		pAct = new CPropertyAction(this, &CMMTUCam::OnRollingScanReset);
		nRet = CreateProperty(g_PropNameRSReset, "", MM::String, false, pAct);
		assert(nRet == DEVICE_OK);

		nRet = SetAllowedRSReset();
		if (nRet != DEVICE_OK)
			return nRet;
	}

	// Test Image
	capaAttr.idCapa = TUIDC_TESTIMGMODE;
	if (TUCAMRET_SUCCESS == TUCAM_Capa_GetAttr(m_opCam.hIdxTUCam, &capaAttr))
	{
		pAct = new CPropertyAction(this, &CMMTUCam::OnTestImageMode);
		nRet = CreateProperty(g_PropNameTestImg, "", MM::String, false, pAct);

		assert(nRet == DEVICE_OK);

		nRet = SetAllowedTestImg();
		if (nRet != DEVICE_OK)
			return nRet;
	}

	// TEC
	capaAttr.idCapa = TUIDC_ENABLETEC;
	if (TUCAMRET_SUCCESS == TUCAM_Capa_GetAttr(m_opCam.hIdxTUCam, &capaAttr))
	{
		pAct = new CPropertyAction(this, &CMMTUCam::OnTECMode);

		nRet = CreateProperty(g_PropNameTEC, g_TEC_OFF, MM::String, false, pAct);
		assert(nRet == DEVICE_OK);
		vector<string>TECValues;

		TECValues.push_back(g_TEC_OFF);
		TECValues.push_back(g_TEC_ON);

		nRet = SetAllowedValues(g_PropNameTEC, TECValues);

		if (nRet != DEVICE_OK)
			return nRet;
	}

	// Trigger
	int nVal = 0;
	m_tgrAttr.nTgrMode = TUCCM_SEQUENCE;
	if (m_bTriEn)
	{
		if (TUCAMRET_SUCCESS == TUCAM_Cap_GetTrigger(m_opCam.hIdxTUCam, &m_tgrAttr))
		{
			// Trigger mode
			pAct = new CPropertyAction (this, &CMMTUCam::OnTriggerMode);
			nRet = CreateProperty(g_PropNameMdTgr, g_TRIGGER_OFF, MM::String, false, pAct);
			assert(nRet == DEVICE_OK);

			vector<string>ModTgrValues;
			ModTgrValues.push_back(g_TRIGGER_OFF);

			if (IsSupport95V2New() || IsSupport401DNew() || IsSupport400BSIV3New())
			{
				ModTgrValues.push_back(g_TRIGGER_STDOVERLAP);
				ModTgrValues.push_back(g_TRIGGER_STDNONOVERLAP);
			}
			else
			{
				ModTgrValues.push_back(g_TRIGGER_STD);
			}

			int nImgMode = 0;
			switch (m_nPID)
			{
            case PID_FL_9BW:
            case PID_FL_9BW_LT:
            case PID_FL_26BW:
			case PID_FL_20BW:
			case DHYANA_401D:
			case DHYANA_201D:
			case DHYANA_4040V2:
			case DHYANA_4040BSI:
			case DHYANA_XF4040BSI:
			case PID_ARIES16LT:
			case PID_ARIES16:
			{
				if (TU_PHXCAMERALINK == m_nDriverType)
				{
					m_bCC1Support = true;
					ModTgrValues.push_back(g_TRIGGER_CC1);
				}
			}
			break;
			default:
			{
				if ((m_nPID == DHYANA_400BSIV2 && m_nBCD > 0x04) || DHYANA_400BSIV3 == m_nPID)
				{
					TUCAM_Capa_GetValue(m_opCam.hIdxTUCam, TUIDC_IMGMODESELECT, &nVal);
					nImgMode = (DHYANA_400BSIV2 == m_nPID) ? 0x03 : 0x05;
					if (nImgMode != nVal)
					{
						ModTgrValues.push_back(g_TRIGGER_SYN);
					}
				}
				else
				{
					ModTgrValues.push_back(g_TRIGGER_SYN);
					ModTgrValues.push_back(g_TRIGGER_GLB);
				}
			}
			break;
			}
			ModTgrValues.push_back(g_TRIGGER_SWF);

			nRet = SetAllowedValues(g_PropNameMdTgr, ModTgrValues);
			if (nRet != DEVICE_OK)
				return nRet;

			// Trigger exposure mode
			pAct = new CPropertyAction (this, &CMMTUCam::OnTriggerExpMode);
			nRet = CreateProperty(g_PropNameMdExp, "Off", MM::String, false, pAct);
			assert(nRet == DEVICE_OK);

			vector<string>ModExpValues;
			ModExpValues.push_back(g_TRIGGER_EXP_EXPTM);
			ModExpValues.push_back(g_TRIGGER_EXP_WIDTH);

			nRet = SetAllowedValues(g_PropNameMdExp, ModExpValues);
			if (nRet != DEVICE_OK)
				return nRet;

			// Trigger edge mode
			pAct = new CPropertyAction (this, &CMMTUCam::OnTriggerEdgeMode);
			nRet = CreateProperty(g_PropNameMdEdg, "Off", MM::String, false, pAct);
			assert(nRet == DEVICE_OK);

			vector<string>ModEdgeValues;
			ModEdgeValues.push_back(g_TRIGGER_EDGE_RISING);
			ModEdgeValues.push_back(g_TRIGGER_EDGE_FALLING);

			nRet = SetAllowedValues(g_PropNameMdEdg, ModEdgeValues);
			if (nRet != DEVICE_OK)
				return nRet;

			// Trigger delay
			pAct = new CPropertyAction (this, &CMMTUCam::OnTriggerDelay);
			nRet = CreateProperty(g_PropNameMdDly, "0", MM::Integer, false, pAct);
			assert(nRet == DEVICE_OK);

			// Trigger Filter
			capaAttr.idCapa = TUIDC_SIGNALFILTER;
			if (TUCAMRET_SUCCESS == TUCAM_Capa_GetAttr(m_opCam.hIdxTUCam, &capaAttr))
			{
				pAct = new CPropertyAction(this, &CMMTUCam::OnTriggerFilter);
				nRet = CreateProperty(g_PropNameFilter, "0", MM::Integer, false, pAct);
				assert(nRet == DEVICE_OK);
				SetPropertyLimits(g_PropNameFilter, 1, 1000000);
			}

            if (PID_FL_9BW == m_nPID || PID_FL_9BW_LT == m_nPID || PID_FL_26BW == m_nPID)
            {
                // Trigger total frames
                pAct = new CPropertyAction(this, &CMMTUCam::OnTriggerTotalFrames);
                nRet = CreateProperty(g_PropNameMdTFrames, "1", MM::Integer, false, pAct);
                assert(nRet == DEVICE_OK);
            }

            SetPropertyLimits(g_PropNameMdTFrames, 1, 0xFFFF);

			if (TUCAMRET_SUCCESS == TUCAM_Capa_GetValue(m_opCam.hIdxTUCam, TUIDC_ROLLINGSCANMODE, &m_rsPara.nMode))
			{
				// Trigger frames
				pAct = new CPropertyAction(this, &CMMTUCam::OnTriggerFrames);
				nRet = CreateProperty(g_PropNameMdFrames, "1", MM::Integer, false, pAct);
				assert(nRet == DEVICE_OK);

                SetPropertyLimits(g_PropNameMdFrames, 1, 0xFFFF);
			}
		}
	}
	
	// Fan gear
	capaAttr.idCapa = TUIDC_FAN_GEAR;
	if (TUCAMRET_SUCCESS == TUCAM_Capa_GetAttr(m_opCam.hIdxTUCam, &capaAttr))
	{
		if (isSupportFanCool())
		{
			if (DHYANA_4040V2 == m_nPID || DHYANA_4040BSI == m_nPID || DHYANA_XF4040BSI == m_nPID)
			{
				pAct = new CPropertyAction(this, &CMMTUCam::OnFanState);
				nRet = CreateProperty(g_PropNameFan, g_FAN_ON, MM::String, false, pAct);
				assert(nRet == DEVICE_OK);
				vector<string>Values;

				Values.push_back(g_FAN_OFF);
				Values.push_back(g_FAN_ON);

				nRet = SetAllowedValues(g_PropNameFan, Values);
			}
			else
			{
				pAct = new CPropertyAction(this, &CMMTUCam::OnFan);
				nRet = CreateProperty(g_PropNameFan, "High", MM::String, false, pAct);
				assert(nRet == DEVICE_OK);

				nRet = SetAllowedFanGear();
			}

			if (nRet != DEVICE_OK)
				return nRet;
		}
	}

	// Sharpness
	propAttr.nIdxChn= 0;
	propAttr.idProp = TUIDP_SHARPNESS;
	if (TUCAMRET_SUCCESS == TUCAM_Prop_GetAttr(m_opCam.hIdxTUCam, &propAttr))
	{
		pAct = new CPropertyAction (this, &CMMTUCam::OnSharpness);
		nRet = CreateProperty(g_PropNameSharp, "0", MM::Integer, false, pAct);
		assert(nRet == DEVICE_OK);

		SetPropertyLimits(g_PropNameSharp, (int)propAttr.dbValMin, (int)propAttr.dbValMax);
	}

	// DPC Level
    int nMaxNoise = 0;
	propAttr.nIdxChn= 0;
	propAttr.idProp = TUIDP_NOISELEVEL;
	if (TUCAMRET_SUCCESS == TUCAM_Prop_GetAttr(m_opCam.hIdxTUCam, &propAttr))
	{
		nMaxNoise = (int)propAttr.dbValMax;
		// DPC mode
		pAct = new CPropertyAction (this, &CMMTUCam::OnDPCLevel);
		nRet = CreateProperty(g_PropNameDPC, g_DPC_OFF, MM::String, false, pAct);
		assert(nRet == DEVICE_OK);

		vector<string>ModDPCValues;
		ModDPCValues.push_back(g_DPC_OFF);
		ModDPCValues.push_back(g_DPC_LOW);
		ModDPCValues.push_back(g_DPC_MEDIUM);
		ModDPCValues.push_back(g_DPC_HIGH);

		nRet = SetAllowedValues(g_PropNameDPC, ModDPCValues);
		if (nRet != DEVICE_OK)
			return nRet;
	}

	if (nMaxNoise == 0)
	{
		propAttr.nIdxChn= 0;
		propAttr.idProp = TUIDP_DPCLEVEL;
		if (TUCAMRET_SUCCESS == TUCAM_Prop_GetAttr(m_opCam.hIdxTUCam, &propAttr))
		{
			// DPC mode
			pAct = new CPropertyAction (this, &CMMTUCam::OnDPCAdjust);
			nRet = CreateProperty(g_PropNameDPC, "0", MM::Integer, false, pAct);
			assert(nRet == DEVICE_OK);

			SetPropertyLimits(g_PropNameDPC, (int)propAttr.dbValMin, (int)propAttr.dbValMax);
		}
	}

	// Offset
	if (m_bOffsetEn)
	{
		propAttr.nIdxChn= 0;
		propAttr.idProp = TUIDP_BLACKLEVEL;
		if (TUCAMRET_SUCCESS == TUCAM_Prop_GetAttr(m_opCam.hIdxTUCam, &propAttr))
		{
			// DPC mode
			pAct = new CPropertyAction (this, &CMMTUCam::OnBlackLevel);
			nRet = CreateProperty(g_PropNameOffset, "0", MM::Integer, false, pAct);
			assert(nRet == DEVICE_OK);

			SetPropertyLimits(g_PropNameOffset, (int)propAttr.dbValMin, (int)propAttr.dbValMax);
		}
	}

	// TriggerOut
	if (TUCAMRET_SUCCESS == TUCAM_Cap_GetTriggerOut(m_opCam.hIdxTUCam, &m_tgrOutAttr))
	{

        m_tgrOutAttr.nTgrOutMode = 5;
		TUCAM_Cap_SetTriggerOut(m_opCam.hIdxTUCam, m_tgrOutAttr);

		m_tgrOutPara.nTgrOutPort = m_tgrOutAttr.nTgrOutPort;
		switch(m_tgrOutPara.nTgrOutPort)
		{
		case TUPORT_ONE:
			m_tgrOutPara.TgrPort1.nTgrOutMode = m_tgrOutAttr.nTgrOutMode;
			m_tgrOutPara.TgrPort1.nEdgeMode = m_tgrOutAttr.nEdgeMode;
			m_tgrOutPara.TgrPort1.nDelayTm = m_tgrOutAttr.nDelayTm;
			m_tgrOutPara.TgrPort1.nWidth = m_tgrOutAttr.nWidth;
			break;

		case TUPORT_TWO:
			m_tgrOutPara.TgrPort2.nTgrOutMode = m_tgrOutAttr.nTgrOutMode;
			m_tgrOutPara.TgrPort2.nEdgeMode = m_tgrOutAttr.nEdgeMode;
			m_tgrOutPara.TgrPort2.nDelayTm = m_tgrOutAttr.nDelayTm;
			m_tgrOutPara.TgrPort2.nWidth = m_tgrOutAttr.nWidth;
			break;

		case TUPORT_THREE:
			m_tgrOutPara.TgrPort3.nTgrOutMode = m_tgrOutAttr.nTgrOutMode;
			m_tgrOutPara.TgrPort3.nEdgeMode = m_tgrOutAttr.nEdgeMode;
			m_tgrOutPara.TgrPort3.nDelayTm = m_tgrOutAttr.nDelayTm;
			m_tgrOutPara.TgrPort3.nWidth = m_tgrOutAttr.nWidth;
			break;
		default:
			break;
		}

		// OutPutTrigger Enable
		capaAttr.idCapa = TUIDC_ENABLETRIOUT;
		if (TUCAMRET_SUCCESS == TUCAM_Capa_GetAttr(m_opCam.hIdxTUCam, &capaAttr))
		{
			pAct = new CPropertyAction(this, &CMMTUCam::OnTriOutEnable);

			nRet = CreateProperty(g_PropNameOTEnable, g_OT_ON, MM::String, false, pAct);
			assert(nRet == DEVICE_OK);
			vector<string>Values;

			Values.push_back(g_OT_OFF);
			Values.push_back(g_OT_ON);

			nRet = SetAllowedValues(g_PropNameOTEnable, Values);

			if (nRet != DEVICE_OK)
				return nRet;
		}

		// OutPutTrigger Port Mode
		pAct = new CPropertyAction (this, &CMMTUCam::OnTrgOutPortMode);
		nRet = CreateProperty(g_PropNamePort, g_TRIGGER_PORT1, MM::String, false, pAct);
		assert(nRet == DEVICE_OK);

		vector<string>ModPortValues;
		ModPortValues.push_back(g_TRIGGER_PORT1);
		ModPortValues.push_back(g_TRIGGER_PORT2);

		if (TRITYPE_SMA == m_nTriType)
		{
			ModPortValues.push_back(g_TRIGGER_PORT3);
		}

		nRet = SetAllowedValues(g_PropNamePort, ModPortValues);
		if (nRet != DEVICE_OK)
			return nRet;

		// OutPutTrigger Kind Mode
		if (TRITYPE_SMA == m_nTriType)
		{
			pAct = new CPropertyAction (this, &CMMTUCam::OnTrgOutKindMode);
			nRet = CreateProperty(g_PropNameKind, g_TRIGGER_READEND, MM::String, false, pAct);
			assert(nRet == DEVICE_OK);

			vector<string>ModKindValues;
			ModKindValues.push_back(g_TRIGGER_EXPSTART);
			ModKindValues.push_back(g_TRIGGER_READEND);
			ModKindValues.push_back(g_TRIGGER_GLBEXP);
			if (IsSupport95V2New() || IsSupport401DNew() || IsSupport400BSIV3New())
			{
				ModKindValues.push_back(g_TRIGGER_TRIREADY);
			}
			ModKindValues.push_back(g_TRIGGER_LOW);
			ModKindValues.push_back(g_TRIGGER_HIGH);

			nRet = SetAllowedValues(g_PropNameKind, ModKindValues);
			if (nRet != DEVICE_OK)
				return nRet;
		}
		
		// OutPutTrigger Edge Mode
		pAct = new CPropertyAction (this, &CMMTUCam::OnTrgOutEdgeMode);
		nRet = CreateProperty(g_PropNameEdge, g_TRIGGER_EDGE_RISING, MM::String, false, pAct);
		assert(nRet == DEVICE_OK);

		vector<string>ModEdgeValues;
		ModEdgeValues.push_back(g_TRIGGER_EDGE_RISING);
		ModEdgeValues.push_back(g_TRIGGER_EDGE_FALLING);

		nRet = SetAllowedValues(g_PropNameEdge, ModEdgeValues);
		if (nRet != DEVICE_OK)
			return nRet;

		// OutPutTrigger Delay
		pAct = new CPropertyAction (this, &CMMTUCam::OnTrgOutDelay);
		nRet = CreateProperty(g_PropNameDelay, "0", MM::Integer, false, pAct);
		assert(nRet == DEVICE_OK);

		SetPropertyLimits(g_PropNameDelay, 0, 10000000);

		// OutPutTrigger Width
		pAct = new CPropertyAction (this, &CMMTUCam::OnTrgOutWidth);
		nRet = CreateProperty(g_PropNameWidth, "1", MM::Integer, false, pAct);
		assert(nRet == DEVICE_OK);

		SetPropertyLimits(g_PropNameWidth, 1, 10000000);

	}

    // initialize image buffer
    nRet = StartCapture();
    if (nRet != DEVICE_OK)
        return nRet;
   
	// pixel type
	vector<string> pixelTypeValues;
	pAct = new CPropertyAction (this, &CMMTUCam::OnPixelType);

	if (3 == m_frame.ucChannels)
	{
#ifdef _WIN64        
		if (2 == m_frame.ucElemBytes)
		{
			nRet = CreateProperty(MM::g_Keyword_PixelType, g_PixelType_64bitRGB, MM::String, false, pAct);
			assert(nRet == DEVICE_OK);

			pixelTypeValues.push_back(g_PixelType_64bitRGB);
		}  
		else
		{
			nRet = CreateProperty(MM::g_Keyword_PixelType, g_PixelType_32bitRGB, MM::String, false, pAct);
			assert(nRet == DEVICE_OK);

			pixelTypeValues.push_back(g_PixelType_32bitRGB);
		} 
#else
		nRet = CreateProperty(MM::g_Keyword_PixelType, g_PixelType_32bitRGB, MM::String, false, pAct);
		assert(nRet == DEVICE_OK);
		pixelTypeValues.push_back(g_PixelType_32bitRGB);
#endif

	}
	else
	{
		if (2 == m_frame.ucElemBytes)
		{
			nRet = CreateProperty(MM::g_Keyword_PixelType, g_PixelType_16bit, MM::String, false, pAct);
			assert(nRet == DEVICE_OK);

			pixelTypeValues.push_back(g_PixelType_16bit);

			if (TRITYPE_HR == m_nTriType)
			{
				pixelTypeValues.push_back(g_PixelType_8bit);
			}
		}
		else
		{
			nRet = CreateProperty(MM::g_Keyword_PixelType, g_PixelType_8bit, MM::String, false, pAct);
			assert(nRet == DEVICE_OK);

			pixelTypeValues.push_back(g_PixelType_8bit);

			if (TRITYPE_HR == m_nTriType)
			{
				pixelTypeValues.push_back(g_PixelType_16bit);
			}
		}
	}  

	nRet = SetAllowedValues(MM::g_Keyword_PixelType, pixelTypeValues);
	if (nRet != DEVICE_OK)
		return nRet;


    // synchronize all properties
    // --------------------------
    nRet = UpdateStatus();
    if (nRet != DEVICE_OK)
        return nRet;

    // setup the buffer
    // ----------------
    nRet = ResizeImageBuffer();
    if (nRet != DEVICE_OK)
        return nRet;

#ifdef TESTRESOURCELOCKING
    TestResourceLocking(true);
    LogMessage("TestResourceLocking OK",true);
#endif

    initialized_ = true;

    // initialize image buffer
    GenerateEmptyImage(img_);


	// initialize image buffer
	nRet = StopCapture();
	if (nRet != DEVICE_OK)
		return nRet;

//     char sz[256] = {0};
//     sprintf(sz, "%d\n", m_pfSave->pFrame);
//     OutputDebugString(sz);
//     TUCAM_File_SaveImage(m_opCam.hIdxTUCam, *m_pfSave);

    OutputDebugString("[Initialize]:Success!\n");

    return DEVICE_OK;
}

/**
* Shuts down (unloads) the device.
* Required by the MM::Device API.
* Ideally this method will completely unload the device and release all resources.
* Shutdown() may be called multiple times in a row.
* After Shutdown() we should be allowed to call Initialize() again to load the device
* without causing problems.
*/
int CMMTUCam::Shutdown()
{
    // Close the get value of temperature thread

    OutputDebugString("[Shutdown]:enter");

    if (NULL != m_hThdTempEvt)
    {
        m_bTemping = false;
        WaitForSingleObject(m_hThdTempEvt, INFINITE);	
        CloseHandle(m_hThdTempEvt);
        m_hThdTempEvt = NULL;
    }

    StopSequenceAcquisition();

    UninitTUCamApi();
    initialized_ = false;
    return DEVICE_OK;
}

/**
* Performs exposure and grabs a single image.
* This function should block during the actual exposure and return immediately afterwards 
* (i.e., before readout).  This behavior is needed for proper synchronization with the shutter.
* Required by the MM::Camera API.
*/
int CMMTUCam::SnapImage()
{
    static int callCounter = 0;
    ++callCounter;

    MM::MMTime startTime = GetCurrentMMTime();
    
	double exp = GetExposure();
    if (sequenceRunning_) 
    {
		// Change the exposure time
        exp = GetSequenceExposure();
		TUCAM_Prop_SetValue(m_opCam.hIdxTUCam, TUIDP_EXPOSURETM, exp);	
    }

	/*if (!m_bLiving)  // should never happen, but just in case...
	{
		StartCapture();
	}*/
	if (!m_bAcquisition)
	{
		StartCapture();
	}

	int nRet = DEVICE_ERR;

	if (TUCCM_TRIGGER_SOFTWARE == m_tgrAttr.nTgrMode)
	{
		int nCnt = 0;
		///int nRet = DEVICE_ERR;

		do 
		{
			TUCAM_Cap_DoSoftwareTrigger(m_opCam.hIdxTUCam);
			nRet = WaitForFrame(img_);
			nCnt++;
		} while (DEVICE_OK != nRet && nCnt < 2);
	}
	else
	{
		if (!fastImage_)
		{	
			MM::MMTime s0(0,0);
			if( s0 < startTime )
			{
				CDeviceUtils::SleepMs((long) exp);
			}
			///TUDBG_PRINTF("SanpImage fastImage_ = %d",fastImage_);
			nRet = WaitForFrame(img_);
		}
	}

   readoutStartTime_ = GetCurrentMMTime();

   if (!m_bAcquisition)
   {
	   StopCapture();
   }

   return nRet/*DEVICE_OK*/;
}


/**
* Returns pixel data.
* Required by the MM::Camera API.
* The calling program will assume the size of the buffer based on the values
* obtained from GetImageBufferSize(), which in turn should be consistent with
* values returned by GetImageWidth(), GetImageHight() and GetImageBytesPerPixel().
* The calling program allso assumes that camera never changes the size of
* the pixel buffer on its own. In other words, the buffer can change only if
* appropriate properties are set (such as binning, pixel type, etc.)
*/
const unsigned char* CMMTUCam::GetImageBuffer()
{
    MMThreadGuard g(imgPixelsLock_);
    MM::MMTime readoutTime(readoutUs_);
    while (readoutTime > (GetCurrentMMTime() - readoutStartTime_)) {}		
    unsigned char *pB = (unsigned char*)(img_.GetPixels());
    return pB;  //NULL
}

/**
* Returns image buffer X-size in pixels.
* Required by the MM::Camera API.
*/
unsigned CMMTUCam::GetImageWidth() const
{
/*
    if (NULL != m_frame.pBuffer)
    {
        return m_frame.usWidth;
    }

    return 0;
*/
    return img_.Width();
}

/**
* Returns image buffer Y-size in pixels.
* Required by the MM::Camera API.
*/
unsigned CMMTUCam::GetImageHeight() const
{
/*
    if (NULL != m_frame.pBuffer)
    {
        return m_frame.usHeight;
    }

    return 0;
*/
    return img_.Height();
}

/**
* Returns image buffer pixel depth in bytes.
* Required by the MM::Camera API.
*/
unsigned CMMTUCam::GetImageBytesPerPixel() const
{
/*
    if (NULL != m_frame.pBuffer)
    {
        int nChnnels = (1 == m_frame.ucChannels) ? 1 : 4;

        return (m_frame.ucElemBytes * nChnnels);
    }

    return 1;
*/
    return img_.Depth();
} 

/**
* Returns the bit depth (dynamic range) of the pixel.
* This does not affect the buffer size, it just gives the client application
* a guideline on how to interpret pixel values.
* Required by the MM::Camera API.
*/
unsigned CMMTUCam::GetBitDepth() const
{
/*
    if (NULL != m_frame.pBuffer)
    {
        return (1 == m_frame.ucElemBytes) ? 8 : 16;
    }

    return 8;
*/
    return bitDepth_;
}

/**
* Returns the size in bytes of the image buffer.
* Required by the MM::Camera API.
*/
long CMMTUCam::GetImageBufferSize() const
{
/*
    if (NULL != m_frame.pBuffer)
    {
        return (m_frame.usWidth * m_frame.usHeight * GetImageBytesPerPixel());
    }

    return 0;
*/    
    
    return img_.Width() * img_.Height() * GetImageBytesPerPixel();
}

/**
* Sets the camera Region Of Interest.
* Required by the MM::Camera API.
* This command will change the dimensions of the image.
* Depending on the hardware capabilities the camera may not be able to configure the
* exact dimensions requested - but should try do as close as possible.
* If the hardware does not have this capability the software should simulate the ROI by
* appropriately cropping each frame.
* This demo implementation ignores the position coordinates and just crops the buffer.
* @param x - top-left corner coordinate
* @param y - top-left corner coordinate
* @param xSize - width
* @param ySize - height
*/

int CMMTUCam::SetROI(unsigned x, unsigned y, unsigned xSize, unsigned ySize)
{
    if (NULL == m_opCam.hIdxTUCam)
        return DEVICE_NOT_CONNECTED;

    if (xSize == 0 && ySize == 0)
    {
        // effectively clear ROI
        ResizeImageBuffer();
        roiX_ = 0;
        roiY_ = 0;
        m_bROI = false;
    }
    else
    {
        if (NULL == m_opCam.hIdxTUCam)
            return DEVICE_NOT_CONNECTED;

        m_bLiving = false;
        TUCAM_Cap_Stop(m_opCam.hIdxTUCam);      // Stop capture   
        ReleaseBuffer();

        // apply ROI 
        TUCAM_ROI_ATTR roiAttr;
        roiAttr.bEnable = TRUE;
        roiAttr.nHOffset= ((x >> 2) << 2);
        roiAttr.nVOffset= ((y >> 2) << 2);
//        roiAttr.nVOffset= (((m_nMaxHeight/*img_.Height()*/ - y - ySize) >> 2) << 2);
        roiAttr.nWidth  = (xSize >> 3) << 3;    //// roiAttr.nWidth  = (xSize >> 2) << 2;
        roiAttr.nHeight = (ySize >> 3) << 3;    //// roiAttr.nHeight = (ySize >> 2) << 2;

		if(roiAttr.nWidth < 32)
			roiAttr.nWidth  = 32;

        TUCAM_Cap_SetROI(m_opCam.hIdxTUCam, roiAttr);
        TUCAM_Cap_GetROI(m_opCam.hIdxTUCam, &roiAttr);

        char sz[256] = {0};
        sprintf(sz, "x:%d, y:%d, xsize:%d, ysize:%d, h:%d, v:%d, wid:%d, hei:%d, maxhei:%d", x, y, xSize, ySize, roiAttr.nHOffset, roiAttr.nVOffset, roiAttr.nWidth, roiAttr.nHeight, m_nMaxHeight);
        OutputDebugString(sz);

        roiX_ = x;
        roiY_ = y;
        m_bROI = true;

//      StartCapture();
        ResizeImageBuffer();

		Sleep(2);
		double dblExp=0;
		TUCAM_Prop_GetValue(m_opCam.hIdxTUCam, TUIDP_EXPOSURETM, &dblExp);
		TUCAM_Prop_SetValue(m_opCam.hIdxTUCam, TUIDP_EXPOSURETM, dblExp);
    }

    return DEVICE_OK;
}

/**
* Returns the actual dimensions of the current ROI.
* Required by the MM::Camera API.
*/
int CMMTUCam::GetROI(unsigned& x, unsigned& y, unsigned& xSize, unsigned& ySize)
{
    x = roiX_;
    y = roiY_;

    xSize = img_.Width();
    ySize = img_.Height();

    return DEVICE_OK;
}

/**
* Resets the Region of Interest to full frame.
* Required by the MM::Camera API.
*/
int CMMTUCam::ClearROI()
{
    if (NULL == m_opCam.hIdxTUCam)
        return DEVICE_NOT_CONNECTED;

    m_bLiving = false;
    TUCAM_Cap_Stop(m_opCam.hIdxTUCam);      // Stop capture   
    ReleaseBuffer();

    // close ROI 
    TUCAM_ROI_ATTR roiAttr;
    TUCAM_Cap_GetROI(m_opCam.hIdxTUCam, &roiAttr);

    roiAttr.bEnable = FALSE;
    TUCAM_Cap_SetROI(m_opCam.hIdxTUCam, roiAttr);
    
    roiX_ = 0;
    roiY_ = 0;
    m_bROI = false;

//  StartCapture();
    ResizeImageBuffer();

    return DEVICE_OK;
}

/**
* Returns the current exposure setting in milliseconds.
* Required by the MM::Camera API.
*/
double CMMTUCam::GetExposure() const
{
    char buf[MM::MaxStrLength];
    int ret = GetProperty(MM::g_Keyword_Exposure, buf);
    if (ret != DEVICE_OK)
        return 0.0;

    return atof(buf);
}

/**
 * Returns the current exposure from a sequence and increases the sequence counter
 * Used for exposure sequences
 */
double CMMTUCam::GetSequenceExposure() 
{
    if (exposureSequence_.size() == 0) 
        return this->GetExposure();

    double exposure = exposureSequence_[sequenceIndex_];

    sequenceIndex_++;
    if (sequenceIndex_ >= exposureSequence_.size())
        sequenceIndex_ = 0;

    return exposure;
}

/**
* Sets exposure in milliseconds.
* Required by the MM::Camera API.
*/
void CMMTUCam::SetExposure(double exp)
{
    if (exp < exposureMinimum_)
    {
       exp = exposureMinimum_;
    } else if (exp > exposureMaximum_) {
       exp = exposureMaximum_;
    }
    SetProperty(MM::g_Keyword_Exposure, CDeviceUtils::ConvertToString(exp));
    GetCoreCallback()->OnExposureChanged(this, exp);
}

/**
* Returns the current binning factor.
* Required by the MM::Camera API.
*/
int CMMTUCam::GetBinning() const
{
    return 1;  // fdy20190212
    /*char buf[MM::MaxStrLength]; 
    int ret = GetProperty(MM::g_Keyword_Binning, buf);
    if (ret != DEVICE_OK)
        return 1;
    return atoi(buf);*/
}

/**
* Sets binning factor.
* Required by the MM::Camera API.
*/
int CMMTUCam::SetBinning(int binF)
{
    return SetProperty(MM::g_Keyword_Binning, CDeviceUtils::ConvertToString(binF));
}

int CMMTUCam::IsExposureSequenceable(bool& isSequenceable) const
{
    isSequenceable = isSequenceable_;
    return DEVICE_OK;
}

int CMMTUCam::GetExposureSequenceMaxLength(long& nrEvents) const
{
    if (!isSequenceable_) 
    {
        return DEVICE_UNSUPPORTED_COMMAND;
    }

    nrEvents = sequenceMaxLength_;
    return DEVICE_OK;
}

int CMMTUCam::StartExposureSequence()
{
    if (!isSequenceable_) {
        return DEVICE_UNSUPPORTED_COMMAND;
    }

    // may need thread lock
    sequenceRunning_ = true;
    return DEVICE_OK;
}

int CMMTUCam::StopExposureSequence()
{
    if (!isSequenceable_) 
    {
        return DEVICE_UNSUPPORTED_COMMAND;
    }

    // may need thread lock
    sequenceRunning_ = false;
    sequenceIndex_ = 0;
    return DEVICE_OK;
}

/**
 * Clears the list of exposures used in sequences
 */
int CMMTUCam::ClearExposureSequence()
{
    if (!isSequenceable_) 
    {
        return DEVICE_UNSUPPORTED_COMMAND;
    }

    exposureSequence_.clear();
    return DEVICE_OK;
}

/**
 * Adds an exposure to a list of exposures used in sequences
 */
int CMMTUCam::AddToExposureSequence(double exposureTime_ms) 
{
    if (!isSequenceable_)
    {
        return DEVICE_UNSUPPORTED_COMMAND;
    }

    exposureSequence_.push_back(exposureTime_ms);
    return DEVICE_OK;
}

int CMMTUCam::SendExposureSequence() const 
{
    if (!isSequenceable_) 
    {
        return DEVICE_UNSUPPORTED_COMMAND;
    }

    return DEVICE_OK;
}

/**
* line interval time show
*/
double CMMTUCam::LineIntervalTime(int nLineDelayTm)
{
	if (NULL == m_opCam.hIdxTUCam)
	{
		return 0;
	}
	double dbValue = 0;
	TUCAM_PROP_ATTR attrProp;
	attrProp.nIdxChn = 0;
	attrProp.idProp = TUIDP_EXPOSURETM;
	TUCAM_Prop_GetAttr(m_opCam.hIdxTUCam, &attrProp);
	dbValue = (1 + nLineDelayTm) * attrProp.dbValStep * 1000;
	return dbValue;
}

/**
* line interval calc
*/
int CMMTUCam::LineIntervalCal(int nVal, bool bExpChange)
{
	if (NULL == m_opCam.hIdxTUCam)
	{
		return 1;
	}

	if (0 >= nVal)
	{
		return 1;
	}

	int nStep = 1;
	int nLine = 1;
	double dblExp = 0;
	TUCAM_PROP_ATTR  attrProp;

	attrProp.nIdxChn = 0;                    // 使用默认通道
	attrProp.idProp = TUIDP_EXPOSURETM;
	TUCAM_Prop_GetAttr(m_opCam.hIdxTUCam, &attrProp);
	TUCAM_Prop_GetValue(m_opCam.hIdxTUCam, TUIDP_EXPOSURETM, &dblExp);
	nLine = (int)(dblExp / attrProp.dbValStep);

	if (nLine > nVal)
	{
		nStep = (int)nLine / nVal;
	}
	else
	{
		nStep = 1;
		nLine = nVal;

		if (bExpChange)
		{
			dblExp = nLine * attrProp.dbValStep;
			TUCAM_Prop_SetValue(m_opCam.hIdxTUCam, TUIDP_EXPOSURETM, dblExp);
		}
	}

	return nStep;
}

int CMMTUCam::SetAllowedDepth()
{
	if (NULL == m_opCam.hIdxTUCam)
		return DEVICE_NOT_CONNECTED;

	TUCAM_CAPA_ATTR capaAttr;
	capaAttr.idCapa = TUIDC_BITOFDEPTH;

	if (TUCAMRET_SUCCESS != TUCAM_Capa_GetAttr(m_opCam.hIdxTUCam, &capaAttr))
	{
		return DEVICE_NOT_SUPPORTED;
	}

	char szBuf[64] = { 0 };
	TUCAM_VALUE_TEXT valText;
	valText.nID = TUIDC_BITOFDEPTH;
	valText.nTextSize = 64;
	valText.pText = &szBuf[0];

	vector<string> depthValues;
	int nCnt = capaAttr.nValMax - capaAttr.nValMin + 1;

	for (int i = 0; i<nCnt; i++)
	{
		valText.dbValue = i;
		TUCAM_Capa_GetValueText(m_opCam.hIdxTUCam, &valText);
		depthValues.push_back(string(valText.pText));
	}

	LogMessage("Setting allowed depth settings", true);
	return SetAllowedValues(g_PropNameBODP, depthValues);
}

int CMMTUCam::SetAllowedBinning() 
{
    if (NULL == m_opCam.hIdxTUCam)
        return DEVICE_NOT_CONNECTED;

    TUCAM_CAPA_ATTR capaAttr;
    capaAttr.idCapa = TUIDC_RESOLUTION;

    if (TUCAMRET_SUCCESS != TUCAM_Capa_GetAttr(m_opCam.hIdxTUCam, &capaAttr))
    {
        return DEVICE_NOT_SUPPORTED;
    }

    char szBuf[64] = {0};
    TUCAM_VALUE_TEXT valText;
    valText.nID       = TUIDC_RESOLUTION;
    valText.nTextSize = 64;
    valText.pText     = &szBuf[0];

    vector<string> binValues;
    int nCnt = capaAttr.nValMax - capaAttr.nValMin + 1;

    for (int i=0; i<nCnt; i++)
    {
        valText.dbValue = i;
        TUCAM_Capa_GetValueText(m_opCam.hIdxTUCam, &valText); 

        binValues.push_back(string(valText.pText));
    }

    if (PID_FL_9BW == m_nPID || PID_FL_9BW_LT == m_nPID)
    {
        capaAttr.idCapa = TUIDC_BINNING_SUM;

        if (TUCAMRET_SUCCESS == TUCAM_Capa_GetAttr(m_opCam.hIdxTUCam, &capaAttr))
        {
            valText.nID = TUIDC_BINNING_SUM;
            nCnt = capaAttr.nValMax - capaAttr.nValMin + 1;

            for (int i = 1; i<nCnt; i++)
            {
                valText.dbValue = i;
                TUCAM_Capa_GetValueText(m_opCam.hIdxTUCam, &valText);

                binValues.push_back(string(valText.pText));
            }
        }
    }

    LogMessage("Setting allowed binning settings", true);
    return SetAllowedValues(MM::g_Keyword_Binning, binValues);
}

int CMMTUCam::SetAllowedBinningSum()
{
    if (NULL == m_opCam.hIdxTUCam)
        return DEVICE_NOT_CONNECTED;

    TUCAM_CAPA_ATTR capaAttr;
    capaAttr.idCapa = TUIDC_BINNING_SUM;

    if (TUCAMRET_SUCCESS != TUCAM_Capa_GetAttr(m_opCam.hIdxTUCam, &capaAttr))
    {
        return DEVICE_NOT_SUPPORTED;
    }

    char szBuf[64] = { 0 };
    TUCAM_VALUE_TEXT valText;
    valText.nID = TUIDC_BINNING_SUM;
    valText.nTextSize = 64;
    valText.pText = &szBuf[0];

    vector<string> binValues;
    int nCnt = capaAttr.nValMax - capaAttr.nValMin + 1;

    for (int i = 0; i<nCnt; i++)
    {
        valText.dbValue = i;
        TUCAM_Capa_GetValueText(m_opCam.hIdxTUCam, &valText);

        binValues.push_back(string(valText.pText));
    }

    LogMessage("Setting allowed binning sum settings", true);
    return SetAllowedValues(g_PropNameBinningSum, binValues);
}

int CMMTUCam::SetAllowedPixelClock()
{
    if (NULL == m_opCam.hIdxTUCam)
        return DEVICE_NOT_CONNECTED;

    TUCAM_CAPA_ATTR capaAttr;
    capaAttr.idCapa = TUIDC_PIXELCLOCK;

    if (TUCAMRET_SUCCESS != TUCAM_Capa_GetAttr(m_opCam.hIdxTUCam, &capaAttr))
    {
        return DEVICE_NOT_SUPPORTED;
    }

    char szBuf[64] = {0};
    TUCAM_VALUE_TEXT valText;
    valText.nID       = TUIDC_PIXELCLOCK;
    valText.nTextSize = 64;
    valText.pText     = &szBuf[0];

    vector<string> plkValues;
    int nCnt = capaAttr.nValMax - capaAttr.nValMin + 1;

    for (int i=0; i<nCnt; i++)
    {
        valText.dbValue = i;
        TUCAM_Capa_GetValueText(m_opCam.hIdxTUCam, &valText); 

        plkValues.push_back(string(valText.pText));
    }

    LogMessage("Setting allowed pixel clock settings", true);
    return SetAllowedValues(g_PropNamePCLK, plkValues);
}

int CMMTUCam::SetAllowedFanGear()
{
    if (NULL == m_opCam.hIdxTUCam)
        return DEVICE_NOT_CONNECTED;

    TUCAM_CAPA_ATTR capaAttr;
    capaAttr.idCapa = TUIDC_FAN_GEAR;

    if (TUCAMRET_SUCCESS != TUCAM_Capa_GetAttr(m_opCam.hIdxTUCam, &capaAttr))
    {
        return DEVICE_NOT_SUPPORTED;
    }

    char szBuf[64] = {0};
    TUCAM_VALUE_TEXT valText;
    valText.nID       = TUIDC_FAN_GEAR;
    valText.nTextSize = 64;
    valText.pText     = &szBuf[0];

    vector<string> fanValues;
    int nCnt = capaAttr.nValMax - capaAttr.nValMin + 1;

    for (int i=0; i<nCnt; i++)
    {
        valText.dbValue = i;
        TUCAM_Capa_GetValueText(m_opCam.hIdxTUCam, &valText); 

        fanValues.push_back(string(valText.pText));
    }

    LogMessage("Setting allowed fan gear settings", true);
    return SetAllowedValues(g_PropNameFan, fanValues);
}

int CMMTUCam::SetAllowedGainMode()
{
	if (NULL == m_opCam.hIdxTUCam)
		return DEVICE_NOT_CONNECTED;

	int nRet = DEVICE_OK;
	CPropertyAction *pAct = NULL;
	TUCAM_CAPA_ATTR capaAttr;
	vector<string>  GAINValues;
	GAINValues.clear();

	// CMSMode Gain
	capaAttr.idCapa = TUIDC_IMGMODESELECT;
	if (TUCAMRET_SUCCESS == TUCAM_Capa_GetAttr(m_opCam.hIdxTUCam, &capaAttr) && m_nPID != DHYANA_D95_X100)
	{
		switch (m_nPID)
		{
		case DHYANA_D95_V2:
		{
			pAct = new CPropertyAction(this, &CMMTUCam::OnGAINMode);
			nRet = CreateProperty(g_PropNameGain, g_HDRBIT_ON, MM::String, false, pAct);
			assert(nRet == DEVICE_OK);
			
			GAINValues.push_back(g_HDRBIT_ON);
			GAINValues.push_back(g_HIGHBIT_ON);
			GAINValues.push_back(g_LOWBIT_ON);
			GAINValues.push_back(g_STDHIGH_ON);
			GAINValues.push_back(g_STDLOW_ON);

			nRet = SetAllowedValues(g_PropNameGain, GAINValues);
		}
		break;

		case DHYANA_400BSIV2:
		{
			pAct = new CPropertyAction(this, &CMMTUCam::OnGAINMode);
			nRet = CreateProperty(g_PropNameGain, g_CMSBIT_ON, MM::String, false, pAct);
			assert(nRet == DEVICE_OK);

			GAINValues.push_back(g_CMSBIT_ON);
			GAINValues.push_back(g_HDRBIT_ON);
			GAINValues.push_back(g_HIGHBIT_ON);

			if (m_nBCD > 0x04 && capaAttr.nValMax > 0x2)
			{
				GAINValues.push_back(g_GRHIGH_ON);
				GAINValues.push_back(g_GRLOW_ON);
			}

			nRet = SetAllowedValues(g_PropNameGain, GAINValues);
		}
		break;

		case DHYANA_400BSIV3:
		{
			pAct = new CPropertyAction(this, &CMMTUCam::OnModeSelect);
			nRet = CreateProperty(g_PropNameMode, g_HIGHDYNAMIC_ON, MM::String, false, pAct);
			assert(nRet == DEVICE_OK);

			GAINValues.push_back(g_HIGHDYNAMIC_ON);    // hdr
			GAINValues.push_back(g_HIGHSPEED_ON);      // highspeedhg
			GAINValues.push_back(g_HIGHSENSITY_ON);    // cms
			GAINValues.push_back(g_GLOBALRESET_ON);    // globalreset

			nRet = SetAllowedValues(g_PropNameMode, GAINValues);
		}
		break;

        case PID_FL_9BW:
        case PID_FL_9BW_LT:
        {
            char szBuf[64] = { 0 };
            TUCAM_VALUE_TEXT valText;
            valText.nID = TUIDC_IMGMODESELECT;
            valText.nTextSize = 64;
            valText.pText = &szBuf[0];

            vector<string> modeValues;
            int nCnt = capaAttr.nValMax - capaAttr.nValMin + 1;

            for (int i = 0; i<nCnt; i++)
            {
                valText.dbValue = i;
                TUCAM_Capa_GetValueText(m_opCam.hIdxTUCam, &valText);

                modeValues.push_back(string(valText.pText));
            }
            
            pAct = new CPropertyAction(this, &CMMTUCam::OnGAINMode);
            nRet = CreateProperty(g_PropNameMode, modeValues.at(0).c_str(), MM::String, false, pAct);
            assert(nRet == DEVICE_OK);

            nRet = SetAllowedValues(g_PropNameMode, modeValues);
        }
        break;

		default:
			break;
		}
	}
	else
	{
		pAct = new CPropertyAction(this, &CMMTUCam::OnImageMode);
		nRet = CreateProperty(g_PropNameGain, "HDR", MM::String, false, pAct);
		assert(nRet == DEVICE_OK);
		nRet = SetAllowedImageMode();
	}
	return nRet;
}

int CMMTUCam::SetAllowedImageMode()
{
    if (NULL == m_opCam.hIdxTUCam)
        return DEVICE_NOT_CONNECTED;

    TUCAM_PROP_ATTR propAttr;
    propAttr.nIdxChn = 0;
    propAttr.idProp  = TUIDP_GLOBALGAIN;

    if (TUCAMRET_SUCCESS != TUCAM_Prop_GetAttr(m_opCam.hIdxTUCam, &propAttr))
    {
        return DEVICE_NOT_SUPPORTED;
    }

    char szBuf[64] = {0};
    TUCAM_VALUE_TEXT valText;
    valText.nID       = TUIDP_GLOBALGAIN;
    valText.nTextSize = 64;
    valText.pText     = &szBuf[0];

    vector<string> modValues;
    int nCnt = 2/*(int)propAttr.dbValMax*/ - (int)propAttr.dbValMin + 1;

    for (int i=0; i<nCnt; i++)
    {
        valText.dbValue = i;
        TUCAM_Prop_GetValueText(m_opCam.hIdxTUCam, &valText); 
        modValues.push_back(string(valText.pText));
    }

    LogMessage("Setting allowed image mode settings", true);
	return SetAllowedValues(g_PropNameGain, modValues);
}

int CMMTUCam::SetAllowedRSMode()
{
	if (NULL == m_opCam.hIdxTUCam)
		return DEVICE_NOT_CONNECTED;

	TUCAM_CAPA_ATTR capaAttr;
	capaAttr.idCapa = TUIDC_ROLLINGSCANMODE;

	if (TUCAMRET_SUCCESS != TUCAM_Capa_GetAttr(m_opCam.hIdxTUCam, &capaAttr))
	{
		return DEVICE_NOT_SUPPORTED;
	}

	char szBuf[64] = { 0 };
	TUCAM_VALUE_TEXT valText;
	valText.nID = TUIDC_ROLLINGSCANMODE;
	valText.nTextSize = 64;
	valText.pText = &szBuf[0];

	vector<string> vecValues;
	int nCnt = capaAttr.nValMax - capaAttr.nValMin + 1;

	for (int i = 0; i<nCnt; i++)
	{
		valText.dbValue = i;
		TUCAM_Capa_GetValueText(m_opCam.hIdxTUCam, &valText);

		vecValues.push_back(string(valText.pText));
	}

	LogMessage("Setting allowed rs mode settings", true);
	return SetAllowedValues(g_PropNameRSMode, vecValues);
}

int CMMTUCam::SetAllowedRSDir()
{
	if (NULL == m_opCam.hIdxTUCam)
		return DEVICE_NOT_CONNECTED;

	TUCAM_CAPA_ATTR capaAttr;
	capaAttr.idCapa = TUIDC_ROLLINGSCANDIR;

	if (TUCAMRET_SUCCESS != TUCAM_Capa_GetAttr(m_opCam.hIdxTUCam, &capaAttr))
	{
		return DEVICE_NOT_SUPPORTED;
	}

	char szBuf[64] = { 0 };
	TUCAM_VALUE_TEXT valText;
	valText.nID = TUIDC_ROLLINGSCANDIR;
	valText.nTextSize = 64;
	valText.pText = &szBuf[0];

	vector<string> vecValues;
	int nCnt = capaAttr.nValMax - capaAttr.nValMin + 1;

	for (int i = 0; i<nCnt; i++)
	{
		valText.dbValue = i;
		TUCAM_Capa_GetValueText(m_opCam.hIdxTUCam, &valText);

		vecValues.push_back(string(valText.pText));
	}

	LogMessage("Setting allowed rs mode settings", true);
	return SetAllowedValues(g_PropNameRSDir, vecValues);
}

int CMMTUCam::SetAllowedRSReset()
{
	if (NULL == m_opCam.hIdxTUCam)
		return DEVICE_NOT_CONNECTED;

	TUCAM_CAPA_ATTR capaAttr;
	capaAttr.idCapa = TUIDC_ROLLINGSCANRESET;

	if (TUCAMRET_SUCCESS != TUCAM_Capa_GetAttr(m_opCam.hIdxTUCam, &capaAttr))
	{
		return DEVICE_NOT_SUPPORTED;
	}

	char szBuf[64] = { 0 };
	TUCAM_VALUE_TEXT valText;
	valText.nID = TUIDC_ROLLINGSCANRESET;
	valText.nTextSize = 64;
	valText.pText = &szBuf[0];

	vector<string> vecValues;
	int nCnt = capaAttr.nValMax - capaAttr.nValMin + 1;

	for (int i = 0; i<nCnt; i++)
	{
		valText.dbValue = i;
		TUCAM_Capa_GetValueText(m_opCam.hIdxTUCam, &valText);

		vecValues.push_back(string(valText.pText));
	}

	LogMessage("Setting allowed rs mode settings", true);
	return SetAllowedValues(g_PropNameRSReset, vecValues);
}

int CMMTUCam::SetAllowedTestImg()
{
	if (NULL == m_opCam.hIdxTUCam)
		return DEVICE_NOT_CONNECTED;

	TUCAM_CAPA_ATTR capaAttr;
	capaAttr.idCapa = TUIDC_TESTIMGMODE;

	if (TUCAMRET_SUCCESS != TUCAM_Capa_GetAttr(m_opCam.hIdxTUCam, &capaAttr))
	{
		return DEVICE_NOT_SUPPORTED;
	}

	char szBuf[64] = { 0 };
	TUCAM_VALUE_TEXT valText;
	valText.nID = TUIDC_TESTIMGMODE;
	valText.nTextSize = 64;
	valText.pText = &szBuf[0];

	vector<string> vecValues;
	int nCnt = capaAttr.nValMax - capaAttr.nValMin + 1;

	for (int i = 0; i < nCnt; i++)
	{
		valText.dbValue = i;
		TUCAM_Capa_GetValueText(m_opCam.hIdxTUCam, &valText);

		vecValues.push_back(string(valText.pText));
	}

	LogMessage("Setting allowed test img settings", true);
	return SetAllowedValues(g_PropNameTestImg, vecValues);
}

int CMMTUCam::SetAllowedClrTemp()
{
	if (NULL == m_opCam.hIdxTUCam)
		return DEVICE_NOT_CONNECTED;

	TUCAM_PROP_ATTR propAttr;
	propAttr.nIdxChn = 0;
	propAttr.idProp = TUIDP_CLRTEMPERATURE;

	if (TUCAMRET_SUCCESS != TUCAM_Prop_GetAttr(m_opCam.hIdxTUCam, &propAttr))
	{
		return DEVICE_NOT_SUPPORTED;
	}

	char szBuf[64] = { 0 };
	TUCAM_VALUE_TEXT valText;
	valText.nID = TUIDP_CLRTEMPERATURE;
	valText.nTextSize = 64;
	valText.pText = &szBuf[0];

	vector<string> vecValues;
	int nCnt = (int)(propAttr.dbValMax - propAttr.dbValMin + 1);

	for (int i = 0; i < nCnt; i++)
	{
		valText.dbValue = i;
		TUCAM_Prop_GetValueText(m_opCam.hIdxTUCam, &valText);

		vecValues.push_back(string(valText.pText));
	}

	LogMessage("Setting allowed color temperature settings", true);
	return SetAllowedValues(g_PropNameCLRTEMP, vecValues);
}

int CMMTUCam::SetAllowedShutterMode()
{
    if (NULL == m_opCam.hIdxTUCam)
        return DEVICE_NOT_CONNECTED;

    TUCAM_CAPA_ATTR capaAttr;
    capaAttr.idCapa = TUIDC_SHUTTER;

    if (TUCAMRET_SUCCESS != TUCAM_Capa_GetAttr(m_opCam.hIdxTUCam, &capaAttr))
    {
        return DEVICE_NOT_SUPPORTED;
    }

    int nCnt = (int)(capaAttr.nValMax - capaAttr.nValMin + 1);

    char szBuf[64] = { 0 };
    TUCAM_VALUE_TEXT valText;
    valText.nTextSize = 64;
    valText.pText = &szBuf[0];
    valText.nID = TUIDC_SHUTTER;

    vector<string>  shutters;

    for (int i = 0; i<nCnt; ++i)
    {
        valText.dbValue = i;
        TUCAM_Capa_GetValueText(m_opCam.hIdxTUCam, &valText);

        shutters.push_back(string(valText.pText));
    }

    LogMessage("Setting allowed shutter mode settings", true);
    return SetAllowedValues(g_PropNameShutter, shutters);
}

/**
 * Required by the MM::Camera API
 * Please implement this yourself and do not rely on the base class implementation
 * The Base class implementation is deprecated and will be removed shortly
 */
int CMMTUCam::StartSequenceAcquisition(double interval)
{
    return StartSequenceAcquisition(LONG_MAX, interval, false);            
}

/**                                                                       
* Stop and wait for the Sequence thread finished                                   
*/                                                                        
int CMMTUCam::StopSequenceAcquisition()                                     
{
	TUDBG_PRINTF("[StopSequenceAcquisition]:Enter \n");
	if (thd_->IsStopped() /*&& !returnToSoftwareTriggers_*/)
	{
		if (m_bLiving)
		{
			m_bAcquisition = false;
			StopCapture();
		}
		return DEVICE_OK;
	}
        
    m_bLiving = false;

	if (NULL == m_opCam.hIdxTUCam)
	{
		return DEVICE_NOT_CONNECTED;
	}
    thd_->Stop(); 
    TUCAM_Buf_AbortWait(m_opCam.hIdxTUCam); 
    thd_->wait();

    TUCAM_Cap_Stop(m_opCam.hIdxTUCam);                      // Stop capture   
    ReleaseBuffer();

    // Switch back to software trigger mode if that is what the user used
	/*if (returnToSoftwareTriggers_)
	{
		TUCAM_Cap_GetTrigger(m_opCam.hIdxTUCam, &m_tgrAttr);
		m_tgrAttr.nTgrMode = TUCCM_TRIGGER_SOFTWARE;
		TUCAM_Cap_SetTrigger(m_opCam.hIdxTUCam, m_tgrAttr);
		returnToSoftwareTriggers_ = false;
	}*/
	m_bAcquisition = false;
    ///StartCapture();  // avoid wasting time in the SnapImage function 

    return DEVICE_OK;
} 

/**
* Simple implementation of Sequence Acquisition
* A sequence acquisition should run on its own thread and transport new images
* coming of the camera into the MMCore circular buffer.
*/
int CMMTUCam::StartSequenceAcquisition(long numImages, double interval_ms, bool stopOnOverflow)
{
    OutputDebugString("[StartSequenceAcquisition]:Enter\n");

    if (IsCapturing())
        return DEVICE_CAMERA_BUSY_ACQUIRING;
    if (m_bLiving)
    {
       StopCapture();
    }

    // Switch to standard trigger mode if we are currently in software trigger mode
    TUCAM_Cap_GetTrigger(m_opCam.hIdxTUCam, &m_tgrAttr);
	if (TUCCM_TRIGGER_SOFTWARE == m_tgrAttr.nTgrMode) 
	{
		returnToSoftwareTriggers_ = true;
		//m_tgrAttr.nTgrMode = TUCCM_SEQUENCE;
		TUCAM_Cap_SetTrigger(m_opCam.hIdxTUCam, m_tgrAttr);
	}

    // initialize image buffer
    int nRet = StartCapture();
    if (nRet != DEVICE_OK)
        return nRet;

    int ret = GetCoreCallback()->PrepareForAcq(this);
    if (ret != DEVICE_OK)
        return ret;

    sequenceStartTime_ = GetCurrentMMTime();
    imageCounter_ = 0;
    thd_->Start(numImages,interval_ms);
    stopOnOverflow_ = stopOnOverflow;
	m_bAcquisition = true;
    return DEVICE_OK;
}

/*
 * Inserts Image and MetaData into MMCore circular Buffer
 */
int CMMTUCam::InsertImage()
{
    MM::MMTime timeStamp = this->GetCurrentMMTime();
    char label[MM::MaxStrLength];
    this->GetLabel(label);

    // Important:  metadata about the image are generated here:
    Metadata md;
    md.put("Camera", label);
    md.put(MM::g_Keyword_Metadata_StartTime, CDeviceUtils::ConvertToString(sequenceStartTime_.getMsec()));
    md.put(MM::g_Keyword_Elapsed_Time_ms, CDeviceUtils::ConvertToString((timeStamp - sequenceStartTime_).getMsec()));
    md.put(MM::g_Keyword_Metadata_ROI_X, CDeviceUtils::ConvertToString( (long) roiX_)); 
    md.put(MM::g_Keyword_Metadata_ROI_Y, CDeviceUtils::ConvertToString( (long) roiY_)); 

    imageCounter_++;

//     char buf[MM::MaxStrLength];
//     GetProperty(MM::g_Keyword_Binning, buf);
//     md.put(MM::g_Keyword_Binning, buf);

    char szTemp[256] = {0};
    sprintf(szTemp, "%.3f", m_fCurTemp);
    md.put("Temperature", szTemp); 

    MMThreadGuard g(imgPixelsLock_);

    const unsigned char* pI;
    pI = GetImageBuffer();

    unsigned int w = GetImageWidth();
    unsigned int h = GetImageHeight();
    unsigned int b = GetImageBytesPerPixel();

    int ret = GetCoreCallback()->InsertImage(this, pI, w, h, b, md.Serialize().c_str());

    if (!stopOnOverflow_ && ret == DEVICE_BUFFER_OVERFLOW)
    {
        // do not stop on overflow - just reset the buffer
        GetCoreCallback()->ClearImageBuffer(this);
        // don't process this same image again...
        return GetCoreCallback()->InsertImage(this, pI, w, h, b, md.Serialize().c_str(), false);
    } else
        return ret;
}

/*
 * Do actual capturing
 * Called from inside the thread  
 */
int CMMTUCam::RunSequenceOnThread(MM::MMTime startTime)
{
    int ret = DEVICE_ERR;

    // Trigger
    if (triggerDevice_.length() > 0) 
    {
        MM::Device* triggerDev = GetDevice(triggerDevice_.c_str());
        if (triggerDev != 0) 
        {
            LogMessage("trigger requested");
            triggerDev->SetProperty("Trigger","+");
        }
    }

    ret = WaitForFrame(img_);

/*   
    if (!fastImage_)
    {
        GenerateSyntheticImage(img_, GetSequenceExposure());
    }
*/
	if (DEVICE_OK == ret)
	{
		ret = InsertImage();
	}
   
   /* while (((double) (this->GetCurrentMMTime() - startTime).getMsec() / imageCounter_) < this->GetSequenceExposure())
    {
        CDeviceUtils::SleepMs(1);
    }*/

    if (ret != DEVICE_OK)
    {
        return ret;
    }

    return ret;
}

bool CMMTUCam::IsCapturing() 
{
     return !thd_->IsStopped();
}

/*
 * called from the thread function before exit 
 */
void CMMTUCam::OnThreadExiting() throw()
{
   try
   {
      LogMessage(g_Msg_SEQUENCE_ACQUISITION_THREAD_EXITING);
      GetCoreCallback()?GetCoreCallback()->AcqFinished(this,0):DEVICE_OK;
   }
   catch(...)
   {
      LogMessage(g_Msg_EXCEPTION_IN_ON_THREAD_EXITING, false);
   }
}

CTUCamThread::CTUCamThread(CMMTUCam* pCam)
   :intervalMs_(default_intervalMS)
   ,numImages_(default_numImages)
   ,imageCounter_(0)
   ,stop_(true)
   ,suspend_(false)
   ,camera_(pCam)
   ,startTime_(0)
   ,actualDuration_(0)
   ,lastFrameTime_(0)
{};

CTUCamThread::~CTUCamThread() {};

void CTUCamThread::Stop() 
{
    MMThreadGuard g(this->stopLock_);
    stop_=true;
}

void CTUCamThread::Start(long numImages, double intervalMs)
{
    OutputDebugString("[CTUCamThread]:Start");
    MMThreadGuard g1(this->stopLock_);
    MMThreadGuard g2(this->suspendLock_);
    numImages_=numImages;
    intervalMs_=intervalMs;
    imageCounter_=0;
    stop_ = false;
    suspend_=false;
    activate();
    actualDuration_ = 0;
    startTime_= camera_->GetCurrentMMTime();
    lastFrameTime_ = 0;
}

bool CTUCamThread::IsStopped()
{
    MMThreadGuard g(this->stopLock_);
    return stop_;
}

void CTUCamThread::Suspend() 
{
    MMThreadGuard g(this->suspendLock_);
    suspend_ = true;
}

bool CTUCamThread::IsSuspended()
{
    MMThreadGuard g(this->suspendLock_);
    return suspend_;
}

void CTUCamThread::Resume()
{
    MMThreadGuard g(this->suspendLock_);
    suspend_ = false;
}

int CTUCamThread::svc(void) throw()
{
    int ret=DEVICE_ERR;
    try 
    {
        do
        {  
            ret = camera_->RunSequenceOnThread(startTime_);

        } while (DEVICE_OK == ret && !IsStopped() && imageCounter_++ < numImages_-1);
        if (IsStopped())
            camera_->LogMessage("SeqAcquisition interrupted by the user\n");
    }catch(...){
        camera_->LogMessage(g_Msg_EXCEPTION_IN_THREAD, false);
    }
    stop_=true;

    actualDuration_ = camera_->GetCurrentMMTime() - startTime_;
    camera_->OnThreadExiting();
    return ret;
}


///////////////////////////////////////////////////////////////////////////////
// CMMTUCam Action handlers
///////////////////////////////////////////////////////////////////////////////

/*
* this Read Only property will update whenever any property is modified
*/
int CMMTUCam::OnTestProperty(MM::PropertyBase* pProp, MM::ActionType eAct, long indexx)
{
    if (eAct == MM::BeforeGet)
    {
        pProp->Set(testProperty_[indexx]);
    }
    else if (eAct == MM::AfterSet)
    {
        pProp->Get(testProperty_[indexx]);
    }
    return DEVICE_OK;
}


/**
* Handles "Binning" property.
*/
int CMMTUCam::OnBinning(MM::PropertyBase* pProp, MM::ActionType eAct)
{
    if (NULL == m_opCam.hIdxTUCam)
        return DEVICE_NOT_CONNECTED;

    int ret = DEVICE_ERR;
    switch(eAct)
    {
    case MM::AfterSet:
        {
            if(IsCapturing())
                return DEVICE_CAMERA_BUSY_ACQUIRING;

            // the user just set the new value for the property, so we have to
            // apply this value to the 'hardware'.

            string val;
            pProp->Get(val);
            if (val.length() != 0)
            {
                TUCAM_CAPA_ATTR capaAttr;
                capaAttr.idCapa = TUIDC_RESOLUTION;

                if (TUCAMRET_SUCCESS == TUCAM_Capa_GetAttr(m_opCam.hIdxTUCam, &capaAttr))
                {
                    m_bLiving = false;
                    TUCAM_Cap_Stop(m_opCam.hIdxTUCam);      // Stop capture   
                    ReleaseBuffer();

                    char szBuf[64] = {0};
                    TUCAM_VALUE_TEXT valText;
                    valText.nID       = TUIDC_RESOLUTION;
                    valText.nTextSize = 64;
                    valText.pText     = &szBuf[0];

                    int i = 0;
                    int nCnt = capaAttr.nValMax - capaAttr.nValMin + 1;

                    for (; i<nCnt; i++)
                    {
                        valText.dbValue = i;
                        TUCAM_Capa_GetValueText(m_opCam.hIdxTUCam, &valText); 
                   
                        if (0 == val.compare(valText.pText))
                        {
                            TUCAM_Capa_SetValue(m_opCam.hIdxTUCam, TUIDC_RESOLUTION, i);
                            if (m_nPID == PID_FL_9BW || PID_FL_9BW_LT == m_nPID || m_nPID == PID_FL_20BW || m_nPID == PID_FL_26BW)
							{
								UpdateExpRange();
							}
                            break;
                        }                         
                    }

                    if (PID_FL_9BW == m_nPID || PID_FL_9BW_LT == m_nPID)
                    {
                        if (0 == i)
                        {
                            TUCAM_Capa_SetValue(m_opCam.hIdxTUCam, TUIDC_BINNING_SUM, 0);
/*
                            SetPropertyLimits(g_PropNameLLev, 0, 16383);
                            SetPropertyLimits(g_PropNameRLev, 1, 16384);

                            SetProperty(g_PropNameLLev, "0");
                            SetProperty(g_PropNameRLev, "16384");
*/
                        }
                        else
                        {
                            valText.nID = TUIDC_BINNING_SUM;
                            capaAttr.idCapa = TUIDC_BINNING_SUM;
                            TUCAM_Capa_GetAttr(m_opCam.hIdxTUCam, &capaAttr);
                            nCnt = capaAttr.nValMax - capaAttr.nValMin + 1;                            

                            for (int j = 1; j < nCnt; j++)
                            {
                                valText.dbValue = j;
                                TUCAM_Capa_GetValueText(m_opCam.hIdxTUCam, &valText);

                                if (0 == val.compare(valText.pText))
                                {
                                    TUCAM_Capa_SetValue(m_opCam.hIdxTUCam, TUIDC_BINNING_SUM, j);
                                    UpdateExpRange();
                                    break;
                                }
                            }
/*
                            SetPropertyLimits(g_PropNameLLev, 0, 65534);
                            SetPropertyLimits(g_PropNameRLev, 1, 65535);

                            SetProperty(g_PropNameLLev, "0");
                            SetProperty(g_PropNameRLev, "65535");
*/
                        }
                    }
                    
                    if (PID_FL_9BW == m_nPID || PID_FL_9BW_LT == m_nPID || PID_FL_26BW == m_nPID)
                    {
                        UpdateLevelsRange();
                    }

                    m_bROI = FALSE;

//                  StartCapture();
                    ResizeImageBuffer();

                    roiX_ = 0;
                    roiY_ = 0;
                }

                OnPropertyChanged(MM::g_Keyword_Binning, val.c_str());

                ret = DEVICE_OK;
            }
        }
        break;
    case MM::BeforeGet:
        {
            if (PID_FL_9BW == m_nPID || PID_FL_9BW_LT == m_nPID)
            {
                int nIdx = 0;
                TUCAM_Capa_GetValue(m_opCam.hIdxTUCam, TUIDC_BINNING_SUM, &nIdx);

                char szBuf[64] = { 0 };
                TUCAM_VALUE_TEXT valText;
                valText.nTextSize = 64;
                valText.pText = &szBuf[0];

                if (nIdx > 0)
                {
                    valText.dbValue = nIdx;
                    valText.nID = TUIDC_BINNING_SUM;
                    
                    TUCAM_Capa_GetValueText(m_opCam.hIdxTUCam, &valText);
                    pProp->Set(valText.pText);
                }
                else
                {
                    valText.dbValue = 0;
                    valText.nID = TUIDC_RESOLUTION;
                    
                    TUCAM_Capa_GetValueText(m_opCam.hIdxTUCam, &valText);
                    pProp->Set(valText.pText);
                }
            }
            else
            {
                int nIdx = 0;
                TUCAM_Capa_GetValue(m_opCam.hIdxTUCam, TUIDC_RESOLUTION, &nIdx);

                char szBuf[64] = { 0 };
                TUCAM_VALUE_TEXT valText;
                valText.nID = TUIDC_RESOLUTION;
                valText.nTextSize = 64;
                valText.pText = &szBuf[0];

                valText.dbValue = nIdx;
                TUCAM_Capa_GetValueText(m_opCam.hIdxTUCam, &valText);
                pProp->Set(valText.pText);
            }
            
            ret = DEVICE_OK;
        }
        break;
    default:
        break;
    }
    return ret; 
}

/**
* Handles "BinningSum" property.
*/
int CMMTUCam::OnBinningSum(MM::PropertyBase* pProp, MM::ActionType eAct)
{
    if (NULL == m_opCam.hIdxTUCam)
        return DEVICE_NOT_CONNECTED;

    int ret = DEVICE_ERR;
    switch (eAct)
    {
    case MM::AfterSet:
    {
        string val;
        pProp->Get(val);

        if (val.length() != 0)
        {
            TUCAM_CAPA_ATTR capaAttr;
            capaAttr.idCapa = TUIDC_BINNING_SUM;

            if (TUCAMRET_SUCCESS == TUCAM_Capa_GetAttr(m_opCam.hIdxTUCam, &capaAttr))
            {
                char szBuf[64] = { 0 };
                TUCAM_VALUE_TEXT valText;
                valText.nID = TUIDC_BINNING_SUM;
                valText.nTextSize = 64;
                valText.pText = &szBuf[0];

                int nCnt = (int)(capaAttr.nValMax - capaAttr.nValMin + 1);

                for (int i = 0; i<nCnt; i++)
                {
                    valText.dbValue = i;
                    TUCAM_Capa_GetValueText(m_opCam.hIdxTUCam, &valText);

                    if (0 == val.compare(valText.pText))
                    {
                        TUCAM_Capa_SetValue(m_opCam.hIdxTUCam, TUIDC_BINNING_SUM, i);
                        break;
                    }
                }

                m_bROI = FALSE;
                ResizeImageBuffer();

                roiX_ = 0;
                roiY_ = 0;
            }

            UpdateLevelsRange();
            OnPropertyChanged(g_PropNameBinningSum, val.c_str());

            ret = DEVICE_OK;
        }
    }
    break;
    case  MM::BeforeGet:
    {
        int val = 0;
        TUCAM_Capa_GetValue(m_opCam.hIdxTUCam, TUIDC_BINNING_SUM, &val);

        char szBuf[64] = { 0 };
        TUCAM_VALUE_TEXT valText;
        valText.nID = TUIDC_BINNING_SUM;
        valText.nTextSize = 64;
        valText.pText = &szBuf[0];

        valText.dbValue = val;
        TUCAM_Capa_GetValueText(m_opCam.hIdxTUCam, &valText);

        pProp->Set(valText.pText);

        ret = DEVICE_OK;
    }
    break;
    default:
        break;
    }
    return ret; 
}

/**
* Handles "PixelClock" property.
*/
int CMMTUCam::OnPixelClock(MM::PropertyBase* pProp, MM::ActionType eAct)
{
    if (NULL == m_opCam.hIdxTUCam)
        return DEVICE_NOT_CONNECTED;

    int ret = DEVICE_ERR;
    switch(eAct)
    {
    case MM::AfterSet:
        {
            if(IsCapturing())
                return DEVICE_CAMERA_BUSY_ACQUIRING;

            // the user just set the new value for the property, so we have to
            // apply this value to the 'hardware'.

            string val;
            pProp->Get(val);

            if (val.length() != 0)
            {
                TUCAM_CAPA_ATTR capaAttr;
                capaAttr.idCapa = TUIDC_PIXELCLOCK;

                if (TUCAMRET_SUCCESS == TUCAM_Capa_GetAttr(m_opCam.hIdxTUCam, &capaAttr))
                {
                    char szBuf[64] = {0};
                    TUCAM_VALUE_TEXT valText;
                    valText.nID       = TUIDC_PIXELCLOCK;
                    valText.nTextSize = 64;
                    valText.pText     = &szBuf[0];

                    int nCnt = capaAttr.nValMax - capaAttr.nValMin + 1;

                    for (int i=0; i<nCnt; i++)
                    {
                        valText.dbValue = i;
                        TUCAM_Capa_GetValueText(m_opCam.hIdxTUCam, &valText); 
                     
                        if (0 == val.compare(valText.pText))
                        {
                            TUCAM_Capa_SetValue(m_opCam.hIdxTUCam, TUIDC_PIXELCLOCK, i);
                            break;
                        }                         
                    }
                }

                OnPropertyChanged(g_PropNamePCLK, val.c_str());

                ret = DEVICE_OK;
            }
        }
        break;
    case MM::BeforeGet:
        {
            int nIdx = 0;
            TUCAM_Capa_GetValue(m_opCam.hIdxTUCam, TUIDC_PIXELCLOCK, &nIdx);

            char szBuf[64] = {0};
            TUCAM_VALUE_TEXT valText;
            valText.nID       = TUIDC_PIXELCLOCK;
            valText.nTextSize = 64;
            valText.pText     = &szBuf[0];

            valText.dbValue = nIdx;
            TUCAM_Capa_GetValueText(m_opCam.hIdxTUCam, &valText); 

            pProp->Set(valText.pText);

            ret = DEVICE_OK;
        }
        break;
    default:
        break;
    }

    return ret; 
}

/**
* Handles "Exposure" property.
*/
int CMMTUCam::OnExposure(MM::PropertyBase* pProp, MM::ActionType eAct)
{
    if (NULL == m_opCam.hIdxTUCam)
        return DEVICE_NOT_CONNECTED;

    int ret = DEVICE_ERR;
    switch(eAct)
    {
    case MM::AfterSet:
        {
            double dblExp;
            pProp->Get(dblExp);          

            if (dblExp < exposureMinimum_)
            {
               dblExp = exposureMinimum_;
            }
            else if (dblExp > exposureMaximum_) {
               dblExp = exposureMaximum_;
            }
            TUCAM_Prop_SetValue(m_opCam.hIdxTUCam, TUIDP_EXPOSURETM, dblExp);

            ret = DEVICE_OK;
        }
        break;
    case MM::BeforeGet:
        {
            double dblExp = 0.0f;

            TUCAM_Prop_GetValue(m_opCam.hIdxTUCam, TUIDP_EXPOSURETM, &dblExp);
            pProp->Set(dblExp);

            ret = DEVICE_OK;
        }
        break;
    default:
        break;
    }

    return ret;
}

/**
* Handles "Brightness" property.
*/
int CMMTUCam::OnBrightness(MM::PropertyBase* pProp, MM::ActionType eAct)
{
	if (NULL == m_opCam.hIdxTUCam)
		return DEVICE_NOT_CONNECTED;

	int ret = DEVICE_ERR;
	switch (eAct)
	{
	case MM::AfterSet:
	{
		double dbVal = 0.0f;
		pProp->Get(dbVal);

		TUCAM_Prop_SetValue(m_opCam.hIdxTUCam, TUIDP_BRIGHTNESS, dbVal);

		ret = DEVICE_OK;
	}
	break;
	case MM::BeforeGet:
	{
		double dbVal = 0.0f;

		TUCAM_Prop_GetValue(m_opCam.hIdxTUCam, TUIDP_BRIGHTNESS, &dbVal);

		pProp->Set(dbVal);

		ret = DEVICE_OK;
	}
	break;
	default:
		break;
	}

	return ret;
}

/**
* Handles "OnPixelRatio" property.
*/
int CMMTUCam::OnPixelRatio(MM::PropertyBase* pProp, MM::ActionType eAct)
{
	if (NULL == m_opCam.hIdxTUCam)
		return DEVICE_NOT_CONNECTED;

	int ret = DEVICE_ERR;
	switch (eAct)
	{
	case MM::AfterSet:
	{
		double dbVal = 0.0f;
		pProp->Get(dbVal);

		TUCAM_Prop_SetValue(m_opCam.hIdxTUCam, TUIDP_PIXELRATIO, dbVal);

		ret = DEVICE_OK;
	}
	break;
	case MM::BeforeGet:
	{
		double dbVal = 0.0f;

		TUCAM_Prop_GetValue(m_opCam.hIdxTUCam, TUIDP_PIXELRATIO, &dbVal);

		pProp->Set(dbVal);

		ret = DEVICE_OK;
	}
	break;
	default:
		break;
	}

	return ret;
}

/**
* Handles "GlobalGain" property.
*/
int CMMTUCam::OnGlobalGain(MM::PropertyBase* pProp, MM::ActionType eAct)
{
    if (NULL == m_opCam.hIdxTUCam)
        return DEVICE_NOT_CONNECTED;

    int ret = DEVICE_ERR;
    switch(eAct)
    {
    case MM::AfterSet:
        {
            double dblGain;
            pProp->Get(dblGain);          

            TUCAM_Prop_SetValue(m_opCam.hIdxTUCam, TUIDP_GLOBALGAIN, dblGain);

            ret = DEVICE_OK;
        }
        break;
    case MM::BeforeGet:
        {
            double dblGain = 0.0f;

            TUCAM_Prop_GetValue(m_opCam.hIdxTUCam, TUIDP_GLOBALGAIN, &dblGain);
            pProp->Set(dblGain);

            ret = DEVICE_OK;
        }
        break;
    default:
        break;
    }

    return ret;
}

/**
* Handles "FrameRate" property.
*/
int CMMTUCam::OnFrameRate(MM::PropertyBase* pProp, MM::ActionType eAct)
{
	if (NULL == m_opCam.hIdxTUCam)
		return DEVICE_NOT_CONNECTED;

	int ret = DEVICE_ERR;
	switch (eAct)
	{
	case MM::AfterSet:
	{
		double dblRate;
		pProp->Get(dblRate);

		TUCAM_Prop_SetValue(m_opCam.hIdxTUCam, TUIDP_FRAME_RATE, dblRate);

		ret = DEVICE_OK;
	}
	break;
	case MM::BeforeGet:
	{
		double dblRate = 0.0f;

		TUCAM_Prop_GetValue(m_opCam.hIdxTUCam, TUIDP_FRAME_RATE, &dblRate);
		pProp->Set(dblRate);

		ret = DEVICE_OK;
	}
	break;
	default:
		break;
	}

	return ret;
}

/**
* Handles "SensorReset" property.
*/
int CMMTUCam::OnSensorReset(MM::PropertyBase* pProp, MM::ActionType eAct)
{
	if (NULL == m_opCam.hIdxTUCam)
		return DEVICE_NOT_CONNECTED;

	int ret = DEVICE_ERR;
	switch (eAct)
	{
	case MM::AfterSet:
	{
		string val;
		pProp->Get(val);
		if (val.length() != 0)
		{
			TUCAM_Capa_SetValue(m_opCam.hIdxTUCam, TUIDC_SENSORRESET, 0x00);
		}

		ret = DEVICE_OK;
	}
	break;
	case MM::BeforeGet:
	{
		pProp->Set(g_PropNameReset);

		ret = DEVICE_OK;
	}
	break;
	default:
		break;
	}

	return ret;
}

/**
* Handles "CMSMode" property.
*/
int CMMTUCam::OnCMSMode(MM::PropertyBase* pProp, MM::ActionType eAct)
{
    if (NULL == m_opCam.hIdxTUCam)
        return DEVICE_NOT_CONNECTED;

    int ret = DEVICE_ERR;
    switch(eAct)
    {
    case MM::AfterSet:
        {
			int nVal = 0;

            string val;
            pProp->Get(val);
            if (val.length() != 0)
            {
				if (0 == val.compare(g_CMS_ON))
				{
					TUCAM_Prop_SetValue(m_opCam.hIdxTUCam, TUIDP_GLOBALGAIN, 0);

					if (TUCAMRET_SUCCESS == TUCAM_Capa_GetValue(m_opCam.hIdxTUCam, TUIDC_IMGMODESELECT, &nVal))
					{
						if (1 != nVal)
						{
							TUCAM_Capa_SetValue(m_opCam.hIdxTUCam, TUIDC_IMGMODESELECT, 1);  
						}
					}                    
				}
				else
				{
					TUCAM_Prop_SetValue(m_opCam.hIdxTUCam, TUIDP_GLOBALGAIN, m_nIdxGain);

					if (TUCAMRET_SUCCESS == TUCAM_Capa_GetValue(m_opCam.hIdxTUCam, TUIDC_IMGMODESELECT, &nVal))
					{
						if (0 != nVal)
						{
							TUCAM_Capa_SetValue(m_opCam.hIdxTUCam, TUIDC_IMGMODESELECT, 0);
						}
					} 
				}

				OnPropertyChanged(g_PropNameFLPH, val.c_str());
				
                ret = DEVICE_OK;
            }
        }
        break;
    case MM::BeforeGet:
        {
            int nVal = 0;
            TUCAM_Capa_GetValue(m_opCam.hIdxTUCam, TUIDC_IMGMODESELECT, &nVal);

            string val;
            pProp->Get(val);

			if (1 == nVal)  
			{
				pProp->Set(g_CMS_ON);
			}
			else
			{
				pProp->Set(g_CMS_OFF);
			}

            ret = DEVICE_OK;
        }
        break;
    default:
        break;
    }

    return ret;
}

/**
* Handles "LEDMode" property.
*/
int CMMTUCam::OnLEDMode(MM::PropertyBase* pProp, MM::ActionType eAct)
{
    if (NULL == m_opCam.hIdxTUCam)
        return DEVICE_NOT_CONNECTED;

    int ret = DEVICE_ERR;
    switch(eAct)
    {
    case MM::AfterSet:
        {
            string val;
            pProp->Get(val);
            if (val.length() != 0)
            {
				if (0 == val.compare(g_LED_ON))
				{
					TUCAM_Capa_SetValue(m_opCam.hIdxTUCam, TUIDC_LEDENBALE, 1);  
				}
				else
				{
					TUCAM_Capa_SetValue(m_opCam.hIdxTUCam, TUIDC_LEDENBALE, 0);  
				}
				
                ret = DEVICE_OK;
            }
        }
        break;
    case MM::BeforeGet:
        {
            int nVal = 0;
            TUCAM_Capa_GetValue(m_opCam.hIdxTUCam, TUIDC_LEDENBALE, &nVal);

            string val;
            pProp->Get(val);

			if (1 == nVal)  
			{
				pProp->Set(g_LED_ON);
			}
			else
			{
				pProp->Set(g_LED_OFF);
			}

            ret = DEVICE_OK;
        }
        break;
    default:
        break;
    }

    return ret;
}

/**
* Handles "PIMode" property.
*/
int CMMTUCam::OnPIMode(MM::PropertyBase* pProp, MM::ActionType eAct)
{
	if (NULL == m_opCam.hIdxTUCam)
		return DEVICE_NOT_CONNECTED;

	int ret = DEVICE_ERR;
	switch (eAct)
	{
	case MM::AfterSet:
	{
		string val;
		pProp->Get(val);
		if (val.length() != 0)
		{
			if (0 == val.compare(g_PI_ON))
			{
				TUCAM_Capa_SetValue(m_opCam.hIdxTUCam, TUIDC_ENABLEPI, 1);
			}
			else
			{
				TUCAM_Capa_SetValue(m_opCam.hIdxTUCam, TUIDC_ENABLEPI, 0);
			}

			ret = DEVICE_OK;
		}
	}
	break;
	case MM::BeforeGet:
	{
		int nVal = 0;
		TUCAM_Capa_GetValue(m_opCam.hIdxTUCam, TUIDC_ENABLEPI, &nVal);

		string val;
		pProp->Get(val);

		if (1 == nVal)
		{
			pProp->Set(g_PI_ON);
		}
		else
		{
			pProp->Set(g_PI_OFF);
		}

		ret = DEVICE_OK;
	}
	break;
	default:
		break;
	}

	return ret;
}

/**
* Handles "TECMode" property.
*/
int CMMTUCam::OnTECMode(MM::PropertyBase* pProp, MM::ActionType eAct)
{
	if (NULL == m_opCam.hIdxTUCam)
		return DEVICE_NOT_CONNECTED;

	int ret = DEVICE_ERR;
	switch (eAct)
	{
	case MM::AfterSet:
	{
		string val;
		pProp->Get(val);
		if (val.length() != 0)
		{
			if (0 == val.compare(g_TEC_ON))
			{
				TUCAM_Capa_SetValue(m_opCam.hIdxTUCam, TUIDC_ENABLETEC, 1);
			}
			else
			{
				TUCAM_Capa_SetValue(m_opCam.hIdxTUCam, TUIDC_ENABLETEC, 0);
			}

			ret = DEVICE_OK;
		}
	}
	break;
	case MM::BeforeGet:
	{
		int nVal = 0;
		TUCAM_Capa_GetValue(m_opCam.hIdxTUCam, TUIDC_ENABLETEC, &nVal);

		string val;
		pProp->Get(val);

		if (1 == nVal)
		{
			pProp->Set(g_TEC_ON);
		}
		else
		{
			pProp->Set(g_TEC_OFF);
		}

		ret = DEVICE_OK;
	}
	break;
	default:
		break;
	}

	return ret;
}

/**
* Handles "OnTriOutEnable" property.
*/
int CMMTUCam::OnTriOutEnable(MM::PropertyBase* pProp, MM::ActionType eAct)
{
	if (NULL == m_opCam.hIdxTUCam)
		return DEVICE_NOT_CONNECTED;

	int ret = DEVICE_ERR;
	switch (eAct)
	{
	case MM::AfterSet:
	{
		string val;
		pProp->Get(val);
		if (val.length() != 0)
		{
			if (0 == val.compare(g_OT_ON))
			{
				TUCAM_Capa_SetValue(m_opCam.hIdxTUCam, TUIDC_ENABLETRIOUT, 1);
			}
			else
			{
				TUCAM_Capa_SetValue(m_opCam.hIdxTUCam, TUIDC_ENABLETRIOUT, 0);
			}

			ret = DEVICE_OK;
		}
	}
	break;
	case MM::BeforeGet:
	{
		int nVal = 0;
		TUCAM_Capa_GetValue(m_opCam.hIdxTUCam, TUIDC_ENABLETRIOUT, &nVal);

		string val;
		pProp->Get(val);

		if (1 == nVal)
		{
			pProp->Set(g_OT_ON);
		}
		else
		{
			pProp->Set(g_OT_OFF);
		}

		ret = DEVICE_OK;
	}
	break;
	default:
		break;
	}

	return ret;
}

/**
* Handles "OnRollingScanMode" property.
*/
int CMMTUCam::OnRollingScanMode(MM::PropertyBase* pProp, MM::ActionType eAct)
{
	if (NULL == m_opCam.hIdxTUCam)
		return DEVICE_NOT_CONNECTED;

	int ret = DEVICE_ERR;
	switch (eAct)
	{
	case MM::AfterSet:
	{
		//if (IsCapturing())
			//return DEVICE_CAMERA_BUSY_ACQUIRING;

		// the user just set the new value for the property, so we have to
		// apply this value to the 'hardware'.

		string val;
		pProp->Get(val);

		if (val.length() != 0)
		{
			TUCAM_CAPA_ATTR capaAttr;
			capaAttr.idCapa = TUIDC_ROLLINGSCANMODE;

			if (TUCAMRET_SUCCESS == TUCAM_Capa_GetAttr(m_opCam.hIdxTUCam, &capaAttr))
			{
				char szBuf[64] = { 0 };
				TUCAM_VALUE_TEXT valText;
				valText.nID = TUIDC_ROLLINGSCANMODE;
				valText.nTextSize = 64;
				valText.pText = &szBuf[0];

				int nCnt = capaAttr.nValMax - capaAttr.nValMin + 1;

				for (int i = 0; i<nCnt; i++)
				{
					valText.dbValue = i;
					TUCAM_Capa_GetValueText(m_opCam.hIdxTUCam, &valText);

					if (0 == val.compare(valText.pText))
					{
						m_rsPara.nMode = i;
						TUCAM_Capa_SetValue(m_opCam.hIdxTUCam, TUIDC_ROLLINGSCANMODE, i);
						if (1 == i)
						{
							TUCAM_Capa_SetValue(m_opCam.hIdxTUCam, TUIDC_ROLLINGSCANLTD, m_rsPara.nLTDelay);
							m_rsPara.nSlitHeight = max(min(LineIntervalCal(m_rsPara.nLTDelay), m_rsPara.nSlitHeightMax), m_rsPara.nSlitHeightMin);
							m_rsPara.dbLineInvalTm = LineIntervalTime(m_rsPara.nLTDelay);
						}
						else if (2 == i)
						{
							UpdateSlitHeightRange();
							TUCAM_Capa_SetValue(m_opCam.hIdxTUCam, TUIDC_ROLLINGSCANSLIT, m_rsPara.nSlitHeight);
							m_rsPara.nLTDelay = max(min(LineIntervalCal(m_rsPara.nSlitHeight / m_rsPara.nSlitHeightStep), m_rsPara.nLTDelayMax), m_rsPara.nLTDelayMin);
							m_rsPara.dbLineInvalTm = LineIntervalTime(m_rsPara.nLTDelay);
							TUCAM_Capa_SetValue(m_opCam.hIdxTUCam, TUIDC_ROLLINGSCANLTD, m_rsPara.nLTDelay);
						}
						else
						{
							///m_rsPara.nLTDelay = 1;
							TUCAM_Capa_SetValue(m_opCam.hIdxTUCam, TUIDC_ROLLINGSCANLTD, 0);
							m_rsPara.dbLineInvalTm = LineIntervalTime(0);
						}
						break;
					}
				}
				OnPropertyChanged(g_PropNameRSMode, val.c_str());
			}
		}
		ret = DEVICE_OK;
	}
	break;
	case MM::BeforeGet:
	{
		int nIdx = 0;
		TUCAM_Capa_GetValue(m_opCam.hIdxTUCam, TUIDC_ROLLINGSCANMODE, &nIdx);

		char szBuf[64] = { 0 };
		TUCAM_VALUE_TEXT valText;
		valText.nID = TUIDC_ROLLINGSCANMODE;
		valText.nTextSize = 64;
		valText.pText = &szBuf[0];

		valText.dbValue = nIdx;
		TUCAM_Capa_GetValueText(m_opCam.hIdxTUCam, &valText);

		pProp->Set(valText.pText);

		ret = DEVICE_OK;
	}
	break;
	default:
		break;
	}

	return ret;
}

/**
* Handles "OnRollingScanLtd" property.
*/
int CMMTUCam::OnRollingScanLtd(MM::PropertyBase* pProp, MM::ActionType eAct)
{
	if (NULL == m_opCam.hIdxTUCam)
		return DEVICE_NOT_CONNECTED;

	int ret = DEVICE_ERR;
	switch (eAct)
	{
	case MM::AfterSet:
	{
		long lVal = 0;
		pProp->Get(lVal);

		if (0x01 == m_rsPara.nMode)
		{
			if (TUCAMRET_SUCCESS == TUCAM_Capa_SetValue(m_opCam.hIdxTUCam, TUIDC_ROLLINGSCANLTD, lVal))
			{
				m_rsPara.nLTDelay = lVal;
				m_rsPara.nSlitHeight = max(min(LineIntervalCal(m_rsPara.nLTDelay), m_rsPara.nSlitHeightMax), m_rsPara.nSlitHeightMin);
				m_rsPara.dbLineInvalTm = LineIntervalTime(m_rsPara.nLTDelay);
			}
		}
		ret = DEVICE_OK;
	}
	break;
	case  MM::BeforeGet:
	{
		int nVal = 0;

		if (TUCAMRET_SUCCESS == TUCAM_Capa_GetValue(m_opCam.hIdxTUCam, TUIDC_ROLLINGSCANLTD, &nVal))
		{
			nVal = m_rsPara.nLTDelay;
		}
		pProp->Set((long)(nVal));

		ret = DEVICE_OK;
	}
	break;
	default:
		break;
	}

	return ret;
}

/**
* Handles "OnRollinScanSlit" property.
*/
int CMMTUCam::OnRollinScanSlit(MM::PropertyBase* pProp, MM::ActionType eAct)
{
	if (NULL == m_opCam.hIdxTUCam)
		return DEVICE_NOT_CONNECTED;

	int ret = DEVICE_ERR;
	switch (eAct)
	{
	case MM::AfterSet:
	{
		long lVal = 0;
		pProp->Get(lVal);

		if (0x02 == m_rsPara.nMode)
		{
			if (0x02 == m_rsPara.nSlitHeightStep)
			{
				lVal = ((lVal + 1) >> 1) << 1;
			}
			if (TUCAMRET_SUCCESS == TUCAM_Capa_SetValue(m_opCam.hIdxTUCam, TUIDC_ROLLINGSCANSLIT, lVal))
			{
				m_rsPara.nSlitHeight = lVal;
				m_rsPara.nLTDelay = max(min(LineIntervalCal(m_rsPara.nSlitHeight / m_rsPara.nSlitHeightStep), m_rsPara.nLTDelayMax), m_rsPara.nLTDelayMin);
				TUCAM_Capa_SetValue(m_opCam.hIdxTUCam, TUIDC_ROLLINGSCANLTD, m_rsPara.nLTDelay);
				m_rsPara.dbLineInvalTm = LineIntervalTime(m_rsPara.nLTDelay);
			}
		}
		ret = DEVICE_OK;
	}
	break;
	case  MM::BeforeGet:
	{
		int nVal = 0;

		if (TUCAMRET_SUCCESS == TUCAM_Capa_GetValue(m_opCam.hIdxTUCam, TUIDC_ROLLINGSCANSLIT, &nVal))
		{
			nVal = m_rsPara.nSlitHeight;
		}
		pProp->Set((long)(nVal));

		ret = DEVICE_OK;
	}
	break;
	default:
		break;
	}

	return ret;
}

/**
* Handles "OnRollinScanLITm" property.
*/
int CMMTUCam::OnRollinScanLITm(MM::PropertyBase* pProp, MM::ActionType eAct)
{
	if (NULL == m_opCam.hIdxTUCam)
		return DEVICE_NOT_CONNECTED;

	int ret = DEVICE_ERR;
	switch (eAct)
	{
	case MM::AfterSet:
	{
		ret = DEVICE_OK;
	}
	///break;
	case  MM::BeforeGet:
	{
		double dbVal = m_rsPara.dbLineInvalTm;
		char sz[256] = { 0 };
		sprintf(sz, "%0.2f us/row", dbVal);
		pProp->Set(sz);

		ret = DEVICE_OK;
	}
	break;
	default:
		break;
	}

	return ret;
}

/**
* Handles "OnRollingScanDir" property.
*/
int CMMTUCam::OnRollingScanDir(MM::PropertyBase* pProp, MM::ActionType eAct)
{
	if (NULL == m_opCam.hIdxTUCam)
		return DEVICE_NOT_CONNECTED;

	int ret = DEVICE_ERR;
	switch (eAct)
	{
	case MM::AfterSet:
	{
		//if (IsCapturing())
			//return DEVICE_CAMERA_BUSY_ACQUIRING;

		// the user just set the new value for the property, so we have to
		// apply this value to the 'hardware'.

		string val;
		pProp->Get(val);

		if (val.length() != 0)
		{
			TUCAM_CAPA_ATTR capaAttr;
			capaAttr.idCapa = TUIDC_ROLLINGSCANDIR;

			if (TUCAMRET_SUCCESS == TUCAM_Capa_GetAttr(m_opCam.hIdxTUCam, &capaAttr))
			{
				char szBuf[64] = { 0 };
				TUCAM_VALUE_TEXT valText;
				valText.nID = TUIDC_ROLLINGSCANDIR;
				valText.nTextSize = 64;
				valText.pText = &szBuf[0];

				int nCnt = capaAttr.nValMax - capaAttr.nValMin + 1;

				for (int i = 0; i < nCnt; i++)
				{
					valText.dbValue = i;
					TUCAM_Capa_GetValueText(m_opCam.hIdxTUCam, &valText);

					if (0 == val.compare(valText.pText))
					{
						TUCAM_Capa_SetValue(m_opCam.hIdxTUCam, TUIDC_ROLLINGSCANDIR, i);
						break;
					}
				}
			}
			OnPropertyChanged(g_PropNameRSDir, val.c_str());
		}
		ret = DEVICE_OK;
	}
	break;
	case MM::BeforeGet:
	{
		int nIdx = 0;
		TUCAM_Capa_GetValue(m_opCam.hIdxTUCam, TUIDC_ROLLINGSCANDIR, &nIdx);

		char szBuf[64] = { 0 };
		TUCAM_VALUE_TEXT valText;
		valText.nID = TUIDC_ROLLINGSCANDIR;
		valText.nTextSize = 64;
		valText.pText = &szBuf[0];

		valText.dbValue = nIdx;
		TUCAM_Capa_GetValueText(m_opCam.hIdxTUCam, &valText);

		pProp->Set(valText.pText);

		ret = DEVICE_OK;
	}
	break;
	default:
		break;
	}

	return ret;
}

/**
* Handles "OnRollingScanReset" property.
*/
int CMMTUCam::OnRollingScanReset(MM::PropertyBase* pProp, MM::ActionType eAct)
{
	if (NULL == m_opCam.hIdxTUCam)
		return DEVICE_NOT_CONNECTED;

	int ret = DEVICE_ERR;
	switch (eAct)
	{
	case MM::AfterSet:
	{
		//if (IsCapturing())
			//return DEVICE_CAMERA_BUSY_ACQUIRING;

		// the user just set the new value for the property, so we have to
		// apply this value to the 'hardware'.

		string val;
		pProp->Get(val);

		if (val.length() != 0)
		{
			int nIdx = 0;
			TUCAM_Capa_GetValue(m_opCam.hIdxTUCam, TUIDC_ROLLINGSCANDIR, &nIdx);

			TUCAM_CAPA_ATTR capaAttr;
			capaAttr.idCapa = TUIDC_ROLLINGSCANRESET;

			if (TUCTD_DOWNUPCYC == nIdx)
			{
				if (TUCAMRET_SUCCESS == TUCAM_Capa_GetAttr(m_opCam.hIdxTUCam, &capaAttr))
				{
					char szBuf[64] = { 0 };
					TUCAM_VALUE_TEXT valText;
					valText.nID = TUIDC_ROLLINGSCANRESET;
					valText.nTextSize = 64;
					valText.pText = &szBuf[0];

					int nCnt = capaAttr.nValMax - capaAttr.nValMin + 1;

					for (int i = 0; i < nCnt; i++)
					{
						valText.dbValue = i;
						TUCAM_Capa_GetValueText(m_opCam.hIdxTUCam, &valText);

						if (0 == val.compare(valText.pText))
						{
							TUCAM_Capa_SetValue(m_opCam.hIdxTUCam, TUIDC_ROLLINGSCANRESET, i);
							break;
						}
					}
				}
				OnPropertyChanged(g_PropNameRSReset, val.c_str());
			}
		}
		ret = DEVICE_OK;
	}
	break;
	case MM::BeforeGet:
	{
		int nIdx = 0;
		TUCAM_Capa_GetValue(m_opCam.hIdxTUCam, TUIDC_ROLLINGSCANRESET, &nIdx);

		char szBuf[64] = { 0 };
		TUCAM_VALUE_TEXT valText;
		valText.nID = TUIDC_ROLLINGSCANRESET;
		valText.nTextSize = 64;
		valText.pText = &szBuf[0];

		valText.dbValue = nIdx;
		TUCAM_Capa_GetValueText(m_opCam.hIdxTUCam, &valText);

		pProp->Set(valText.pText);

		ret = DEVICE_OK;
	}
	break;
	default:
		break;
	}

	return ret;
}

/**
* Handles "OnTestImageMode" property.
*/
int CMMTUCam::OnTestImageMode(MM::PropertyBase* pProp, MM::ActionType eAct)
{
	if (NULL == m_opCam.hIdxTUCam)
		return DEVICE_NOT_CONNECTED;

	int ret = DEVICE_ERR;
	switch (eAct)
	{
	case MM::AfterSet:
	{
		//if (IsCapturing())
		//return DEVICE_CAMERA_BUSY_ACQUIRING;

		// the user just set the new value for the property, so we have to
		// apply this value to the 'hardware'.
		string val;
		pProp->Get(val);

		if (val.length() != 0)
		{
			TUCAM_CAPA_ATTR capaAttr;
			capaAttr.idCapa = TUIDC_TESTIMGMODE;

			if (TUCAMRET_SUCCESS == TUCAM_Capa_GetAttr(m_opCam.hIdxTUCam, &capaAttr))
			{
				char szBuf[64] = { 0 };
				TUCAM_VALUE_TEXT valText;
				valText.nID = TUIDC_TESTIMGMODE;
				valText.nTextSize = 64;
				valText.pText = &szBuf[0];

				int nCnt = capaAttr.nValMax - capaAttr.nValMin + 1;

				for (int i = 0; i < nCnt; i++)
				{
					valText.dbValue = i;
					TUCAM_Capa_GetValueText(m_opCam.hIdxTUCam, &valText);

					if (0 == val.compare(valText.pText))
					{
						TUCAM_Capa_SetValue(m_opCam.hIdxTUCam, TUIDC_TESTIMGMODE, i);
						break;
					}
				}
			}

			OnPropertyChanged(g_PropNamePCLK, val.c_str());
			
		}
		ret = DEVICE_OK;
	}
	break;
	case MM::BeforeGet:
	{
		int nIdx = 0;
		TUCAM_Capa_GetValue(m_opCam.hIdxTUCam, TUIDC_TESTIMGMODE, &nIdx);

		char szBuf[64] = { 0 };
		TUCAM_VALUE_TEXT valText;
		valText.nID = TUIDC_TESTIMGMODE;
		valText.nTextSize = 64;
		valText.pText = &szBuf[0];

		valText.dbValue = nIdx;
		TUCAM_Capa_GetValueText(m_opCam.hIdxTUCam, &valText);

		pProp->Set(valText.pText);

		ret = DEVICE_OK;
	}
	break;
	default:
		break;
	}

	return ret;
}

/**
* Handles "OnGlobalGainMode" property.
*/
int  CMMTUCam::OnGlobalGainMode(MM::PropertyBase* pProp, MM::ActionType eAct)
{
    if (NULL == m_opCam.hIdxTUCam)
        return DEVICE_NOT_CONNECTED;

    int ret = DEVICE_ERR;
    switch (eAct)
    {
    case MM::AfterSet:
    {
        if (IsCapturing())
            return DEVICE_CAMERA_BUSY_ACQUIRING;

        // the user just set the new value for the property, so we have to
        // apply this value to the 'hardware'.

        string val;
        pProp->Get(val);

        if (val.length() != 0)
        {
            TUCAM_PROP_ATTR propAttr;
            propAttr.nIdxChn = 0;
            propAttr.idProp = TUIDP_GLOBALGAIN;

            if (TUCAMRET_SUCCESS == TUCAM_Prop_GetAttr(m_opCam.hIdxTUCam, &propAttr))
            {
                char szBuf[64] = { 0 };
                TUCAM_VALUE_TEXT valText;
                valText.nID = TUIDP_GLOBALGAIN;
                valText.nTextSize = 64;
                valText.pText = &szBuf[0];

                int nCnt = (int)(propAttr.dbValMax - propAttr.dbValMin + 1);

                for (int i = 0; i<nCnt; i++)
                {
                    valText.dbValue = i;
                    TUCAM_Prop_GetValueText(m_opCam.hIdxTUCam, &valText);

                    if (0 == val.compare(valText.pText))
                    {
                        TUCAM_Prop_SetValue(m_opCam.hIdxTUCam, TUIDP_GLOBALGAIN, i);
                        break;
                    }
                }

				if (IsSupportAries16())
				{
					UpdateLevelsRange();
				}

                OnPropertyChanged(g_PropNameGain, val.c_str());
            }                       

            ret = DEVICE_OK;
        }
    }
    break;
    case MM::BeforeGet:
    {
        double gain = 0;
        TUCAM_Prop_GetValue(m_opCam.hIdxTUCam, TUIDP_GLOBALGAIN, &gain);

        char szBuf[64] = { 0 };
        TUCAM_VALUE_TEXT valText;
        valText.nID = TUIDP_GLOBALGAIN;
        valText.nTextSize = 64;
        valText.pText = &szBuf[0];

        valText.dbValue = gain;
        TUCAM_Prop_GetValueText(m_opCam.hIdxTUCam, &valText);

        pProp->Set(valText.pText);

        ret = DEVICE_OK;
    }
    break;
    default:
        break;
    }

    return ret;
}

/**
* Handles "OnGAINMode" property.
*/
int CMMTUCam::OnGAINMode(MM::PropertyBase* pProp, MM::ActionType eAct)
{
    if (NULL == m_opCam.hIdxTUCam)
        return DEVICE_NOT_CONNECTED;

    int ret = DEVICE_ERR;
    switch(eAct)
    {
    case MM::AfterSet:
        {
            if (PID_FL_9BW == m_nPID || PID_FL_9BW_LT == m_nPID)
            {
                string val;
                pProp->Get(val);

                if (val.length() != 0)
                {
                    TUCAM_CAPA_ATTR capaAttr;
                    capaAttr.idCapa = TUIDC_IMGMODESELECT;

                    if (TUCAMRET_SUCCESS == TUCAM_Capa_GetAttr(m_opCam.hIdxTUCam, &capaAttr))
                    {
                        char szBuf[64] = { 0 };
                        TUCAM_VALUE_TEXT valText;
                        valText.nID = TUIDC_IMGMODESELECT;
                        valText.nTextSize = 64;
                        valText.pText = &szBuf[0];

                        int nCnt = capaAttr.nValMax - capaAttr.nValMin + 1;

                        for (int i = 0; i < nCnt; i++)
                        {
                            valText.dbValue = i;
                            TUCAM_Capa_GetValueText(m_opCam.hIdxTUCam, &valText);

                            if (0 == val.compare(valText.pText))
                            {
                                TUCAM_Capa_SetValue(m_opCam.hIdxTUCam, TUIDC_IMGMODESELECT, i);
                                break;
                            }
                        }
                    }

                    OnPropertyChanged(g_PropNameMode, val.c_str());

                    double val = 0;
                    TUCAM_Prop_GetValue(m_opCam.hIdxTUCam, TUIDP_GLOBALGAIN, &val);
                    SetProperty(g_PropNameGain, CDeviceUtils::ConvertToString((int)val));

                    UpdateExpRange();

                    ret = DEVICE_OK;
                }
            }
            else
            {
                int nVal = 0;
                int nImgMode = 0;
                int nGain = 0;
                double dblExp = 0.0;
                string val;
                pProp->Get(val);
                if (val.length() != 0)
                {
                    if (TUCAMRET_SUCCESS == TUCAM_Capa_GetValue(m_opCam.hIdxTUCam, TUIDC_IMGMODESELECT, &nVal))
                    {
                        if (DHYANA_D95_V2 == m_nPID)
                        {
                            bool bLiving = m_bLiving;
                            if (0 == val.compare(g_HDRBIT_ON))
                            {
                                nImgMode = 0;  //HDR
                                nGain = 0;
                            }
                            else if (0 == val.compare(g_HIGHBIT_ON))
                            {
                                nImgMode = 0; //HIGH
                                nGain = 1;
                            }
                            else if (0 == val.compare(g_LOWBIT_ON))
                            {
                                nImgMode = 0; //LOW
                                nGain = 2;
                            }
                            else if (0 == val.compare(g_STDHIGH_ON))
                            {
                                nImgMode = 1; //STDH
                            }
                            else if (0 == val.compare(g_STDLOW_ON))
                            {
                                nImgMode = 2; //STDL
                            }

						if (nImgMode != nVal)
						{
							if (bLiving){ StopCapture(); }
							TUCAM_Capa_SetValue(m_opCam.hIdxTUCam, TUIDC_IMGMODESELECT, nImgMode);  
							if (bLiving){ StartCapture(); }
						}
						TUCAM_Prop_SetValue(m_opCam.hIdxTUCam, TUIDP_GLOBALGAIN, nGain);
						
					}
					else
					{
						TUCAM_Prop_GetValue(m_opCam.hIdxTUCam, TUIDP_EXPOSURETM, &dblExp);

						if (0 == val.compare(g_CMSBIT_ON)){
							nImgMode = 1;  //CMS
							nGain    = 0;
						}
						else if(0 == val.compare(g_HDRBIT_ON)){
							nImgMode = 2;  //HDR
							nGain    = 0;
						}
						else if(0 == val.compare(g_HIGHBIT_ON)){
							nImgMode = 2;  //High Gain
							nGain    = 1;
						}
						else if (0 == val.compare(g_LOWBIT_ON)){
							nImgMode = 2;  //High Gain
							nGain    = 2;
						}
						else if(0 == val.compare(g_GRHIGH_ON)){
							nImgMode = 3;  //Global Reset High Gain
							nGain    = 1;
						}
						else if(0 == val.compare(g_GRLOW_ON)){
							nImgMode = 3;  //Global Reset Low Gain
							nGain    = 2;
						}
						else if (0 == val.compare(g_HSHIGH_ON)){
							nImgMode = 3;  //HighSpeed High Gain
							nGain    = 1;
						}
						else if (0 == val.compare(g_HSLOW_ON)){
							nImgMode = 4;  //HighSpeed Low Gain
							nGain    = 2;
						}

						if (nImgMode != nVal)
						{
							TUCAM_Capa_SetValue(m_opCam.hIdxTUCam, TUIDC_IMGMODESELECT, nImgMode);
						}
					    TUCAM_Prop_SetValue(m_opCam.hIdxTUCam, TUIDP_GLOBALGAIN, nGain);

						TUCAM_Prop_SetValue(m_opCam.hIdxTUCam, TUIDP_EXPOSURETM, dblExp);

						UpdateExpRange();

                            if ((DHYANA_400BSIV2 == m_nPID && m_nBCD > 0x04))
                            {
                                vector<string>ModTgrValues;
                                ModTgrValues.push_back(g_TRIGGER_OFF);
                                if (0 == val.compare(g_GRLOW_ON) || 0 == val.compare(g_GRHIGH_ON))
                                {
                                    ModTgrValues.push_back(g_TRIGGER_STD);
                                }
                                else
                                {
                                    ModTgrValues.push_back(g_TRIGGER_STD);
                                    ModTgrValues.push_back(g_TRIGGER_SYN);
                                }
                                ModTgrValues.push_back(g_TRIGGER_SWF);
                                ClearAllowedValues(g_PropNameMdTgr);
                                SetAllowedValues(g_PropNameMdTgr, ModTgrValues);
                            }
                        }
                    }

                    ret = DEVICE_OK;
                }
            }
        }
        break;
    case MM::BeforeGet:
        {
            int nVal = 0;
			double dVal = 0;
            TUCAM_Capa_GetValue(m_opCam.hIdxTUCam, TUIDC_IMGMODESELECT, &nVal);
			TUCAM_Prop_GetValue(m_opCam.hIdxTUCam, TUIDP_GLOBALGAIN, &dVal);

            string val;
            pProp->Get(val);

			if (DHYANA_D95_V2 == m_nPID)
			{
				if (1 == nVal)
					pProp->Set(g_STDHIGH_ON);
				else if (2 == nVal)
					pProp->Set(g_STDLOW_ON);
				else {
					if (1 == dVal)
						pProp->Set(g_HIGHBIT_ON);
					else if (2 == dVal)
						pProp->Set(g_LOWBIT_ON);
					else
						pProp->Set(g_HDRBIT_ON);
				}
			}
            else if (PID_FL_9BW == m_nPID || PID_FL_9BW_LT == m_nPID)
            {
                int nIdx = 0;
                TUCAM_Capa_GetValue(m_opCam.hIdxTUCam, TUIDC_IMGMODESELECT, &nIdx);

                char szBuf[64] = { 0 };
                TUCAM_VALUE_TEXT valText;
                valText.nID = TUIDC_IMGMODESELECT;
                valText.nTextSize = 64;
                valText.pText = &szBuf[0];

                valText.dbValue = nIdx;
                TUCAM_Capa_GetValueText(m_opCam.hIdxTUCam, &valText);

                pProp->Set(valText.pText);
            }
			else
			{
				if (1 == nVal)
					pProp->Set(g_CMSBIT_ON);
				else if (3 == nVal){
					if (1 == dVal)
						pProp->Set(g_GRHIGH_ON);
					else
						pProp->Set(g_GRLOW_ON);
				}
				else{
					if (1 == dVal)
						pProp->Set(g_HIGHBIT_ON);
					else if (2 == dVal)
						pProp->Set(g_LOWBIT_ON);
					else
						pProp->Set(g_HDRBIT_ON);
				}
			}
			
            ret = DEVICE_OK;
        }
        break;
    default:
        break;
    }

    return ret;
}

/**
* Handles "OnShutterMode" property.
*/
int CMMTUCam::OnShutterMode(MM::PropertyBase* pProp, MM::ActionType eAct)
{
    if (NULL == m_opCam.hIdxTUCam)
        return DEVICE_NOT_CONNECTED;

    int ret = DEVICE_ERR;
    switch (eAct)
    {
    case MM::AfterSet:
    {
        string val;
        pProp->Get(val);

        if (val.length() != 0)
        {
            TUCAM_CAPA_ATTR capaAttr;
            capaAttr.idCapa = TUIDC_SHUTTER;

            if (TUCAMRET_SUCCESS == TUCAM_Capa_GetAttr(m_opCam.hIdxTUCam, &capaAttr))
            {
                char szBuf[64] = { 0 };
                TUCAM_VALUE_TEXT valText;
                valText.nID = TUIDC_SHUTTER;
                valText.nTextSize = 64;
                valText.pText = &szBuf[0];

                int nCnt = (int)(capaAttr.nValMax - capaAttr.nValMin + 1);

                for (int i = 0; i<nCnt; i++)
                {
                    valText.dbValue = i;
                    TUCAM_Capa_GetValueText(m_opCam.hIdxTUCam, &valText);

                    if (0 == val.compare(valText.pText))
                    {
                        TUCAM_Capa_SetValue(m_opCam.hIdxTUCam, TUIDC_SHUTTER, i);
                        break;
                    }
                }
            }

            OnPropertyChanged(g_PropNameShutter, val.c_str());

            ret = DEVICE_OK;
        }
    }
    break;
    case  MM::BeforeGet:
    {
        int val = 0;
        TUCAM_Capa_GetValue(m_opCam.hIdxTUCam, TUIDC_SHUTTER, &val);

        char szBuf[64] = { 0 };
        TUCAM_VALUE_TEXT valText;
        valText.nID = TUIDC_SHUTTER;
        valText.nTextSize = 64;
        valText.pText = &szBuf[0];

        valText.dbValue = val;
        TUCAM_Capa_GetValueText(m_opCam.hIdxTUCam, &valText);

        pProp->Set(valText.pText);

        ret = DEVICE_OK;
    }
    break;
    default:
        break;
    }

    return ret;
}

/**
* Handles "OnModeSelect" property.
*/
int CMMTUCam::OnModeSelect(MM::PropertyBase* pProp, MM::ActionType eAct)
{
	if (NULL == m_opCam.hIdxTUCam)
		return DEVICE_NOT_CONNECTED;

	int ret = DEVICE_ERR;
	switch (eAct)
	{
	case MM::AfterSet:
	{
		int nVal = 0;
		int nImgMode = 0;
		int nGain = 0;
		double dblExp = 0.0;
		string val;
		pProp->Get(val);
		if (val.length() != 0)
		{
			if (TUCAMRET_SUCCESS == TUCAM_Capa_GetValue(m_opCam.hIdxTUCam, TUIDC_IMGMODESELECT, &nVal))
			{
				TUCAM_Prop_GetValue(m_opCam.hIdxTUCam, TUIDP_EXPOSURETM, &dblExp);
				m_rsPara.nSlitHeightMin = 1;
				m_rsPara.nSlitHeightStep = 1;
				if (0 == val.compare(g_HIGHDYNAMIC_ON)){
					nImgMode = 2;  //HDR
					nGain    = 0;
				}
				else if (0 == val.compare(g_HIGHSPEED_ON)){
					nImgMode = 3;  //HighSpeedHg
					nGain    = 1;
					m_rsPara.nSlitHeightMin = 2;
					m_rsPara.nSlitHeightStep = 2;
				}
				else if (0 == val.compare(g_HIGHSENSITY_ON)){
					nImgMode = 1;  //CMS
					nGain    = 0;
				}
				else if (0 == val.compare(g_GLOBALRESET_ON)){
					nImgMode = 5;  //GlobalReset Hg
					nGain    = 1;
					m_rsPara.nMode = 0x00;
					TUCAM_Capa_SetValue(m_opCam.hIdxTUCam, TUIDC_ROLLINGSCANMODE, m_rsPara.nMode);
				}

				if (nImgMode != nVal)
				{
					TUCAM_Capa_SetValue(m_opCam.hIdxTUCam, TUIDC_IMGMODESELECT, nImgMode);
				}
				TUCAM_Prop_SetValue(m_opCam.hIdxTUCam, TUIDP_GLOBALGAIN, nGain);
				TUCAM_Prop_SetValue(m_opCam.hIdxTUCam, TUIDP_EXPOSURETM, dblExp);

				UpdateExpRange();

				TUCAM_CAPA_ATTR  capaAttr;
				capaAttr.idCapa = TUIDC_ROLLINGSCANLTD;
				TUCAM_Capa_GetAttr(m_opCam.hIdxTUCam, &capaAttr);

				m_rsPara.nLTDelayMax = capaAttr.nValMax;
				SetPropertyLimits(g_PropNameRSLtd, m_rsPara.nLTDelayMin, m_rsPara.nLTDelayMax);
				SetPropertyLimits(g_PropNameRSSlit, m_rsPara.nSlitHeightMin, m_rsPara.nSlitHeightMax);
				UpdateSlitHeightRange();
				m_rsPara.dbLineInvalTm = LineIntervalTime(0 == m_rsPara.nMode ? 0 : m_rsPara.nLTDelay);

				vector<string>ModTgrValues;
				ModTgrValues.push_back(g_TRIGGER_OFF);
				if (0 == val.compare(g_GLOBALRESET_ON))
				{
					ModTgrValues.push_back(g_TRIGGER_STD);
				}
				else
				{
					ModTgrValues.push_back(g_TRIGGER_STD);
					ModTgrValues.push_back(g_TRIGGER_SYN);
				}
				ModTgrValues.push_back(g_TRIGGER_SWF);
				ClearAllowedValues(g_PropNameMdTgr);
				SetAllowedValues(g_PropNameMdTgr, ModTgrValues);
			}

			ret = DEVICE_OK;
		}
	}
	break;
	case MM::BeforeGet:
	{
		int nVal = 0;
		TUCAM_Capa_GetValue(m_opCam.hIdxTUCam, TUIDC_IMGMODESELECT, &nVal);
		m_rsPara.nSlitHeightMin = 1;
		m_rsPara.nSlitHeightStep = 1;
		if (1 == nVal)      // CMS
			pProp->Set(g_HIGHSENSITY_ON);
		else if (2 == nVal) // HDR
			pProp->Set(g_HIGHDYNAMIC_ON);
		else if (3 == nVal) // HighSpeed HG
		{
			pProp->Set(g_HIGHSPEED_ON);
			m_rsPara.nSlitHeightMin = 2;
			m_rsPara.nSlitHeightStep = 2;
		}
		else if (5 == nVal){ // GlobaelReset HG
			pProp->Set(g_GLOBALRESET_ON);
			m_rsPara.nMode = 0x00;
		}
		else{
			pProp->Set(g_HIGHDYNAMIC_ON);
		}

		ret = DEVICE_OK;
	}
	break;
	default:
		break;
	}

	return ret;
}

/**
* Handles "ImageMode" property.
*/
int CMMTUCam::OnImageMode(MM::PropertyBase* pProp, MM::ActionType eAct)
{
    if (NULL == m_opCam.hIdxTUCam)
        return DEVICE_NOT_CONNECTED;

    int ret = DEVICE_ERR;
    switch(eAct)
    {
    case MM::AfterSet:
        {
            string val;
            pProp->Get(val);            

            if (val.length() != 0)
            {
                TUCAM_PROP_ATTR propAttr;
                propAttr.nIdxChn= 0;
                propAttr.idProp = TUIDP_GLOBALGAIN;

                if (TUCAMRET_SUCCESS == TUCAM_Prop_GetAttr(m_opCam.hIdxTUCam, &propAttr))
                {
                    char szBuf[64] = {0};
                    TUCAM_VALUE_TEXT valText;
                    valText.nID       = TUIDP_GLOBALGAIN;
                    valText.nTextSize = 64;
                    valText.pText     = &szBuf[0];

                    int nCnt = 2/*(int)propAttr.dbValMax*/ - (int)propAttr.dbValMin + 1;

                    for (int i=0; i<nCnt; i++)
                    {
                        valText.dbValue = i;
                        TUCAM_Prop_GetValueText(m_opCam.hIdxTUCam, &valText);   

                        if (0 == val.compare(valText.pText))
                        {
                            TUCAM_Prop_SetValue(m_opCam.hIdxTUCam, TUIDP_GLOBALGAIN, i);
                            m_nIdxGain = i;

                            int nVal = 0;
                            if (TUCAMRET_SUCCESS == TUCAM_Capa_GetValue(m_opCam.hIdxTUCam, TUIDC_IMGMODESELECT, &nVal))
                            {
								TUCAM_CAPA_ATTR  capaAttr;
								capaAttr.idCapa = TUIDC_IMGMODESELECT;
								TUCAM_Capa_GetAttr(m_opCam.hIdxTUCam, &capaAttr);
                                nCnt = capaAttr.nValMax - capaAttr.nValMin;
                                if (0 != nVal && nCnt < 2)
                                {
                                    TUCAM_Capa_SetValue(m_opCam.hIdxTUCam,TUIDC_IMGMODESELECT, 0);
                                }                                
                            }

                            break;
                        }                         
                    }
                }

				OnPropertyChanged(g_PropNameGain, val.c_str());

                ret = DEVICE_OK;
            }
        }
        break;
    case MM::BeforeGet:
        {
            double dblVal = 0;
            TUCAM_Prop_GetValue(m_opCam.hIdxTUCam, TUIDP_GLOBALGAIN, &dblVal);

            char szBuf[64] = {0};
            TUCAM_VALUE_TEXT valText;
            valText.nID       = TUIDP_GLOBALGAIN;
            valText.nTextSize = 64;
            valText.pText     = &szBuf[0];

            valText.dbValue = dblVal;
            TUCAM_Prop_GetValueText(m_opCam.hIdxTUCam, &valText); 

            m_nIdxGain = (int)dblVal;

            pProp->Set(valText.pText);

            ret = DEVICE_OK;
        }
        break;
    default:
        break;
    }

    return ret; 
}

/**
* Handles "PixelType" property.
*/
int CMMTUCam::OnPixelType(MM::PropertyBase* pProp, MM::ActionType eAct)
{      
    int ret = DEVICE_ERR;
    switch(eAct)
    {
    case MM::AfterSet:
        {
            if(IsCapturing())
                return DEVICE_CAMERA_BUSY_ACQUIRING;

            string pixelType;
            pProp->Get(pixelType);

            if (pixelType.compare(g_PixelType_8bit) == 0)
            {
                nComponents_ = 1;
                img_.Resize(img_.Width(), img_.Height(), 1);
                bitDepth_ = 8;
                ret=DEVICE_OK;
            }
            else if (pixelType.compare(g_PixelType_16bit) == 0)
            {
                nComponents_ = 1;
                img_.Resize(img_.Width(), img_.Height(), 2);
                bitDepth_ = 16;
                ret=DEVICE_OK;
            }
            else if ( pixelType.compare(g_PixelType_32bitRGB) == 0)
            {
                nComponents_ = 4;
                img_.Resize(img_.Width(), img_.Height(), 4);
                bitDepth_ = 8;
                ret=DEVICE_OK;
            }
            else if ( pixelType.compare(g_PixelType_64bitRGB) == 0)
            {
                nComponents_ = 4;
                img_.Resize(img_.Width(), img_.Height(), 8);
                bitDepth_ = 16;
                ret=DEVICE_OK;
            }
            else if ( pixelType.compare(g_PixelType_32bit) == 0)
            {
                nComponents_ = 1;
                img_.Resize(img_.Width(), img_.Height(), 4);
                bitDepth_ = 32;
                ret=DEVICE_OK;
            }
            else
            {
                // on error switch to default pixel type
                nComponents_ = 1;
                img_.Resize(img_.Width(), img_.Height(), 1);
                pProp->Set(g_PixelType_8bit);
                bitDepth_ = 8;
                ret = ERR_UNKNOWN_MODE;
            }
        }
        break;
    case MM::BeforeGet:
        {
            long bytesPerPixel = GetImageBytesPerPixel();
            if (bytesPerPixel == 1)
            {
                pProp->Set(g_PixelType_8bit);
            }
            else if (bytesPerPixel == 2)
            {
                pProp->Set(g_PixelType_16bit);
            }
            else if (bytesPerPixel == 4)
            {              
                if (nComponents_ == 4)
                {
                    pProp->Set(g_PixelType_32bitRGB);
                }
                else if (nComponents_ == 1)
                {
                    pProp->Set(::g_PixelType_32bit);
                }
            }
            else if (bytesPerPixel == 8)
            {
                pProp->Set(g_PixelType_64bitRGB);
            }
            else
            {
                pProp->Set(g_PixelType_8bit);
            }
            ret = DEVICE_OK;
        } break;
    default:
        break;
    }

    return ret; 
}

/**
* Handles "BitDepth" property.
*/
int CMMTUCam::OnBitDepth(MM::PropertyBase* pProp, MM::ActionType eAct)
{
    if (NULL == m_opCam.hIdxTUCam)
        return DEVICE_NOT_CONNECTED;

    if (PID_FL_9BW == m_nPID || PID_FL_9BW_LT == m_nPID || PID_FL_26BW == m_nPID)
        return DEVICE_OK;

    int ret = DEVICE_ERR;
    switch(eAct)
    {
    case MM::AfterSet:
        {
            if(IsCapturing())
                return DEVICE_CAMERA_BUSY_ACQUIRING;

            // the user just set the new value for the property, so we have to
            // apply this value to the 'hardware'.

            string val;
            pProp->Get(val);
            if (val.length() != 0)
            {
                TUCAM_CAPA_ATTR capaAttr;
                capaAttr.idCapa = TUIDC_BITOFDEPTH;

                if (TUCAMRET_SUCCESS == TUCAM_Capa_GetAttr(m_opCam.hIdxTUCam, &capaAttr))
                {
                    m_bLiving = false;
                    TUCAM_Cap_Stop(m_opCam.hIdxTUCam);      // Stop capture   
                    ReleaseBuffer();

                    if (0 == val.compare("16"))
                    {
                        TUCAM_Capa_SetValue(m_opCam.hIdxTUCam, TUIDC_BITOFDEPTH, 16);
                        SetPropertyLimits(g_PropNameLLev, 0, 65534);
                        SetPropertyLimits(g_PropNameRLev, 1, 65535);
						SetProperty(MM::g_Keyword_PixelType, g_PixelType_16bit);
                    }
                    else
                    {
                        TUCAM_Capa_SetValue(m_opCam.hIdxTUCam, TUIDC_BITOFDEPTH, 8);
                        SetPropertyLimits(g_PropNameLLev, 0, 254);
                        SetPropertyLimits(g_PropNameRLev, 1, 255);
						SetProperty(MM::g_Keyword_PixelType, g_PixelType_8bit);
                    }

                    if (m_nPID == PID_FL_9BW || PID_FL_9BW_LT == m_nPID || m_nPID == PID_FL_20BW || m_nPID == PID_FL_26BW)
					{
						UpdateExpRange();
					}

//                  StartCapture();
                    ResizeImageBuffer();

                    roiX_ = 0;
                    roiY_ = 0;
                }

                OnPropertyChanged(g_PropNameBODP, val.c_str());

                ret = DEVICE_OK;
            }
        }
        break;
    case MM::BeforeGet:
        {
            int nVal = 0;
            TUCAM_Capa_GetValue(m_opCam.hIdxTUCam, TUIDC_BITOFDEPTH, &nVal);

            if (16 == nVal)
            {
                pProp->Set("16");
            }
            else
            {
                pProp->Set("8");
            }

            ret = DEVICE_OK;
        }
        break;
    default:
        break;
    }

    return ret;
}

/**
* Handles "BitDepth" property.
*/
int CMMTUCam::OnBitDepthEum(MM::PropertyBase* pProp, MM::ActionType eAct)
{
	if (NULL == m_opCam.hIdxTUCam)
		return DEVICE_NOT_CONNECTED;

	int ret = DEVICE_ERR;
	switch (eAct)
	{
	case MM::AfterSet:
	{
		if (IsCapturing())
			return DEVICE_CAMERA_BUSY_ACQUIRING;

		// the user just set the new value for the property, so we have to
		// apply this value to the 'hardware'.
		string val;
		pProp->Get(val);
		if (val.length() != 0)
		{
			TUCAM_CAPA_ATTR capaAttr;
			capaAttr.idCapa = TUIDC_BITOFDEPTH;

			if (TUCAMRET_SUCCESS == TUCAM_Capa_GetAttr(m_opCam.hIdxTUCam, &capaAttr))
			{
				m_bLiving = false;
				TUCAM_Cap_Stop(m_opCam.hIdxTUCam);      // Stop capture   
				ReleaseBuffer();

				char szBuf[64] = { 0 };
				TUCAM_VALUE_TEXT valText;
				valText.nID = TUIDC_BITOFDEPTH;
				valText.nTextSize = 64;
				valText.pText = &szBuf[0];

				int i = 0;
				int nCnt = capaAttr.nValMax - capaAttr.nValMin + 1;

				for (; i<nCnt; i++)
				{
					valText.dbValue = i;
					TUCAM_Capa_GetValueText(m_opCam.hIdxTUCam, &valText);

					if (0 == val.compare(valText.pText))
					{
						TUCAM_Capa_SetValue(m_opCam.hIdxTUCam, TUIDC_BITOFDEPTH, i);
						break;
					}
				}

				UpdateLevelsRange();
				//                  StartCapture();
				ResizeImageBuffer();

				roiX_ = 0;
				roiY_ = 0;
			}

			OnPropertyChanged(g_PropNameBODP, val.c_str());

			ret = DEVICE_OK;
		}
	}
	break;
	case MM::BeforeGet:
	{
		int nIdx = 0;
		TUCAM_Capa_GetValue(m_opCam.hIdxTUCam, TUIDC_BITOFDEPTH, &nIdx);

		char szBuf[64] = { 0 };
		TUCAM_VALUE_TEXT valText;
		valText.nID = TUIDC_BITOFDEPTH;
		valText.nTextSize = 64;
		valText.pText = &szBuf[0];

		valText.dbValue = nIdx;
		TUCAM_Capa_GetValueText(m_opCam.hIdxTUCam, &valText);
		pProp->Set(valText.pText);

            ret = DEVICE_OK;
        }
        break;
    default:
        break;
    }

    return ret;
}

/**
* Handles "FlipHorizontal" property.
*/
int CMMTUCam::OnFlipH(MM::PropertyBase* pProp, MM::ActionType eAct)
{
    if (NULL == m_opCam.hIdxTUCam)
        return DEVICE_NOT_CONNECTED;

    int ret = DEVICE_ERR;
    switch(eAct)
    {
    case MM::AfterSet:
        {
            string val;
            pProp->Get(val);
            if (val.length() != 0)
            {
                TUCAM_CAPA_ATTR capaAttr;
                capaAttr.idCapa = TUIDC_HORIZONTAL;

                if (TUCAMRET_SUCCESS == TUCAM_Capa_GetAttr(m_opCam.hIdxTUCam, &capaAttr))
                {
                    if (0 == val.compare("TRUE"))
                    {
                        TUCAM_Capa_SetValue(m_opCam.hIdxTUCam, TUIDC_HORIZONTAL, 1);
                    }
                    else
                    {
                        TUCAM_Capa_SetValue(m_opCam.hIdxTUCam, TUIDC_HORIZONTAL, 0);
                    }
                }

                OnPropertyChanged(g_PropNameFLPH, val.c_str());

                ret = DEVICE_OK;
            }
        }
        break;
    case MM::BeforeGet:
        {
            int nVal = 0;
            TUCAM_Capa_GetValue(m_opCam.hIdxTUCam, TUIDC_HORIZONTAL, &nVal);

            if (1 == nVal)
            {
                pProp->Set("TRUE");
            }
            else
            {
                pProp->Set("FALSE");
            }

            ret = DEVICE_OK;
        }
        break;
    default:
        break;
    }

    return ret;
}

/**
* Handles "FlipVertical" property.
*/
int CMMTUCam::OnFlipV(MM::PropertyBase* pProp, MM::ActionType eAct)
{
    if (NULL == m_opCam.hIdxTUCam)
        return DEVICE_NOT_CONNECTED;

    int ret = DEVICE_ERR;
    switch(eAct)
    {
    case MM::AfterSet:
        {
            string val;
            pProp->Get(val);
            if (val.length() != 0)
            {
                TUCAM_CAPA_ATTR capaAttr;
                capaAttr.idCapa = TUIDC_VERTICAL;

                if (TUCAMRET_SUCCESS == TUCAM_Capa_GetAttr(m_opCam.hIdxTUCam, &capaAttr))
                {
                    if (0 == val.compare("TRUE"))
                    {
                        TUCAM_Capa_SetValue(m_opCam.hIdxTUCam, TUIDC_VERTICAL, 1);
                    }
                    else
                    {
                        TUCAM_Capa_SetValue(m_opCam.hIdxTUCam, TUIDC_VERTICAL, 0);
                    }
                }

                OnPropertyChanged(g_PropNameFLPV, val.c_str());

                ret = DEVICE_OK;
            }
        }
        break;
    case MM::BeforeGet:
        {
            int nVal = 0;
            TUCAM_Capa_GetValue(m_opCam.hIdxTUCam, TUIDC_VERTICAL, &nVal);

            if (1 == nVal)
            {
                pProp->Set("TRUE");
            }
            else
            {
                pProp->Set("FALSE");
            }

            ret = DEVICE_OK;
        }
        break;
    default:
        break;
    }

    return ret;
}

/**
* Handles "Gamma" property.
*/
int CMMTUCam::OnGamma(MM::PropertyBase* pProp, MM::ActionType eAct)
{
    if (NULL == m_opCam.hIdxTUCam)
        return DEVICE_NOT_CONNECTED;

    int ret = DEVICE_ERR;
    switch(eAct)
    {
    case MM::AfterSet:
        {
            long lVal = 0;
            pProp->Get(lVal);

            TUCAM_Prop_SetValue(m_opCam.hIdxTUCam, TUIDP_GAMMA, lVal);

            ret = DEVICE_OK;
        }
        break;
    case  MM::BeforeGet:
        {
            double dblVal = 0.0f;
            TUCAM_Prop_GetValue(m_opCam.hIdxTUCam, TUIDP_GAMMA, &dblVal);

            pProp->Set((long)(dblVal));

            ret = DEVICE_OK;
        }
        break;
    default:
        break;
    }

    return ret;
}

/**
* Handles "Contrast" property.
*/
int CMMTUCam::OnContrast(MM::PropertyBase* pProp, MM::ActionType eAct)
{
    if (NULL == m_opCam.hIdxTUCam)
        return DEVICE_NOT_CONNECTED;

    int ret = DEVICE_ERR;
    switch(eAct)
    {
    case MM::AfterSet:
        {
            long lVal = 0;
            pProp->Get(lVal);

            TUCAM_Prop_SetValue(m_opCam.hIdxTUCam, TUIDP_CONTRAST, lVal);

            ret = DEVICE_OK;
        }
        break;
    case  MM::BeforeGet:
        {
            double dblVal = 0.0f;
            TUCAM_Prop_GetValue(m_opCam.hIdxTUCam, TUIDP_CONTRAST, &dblVal);

            pProp->Set((long)(dblVal));

            ret = DEVICE_OK;
        }
        break;
    default:
        break;
    }

    return ret;
}

/**
* Handles "Saturation" property.
*/
int CMMTUCam::OnSaturation(MM::PropertyBase* pProp, MM::ActionType eAct)
{
    if (NULL == m_opCam.hIdxTUCam)
        return DEVICE_NOT_CONNECTED;

    int ret = DEVICE_ERR;
    switch(eAct)
    {
    case MM::AfterSet:
        {
            long lVal = 0;
            pProp->Get(lVal);

            TUCAM_Prop_SetValue(m_opCam.hIdxTUCam, TUIDP_SATURATION, lVal);

            ret = DEVICE_OK;
        }
        break;
    case  MM::BeforeGet:
        {
            double dblVal = 0.0f;
            TUCAM_Prop_GetValue(m_opCam.hIdxTUCam, TUIDP_SATURATION, &dblVal);

            pProp->Set((long)(dblVal));

            ret = DEVICE_OK;
        }
        break;
    default:
        break;
    }

    return ret;
}

/**
* Handles "WhiteBalance" property.
*/
int CMMTUCam::OnWhiteBalance(MM::PropertyBase* pProp, MM::ActionType eAct)
{
    if (NULL == m_opCam.hIdxTUCam)
        return DEVICE_NOT_CONNECTED;

    int ret = DEVICE_ERR;
    switch(eAct)
    {
    case MM::AfterSet:
        {
            string val;
            pProp->Get(val);
            if (val.length() != 0)
            {
                TUCAM_CAPA_ATTR capaAttr;
                capaAttr.idCapa = TUIDC_ATWBALANCE;

                if (TUCAMRET_SUCCESS == TUCAM_Capa_GetAttr(m_opCam.hIdxTUCam, &capaAttr))
                {
                    if (0 == val.compare("Click"))
                    {
                        TUCAM_Capa_SetValue(m_opCam.hIdxTUCam, TUIDC_ATWBALANCE, 1);
                    }
                    else
                    {
                        if (0 == val.compare("TRUE"))
                        {
                            TUCAM_Capa_SetValue(m_opCam.hIdxTUCam, TUIDC_ATWBALANCE, 2);
                        }
                        else
                        {
                            TUCAM_Capa_SetValue(m_opCam.hIdxTUCam, TUIDC_ATWBALANCE, 0);
                        }
                    }
                }

                OnPropertyChanged(g_PropNameATWB, val.c_str());

                ret = DEVICE_OK;
            }
        }
        break;
    case MM::BeforeGet:
        {
            int nVal = 0;
            TUCAM_Capa_GetValue(m_opCam.hIdxTUCam, TUIDC_ATWBALANCE, &nVal);

            string val;
            pProp->Get(val);

            if (0 == val.compare("Click"))
            {

            }
            else
            {
                if (2 == nVal)
                {
                    pProp->Set("TRUE");
                }
                else
                {
                    pProp->Set("FALSE");
                }
            }

            ret = DEVICE_OK;
        }
        break;
    default:
        break;
    }

    return ret;
}

/**
* Handles "Color Temperature" property.
*/
int CMMTUCam::OnClrTemp(MM::PropertyBase* pProp, MM::ActionType eAct)
{
	if (NULL == m_opCam.hIdxTUCam)
		return DEVICE_NOT_CONNECTED;

	int ret = DEVICE_ERR;
	switch (eAct)
	{
	case MM::AfterSet:
	{
		string val;
		pProp->Get(val);
		if (val.length() != 0)
		{
			TUCAM_PROP_ATTR propAttr;
			propAttr.nIdxChn = 0;
			propAttr.idProp = TUIDP_CLRTEMPERATURE;

			if (TUCAMRET_SUCCESS == TUCAM_Prop_GetAttr(m_opCam.hIdxTUCam, &propAttr))
			{
				char szBuf[64] = { 0 };
				TUCAM_VALUE_TEXT valText;
				valText.nID = TUIDP_CLRTEMPERATURE;
				valText.nTextSize = 64;
				valText.pText = &szBuf[0];

				int nCnt = (int)(propAttr.dbValMax - propAttr.dbValMin + 1);

				for (int i = 0; i < nCnt; i++)
				{
					valText.dbValue = i;
					TUCAM_Prop_GetValueText(m_opCam.hIdxTUCam, &valText);
					if (0 == val.compare(valText.pText))
					{
						TUCAM_Prop_SetValue(m_opCam.hIdxTUCam, TUIDP_CLRTEMPERATURE, i);
						break;
					}
				}
			}

			OnPropertyChanged(g_PropNameCLRTEMP, val.c_str());

			ret = DEVICE_OK;
		}
	}
	break;
	case MM::BeforeGet:
	{
		double dbVal = 0;
		TUCAM_Prop_GetValue(m_opCam.hIdxTUCam, TUIDP_CLRTEMPERATURE, &dbVal);

		char szBuf[64] = { 0 };
		TUCAM_VALUE_TEXT valText;
		valText.nID = TUIDP_CLRTEMPERATURE;
		valText.nTextSize = 64;
		valText.pText = &szBuf[0];

		valText.dbValue = dbVal;
		TUCAM_Prop_GetValueText(m_opCam.hIdxTUCam, &valText);

		pProp->Set(valText.pText);

		ret = DEVICE_OK;
	}
	break;
	default:
		break;
	}

	return ret;
}

/**
* Handles "RedGain" property.
*/
int CMMTUCam::OnRedGain(MM::PropertyBase* pProp, MM::ActionType eAct)
{
    if (NULL == m_opCam.hIdxTUCam)
        return DEVICE_NOT_CONNECTED;

    int ret = DEVICE_ERR;
    switch(eAct)
    {
    case MM::AfterSet:
        {
            long lVal = 0;
            pProp->Get(lVal);

            TUCAM_Prop_SetValue(m_opCam.hIdxTUCam, TUIDP_CHNLGAIN, lVal, 1);

            ret = DEVICE_OK;
        }
        break;
    case  MM::BeforeGet:
        {
            double dblVal = 0.0f;
            TUCAM_Prop_GetValue(m_opCam.hIdxTUCam, TUIDP_CHNLGAIN, &dblVal, 1);

            pProp->Set((long)(dblVal));

            ret = DEVICE_OK;
        }
        break;
    default:
        break;
    }

    return ret;
}

/**
* Handles "GreenGain" property.
*/
int CMMTUCam::OnGreenGain(MM::PropertyBase* pProp, MM::ActionType eAct)
{
    if (NULL == m_opCam.hIdxTUCam)
        return DEVICE_NOT_CONNECTED;

    int ret = DEVICE_ERR;
    switch(eAct)
    {
    case MM::AfterSet:
        {
            long lVal = 0;
            pProp->Get(lVal);

            TUCAM_Prop_SetValue(m_opCam.hIdxTUCam, TUIDP_CHNLGAIN, lVal, 2);

            ret = DEVICE_OK;
        }
        break;
    case  MM::BeforeGet:
        {
            double dblVal = 0.0f;
            TUCAM_Prop_GetValue(m_opCam.hIdxTUCam, TUIDP_CHNLGAIN, &dblVal, 2);

            pProp->Set((long)(dblVal));

            ret = DEVICE_OK;
        }
        break;
    default:
        break;
    }

    return ret;
}

/**
* Handles "BlueGain" property.
*/
int CMMTUCam::OnBlueGain(MM::PropertyBase* pProp, MM::ActionType eAct)
{
    if (NULL == m_opCam.hIdxTUCam)
        return DEVICE_NOT_CONNECTED;

    int ret = DEVICE_ERR;
    switch(eAct)
    {
    case MM::AfterSet:
        {
            long lVal = 0;
            pProp->Get(lVal);

            TUCAM_Prop_SetValue(m_opCam.hIdxTUCam, TUIDP_CHNLGAIN, lVal, 3);

            ret = DEVICE_OK;
        }
        break;
    case  MM::BeforeGet:
        {
            double dblVal = 0.0f;
            TUCAM_Prop_GetValue(m_opCam.hIdxTUCam, TUIDP_CHNLGAIN, &dblVal, 3);

            pProp->Set((long)(dblVal));

            ret = DEVICE_OK;
        }
        break;
    default:
        break;
    }

    return ret;
}

/**
* Handles "OnATExpMode" property.
*/
int CMMTUCam::OnATExpMode(MM::PropertyBase* pProp, MM::ActionType eAct)
{
	if (NULL == m_opCam.hIdxTUCam)
		return DEVICE_NOT_CONNECTED;

	int ret = DEVICE_ERR;
	switch (eAct)
	{
	case MM::AfterSet:
	{
		long lVal = 0;
		pProp->Get(lVal);

		TUCAM_Capa_SetValue(m_opCam.hIdxTUCam, TUIDC_ATEXPOSURE_MODE, (int)lVal);

		ret = DEVICE_OK;
	}
	break;
	case MM::BeforeGet:
	{
		int nVal = 0;

		TUCAM_Capa_GetValue(m_opCam.hIdxTUCam, TUIDC_ATEXPOSURE_MODE, &nVal);

		pProp->Set((long)nVal);

		ret = DEVICE_OK;
	}
	break;
	default:
		break;
	}

	return ret;
}

/**
* Handles "ATExposure" property.
*/
int CMMTUCam::OnATExposure(MM::PropertyBase* pProp, MM::ActionType eAct)
{
    if (NULL == m_opCam.hIdxTUCam)
        return DEVICE_NOT_CONNECTED;

    int ret = DEVICE_ERR;
    switch(eAct)
    {
    case MM::AfterSet:
        {
            string val;
            pProp->Get(val);
            if (val.length() != 0)
            {
                TUCAM_CAPA_ATTR capaAttr;
                capaAttr.idCapa = TUIDC_ATEXPOSURE;

                if (TUCAMRET_SUCCESS == TUCAM_Capa_GetAttr(m_opCam.hIdxTUCam, &capaAttr))
                {
                    if (0 == val.compare("TRUE"))
                    {
                        TUCAM_Capa_SetValue(m_opCam.hIdxTUCam, TUIDC_ATEXPOSURE, 1);
                    }
                    else
                    {
                        TUCAM_Capa_SetValue(m_opCam.hIdxTUCam, TUIDC_ATEXPOSURE, 0);
                    }
                }

                OnPropertyChanged(g_PropNameATEXP, val.c_str());

                ret = DEVICE_OK;
            }
        }
        break;
    case MM::BeforeGet:
        {
            int nVal = 0;
            TUCAM_Capa_GetValue(m_opCam.hIdxTUCam, TUIDC_ATEXPOSURE, &nVal);

            if (1 == nVal)
            {
                pProp->Set("TRUE");
            }
            else
            {
                pProp->Set("FALSE");
            }

            ret = DEVICE_OK;
        }
        break;
    default:
        break;
    }

    return ret;
}

/**
* Handles "OnTimeStamp" property.
*/
int CMMTUCam::OnTimeStamp(MM::PropertyBase* pProp, MM::ActionType eAct)
{
	if (NULL == m_opCam.hIdxTUCam)
		return DEVICE_NOT_CONNECTED;

	int ret = DEVICE_ERR;
	switch (eAct)
	{
	case MM::AfterSet:
	{
		string val;
		pProp->Get(val);
		if (val.length() != 0)
		{
			TUCAM_CAPA_ATTR capaAttr;
			capaAttr.idCapa = TUIDC_ENABLETIMESTAMP;

			if (TUCAMRET_SUCCESS == TUCAM_Capa_GetAttr(m_opCam.hIdxTUCam, &capaAttr))
			{
				if (0 == val.compare("TRUE"))
				{
					TUCAM_Capa_SetValue(m_opCam.hIdxTUCam, TUIDC_ENABLETIMESTAMP, 1);
				}
				else
				{
					TUCAM_Capa_SetValue(m_opCam.hIdxTUCam, TUIDC_ENABLETIMESTAMP, 0);
				}
			}

			OnPropertyChanged(g_PropNameATEXP, val.c_str());

			ret = DEVICE_OK;
		}
	}
	break;
	case MM::BeforeGet:
	{
		int nVal = 0;
		TUCAM_Capa_GetValue(m_opCam.hIdxTUCam, TUIDC_ENABLETIMESTAMP, &nVal);

		if (1 == nVal)
		{
			pProp->Set("TRUE");
		}
		else
		{
			pProp->Set("FALSE");
		}

		ret = DEVICE_OK;
	}
	break;
	default:
		break;
	}

	return ret;
}

/**
* Handles "Temperature" property.
*/
int CMMTUCam::OnTemperature(MM::PropertyBase* pProp, MM::ActionType eAct)
{
    if (NULL == m_opCam.hIdxTUCam)
        return DEVICE_NOT_CONNECTED;

    int ret = DEVICE_ERR;
    switch(eAct)
    {
    case MM::AfterSet:
        {
            double dblTemp;
            pProp->Get(dblTemp);          

            m_fValTemp = (float)dblTemp;
//          TUCAM_Prop_SetValue(m_opCam.hIdxTUCam, TUIDP_TEMPERATURE, (dblTemp + m_nMidTemp));
            TUCAM_Prop_SetValue(m_opCam.hIdxTUCam, TUIDP_TEMPERATURE, (dblTemp * m_fScaTemp + m_nMidTemp));

            ret = DEVICE_OK;
        }
        break;
    case MM::BeforeGet:
        {
            double dblTemp;
            if (TUCAMRET_SUCCESS == TUCAM_Prop_GetValue(m_opCam.hIdxTUCam, TUIDP_TEMPERATURE_TARGET, &dblTemp))
            {
//              pProp->Set((dblTemp - 50));
                pProp->Set((dblTemp - m_nMidTemp) / m_fScaTemp);
            }
            else
            {
                pProp->Set(m_fValTemp);
            }           

            ret = DEVICE_OK;
        }
        break;
    default:
        break;
    }

    return ret;
}

/**
* Handles "Fan" property.
*/
int CMMTUCam::OnFan(MM::PropertyBase* pProp, MM::ActionType eAct)
{
    if (NULL == m_opCam.hIdxTUCam)
        return DEVICE_NOT_CONNECTED;

    int ret = DEVICE_ERR;
    switch(eAct)
    {
    case MM::AfterSet:
        {
            string val;
            pProp->Get(val);

            if (val.length() != 0)
            {
                TUCAM_CAPA_ATTR capaAttr;
                capaAttr.idCapa = TUIDC_FAN_GEAR;

                if (TUCAMRET_SUCCESS == TUCAM_Capa_GetAttr(m_opCam.hIdxTUCam, &capaAttr))
                {
                    char szBuf[64] = {0};
                    TUCAM_VALUE_TEXT valText;
                    valText.nID       = TUIDC_FAN_GEAR;
                    valText.nTextSize = 64;
                    valText.pText     = &szBuf[0];

                    int nCnt = capaAttr.nValMax - capaAttr.nValMin + 1;

                    for (int i=0; i<nCnt; i++)
                    {
                        valText.dbValue = i;
                        TUCAM_Capa_GetValueText(m_opCam.hIdxTUCam, &valText); 

                        if (0 == val.compare(valText.pText))
                        {
                            TUCAM_Capa_SetValue(m_opCam.hIdxTUCam, TUIDC_FAN_GEAR, i);
                            break;
                        }                         
                    }
                }

                OnPropertyChanged(g_PropNameFan, val.c_str());

                ret = DEVICE_OK;
            }
        }
        break;
    case MM::BeforeGet:
        {
            int nIdx = 0;
            TUCAM_Capa_GetValue(m_opCam.hIdxTUCam, TUIDC_FAN_GEAR, &nIdx);

            char szBuf[64] = {0};
            TUCAM_VALUE_TEXT valText;
            valText.nID       = TUIDC_FAN_GEAR;
            valText.nTextSize = 64;
            valText.pText     = &szBuf[0];

            valText.dbValue = nIdx;
            TUCAM_Capa_GetValueText(m_opCam.hIdxTUCam, &valText); 

            pProp->Set(valText.pText);

            ret = DEVICE_OK;
        }
        break;
    default:
        break;
    }

    return ret; 
}

/**
* Handles "FanState" property.
*/
int CMMTUCam::OnFanState(MM::PropertyBase* pProp, MM::ActionType eAct)
{
	if (NULL == m_opCam.hIdxTUCam)
		return DEVICE_NOT_CONNECTED;

	int ret = DEVICE_ERR;
	switch (eAct)
	{
	case MM::AfterSet:
	{
		string val;
		pProp->Get(val);
		if (val.length() != 0)
		{
			if (0 == val.compare(g_FAN_ON))
			{
				TUCAM_Capa_SetValue(m_opCam.hIdxTUCam, TUIDC_FAN_GEAR, 0);
			}
			else
			{
				TUCAM_Capa_SetValue(m_opCam.hIdxTUCam, TUIDC_FAN_GEAR, 3);
			}

			ret = DEVICE_OK;
		}
	}
	break;
	case MM::BeforeGet:
	{
		int nVal = 0;
		TUCAM_Capa_GetValue(m_opCam.hIdxTUCam, TUIDC_FAN_GEAR, &nVal);

		if (3 == nVal)
		{
			pProp->Set(g_FAN_OFF);
		}
		else
		{
			pProp->Set(g_FAN_ON);
		}

		ret = DEVICE_OK;
	}
	break;
	default:
		break;
	}

	return ret;
}

/**
* Handles "LeftLevels" property.
*/
int CMMTUCam::OnLeftLevels(MM::PropertyBase* pProp, MM::ActionType eAct)
{
    if (NULL == m_opCam.hIdxTUCam)
        return DEVICE_NOT_CONNECTED;

    int ret = DEVICE_ERR;
    switch(eAct)
    {
    case MM::AfterSet:
        {
            double dblLLev = 0.0f;
            double dblRLev = 0.0f;
            pProp->Get(dblLLev);          

            TUCAM_Prop_GetValue(m_opCam.hIdxTUCam, TUIDP_RGTLEVELS, &dblRLev);
            TUCAM_Prop_SetValue(m_opCam.hIdxTUCam, TUIDP_LFTLEVELS, dblLLev);

            if ((int)dblLLev > (int)dblRLev)
            {
                dblRLev = dblLLev + 1;
                TUCAM_Prop_SetValue(m_opCam.hIdxTUCam, TUIDP_RGTLEVELS, dblRLev);
            }

            ret = DEVICE_OK;
        }
        break;
    case MM::BeforeGet:
        {
            double dblLLev = 0.0f;

            TUCAM_Prop_GetValue(m_opCam.hIdxTUCam, TUIDP_LFTLEVELS, &dblLLev);
            
            pProp->Set(dblLLev);

            ret = DEVICE_OK;
        }
        break;
    default:
        break;
    }

    return ret;
}

/**
* Handles "RightLevels" property.
*/
int CMMTUCam::OnRightLevels(MM::PropertyBase* pProp, MM::ActionType eAct)
{
    if (NULL == m_opCam.hIdxTUCam)
        return DEVICE_NOT_CONNECTED;

    int ret = DEVICE_ERR;
    switch(eAct)
    {
    case MM::AfterSet:
        {
            double dblLLev = 0.0f;
            double dblRLev = 0.0f;
            pProp->Get(dblRLev);          

            TUCAM_Prop_GetValue(m_opCam.hIdxTUCam, TUIDP_LFTLEVELS, &dblLLev);
            TUCAM_Prop_SetValue(m_opCam.hIdxTUCam, TUIDP_RGTLEVELS, dblRLev);

            if ((int)dblLLev > (int)dblRLev)
            {
                dblLLev = dblRLev - 1;
                TUCAM_Prop_SetValue(m_opCam.hIdxTUCam, TUIDP_LFTLEVELS, dblLLev);
            }

            ret = DEVICE_OK;
        }
        break;
    case MM::BeforeGet:
        {
            double dblRLev = 0.0f;

            TUCAM_Prop_GetValue(m_opCam.hIdxTUCam, TUIDP_RGTLEVELS, &dblRLev);

            pProp->Set(dblRLev);

            ret = DEVICE_OK;
        }
        break;
    default:
        break;
    }

    return ret;
}

/**
* Handles "ImageFormat" property.
*/
int CMMTUCam::OnImageFormat(MM::PropertyBase* pProp, MM::ActionType eAct)
{
    if (NULL == m_opCam.hIdxTUCam)
        return DEVICE_NOT_CONNECTED;

    int ret = DEVICE_ERR;
    switch(eAct)
    {
    case MM::AfterSet:
        {
            string val;
            pProp->Get(val);

            if (val.length() != 0)
            {
                if (0 == val.compare(g_Format_RAW))
                {

                }  

                char szPath[MAX_PATH];
                GetCurrentDirectory(MAX_PATH, szPath);
                strcat(szPath, g_FileName);

                OutputDebugString(szPath);

                // Create not exists folder
                if (!PathIsDirectory(szPath))
                    CreateDirectory(szPath, NULL);

                SYSTEMTIME sysTm;
                GetLocalTime(&sysTm);
                sprintf(m_szImgPath, ("%s\\MM_%02d%02d%02d%02d%03d"), szPath, sysTm.wDay, sysTm.wHour, sysTm.wMinute, sysTm.wSecond, sysTm.wMilliseconds);

                m_bSaving = true;

                OutputDebugString(m_szImgPath);
            }

            ret = DEVICE_OK;
        }
        break;
    case MM::BeforeGet:
        {
            pProp->Set(g_Format_RAW);

            ret = DEVICE_OK;
        }
        break;
    default:
        break;
    }

    return ret;
}

int CMMTUCam::OnTriggerMode(MM::PropertyBase* pProp, MM::ActionType eAct)
{
	if (NULL == m_opCam.hIdxTUCam)
		return DEVICE_NOT_CONNECTED;

	int ret = DEVICE_ERR;
	switch(eAct)
	{
	case MM::AfterSet:
		{
			if(IsCapturing())
				return DEVICE_CAMERA_BUSY_ACQUIRING;

			// the user just set the new value for the property, so we have to
			// apply this value to the 'hardware'.

			string val;
			pProp->Get(val);

			if (val.length() != 0)
			{
				TUCAM_Cap_GetTrigger(m_opCam.hIdxTUCam, &m_tgrAttr);

				if (0 == val.compare(g_TRIGGER_OFF))
				{
					m_tgrAttr.nTgrMode = TUCCM_SEQUENCE;
				}
				else if (0 == val.compare(g_TRIGGER_STD) || 0 == val.compare(g_TRIGGER_STDOVERLAP))
				{
					m_tgrAttr.nTgrMode = TUCCM_TRIGGER_STANDARD;
				}
				else if (0 == val.compare(g_TRIGGER_STDNONOVERLAP))
				{
					m_tgrAttr.nTgrMode = TUCCM_TRIGGER_STANDARD_NONOVERLAP;
				}
				else if (0 == val.compare(g_TRIGGER_SYN))
				{
					m_tgrAttr.nTgrMode = TUCCM_TRIGGER_SYNCHRONOUS;
					m_tgrAttr.nExpMode = TUCTE_WIDTH;
				}
				else if (0 == val.compare(g_TRIGGER_CC1))
				{
					m_tgrAttr.nTgrMode = TUCCM_TRIGGER_SYNCHRONOUS;
				}
				else if (0 == val.compare(g_TRIGGER_GLB))
				{
					m_tgrAttr.nTgrMode = TUCCM_TRIGGER_GLOBAL;
				}
				else if (0 == val.compare(g_TRIGGER_SWF))
				{
					m_tgrAttr.nTgrMode = TUCCM_TRIGGER_SOFTWARE;
				}

				if (TUCCM_TRIGGER_STANDARD != m_tgrAttr.nTgrMode || TUCTE_EXPTM != m_tgrAttr.nExpMode)
					m_tgrAttr.nFrames = 1;

				if (m_bLiving)
				{
					StopCapture();
					TUCAM_Cap_SetTrigger(m_opCam.hIdxTUCam, m_tgrAttr);
					StartCapture();
				} else
				{
					TUCAM_Cap_SetTrigger(m_opCam.hIdxTUCam, m_tgrAttr);
				}

				OnPropertyChanged(g_PropNameMdTgr, val.c_str());

				ret = DEVICE_OK;
			}
		}
		break;
	case MM::BeforeGet:
		{
			TUCAM_Cap_GetTrigger(m_opCam.hIdxTUCam, &m_tgrAttr);

			if (TUCCM_SEQUENCE == m_tgrAttr.nTgrMode)
			{
				pProp->Set(g_TRIGGER_OFF);
			}
			else if (TUCCM_TRIGGER_STANDARD == m_tgrAttr.nTgrMode)
			{
				if (IsSupport95V2New() || IsSupport401DNew() || IsSupport400BSIV3New())
				{
					pProp->Set(g_TRIGGER_STDOVERLAP);
				}
				else
				{
					pProp->Set(g_TRIGGER_STD);
				}
				
			}
			else if (TUCCM_TRIGGER_STANDARD_NONOVERLAP == m_tgrAttr.nTgrMode)
			{
				pProp->Set(g_TRIGGER_STDNONOVERLAP);
			}
			else if (TUCCM_TRIGGER_SYNCHRONOUS == m_tgrAttr.nTgrMode)
			{
				pProp->Set(m_bCC1Support ? g_TRIGGER_CC1 : g_TRIGGER_SYN);
			}
			else if (TUCCM_TRIGGER_GLOBAL == m_tgrAttr.nTgrMode)
			{
				pProp->Set(g_TRIGGER_GLB);
			}
			else if (TUCCM_TRIGGER_SOFTWARE == m_tgrAttr.nTgrMode)
			{
				pProp->Set(g_TRIGGER_SWF);
			}

			ret = DEVICE_OK;
		}
		break;
	default:
		break;
	}

	return ret; 
}

int CMMTUCam::OnTriggerExpMode(MM::PropertyBase* pProp, MM::ActionType eAct)
{
	if (NULL == m_opCam.hIdxTUCam)
		return DEVICE_NOT_CONNECTED;

	int ret = DEVICE_ERR;
	switch(eAct)
	{
	case MM::AfterSet:
		{
			if(IsCapturing())
				return DEVICE_CAMERA_BUSY_ACQUIRING;

			// the user just set the new value for the property, so we have to
			// apply this value to the 'hardware'.

			string val;
			pProp->Get(val);

			if (val.length() != 0)
			{
				TUCAM_Cap_GetTrigger(m_opCam.hIdxTUCam, &m_tgrAttr);

				if (0 == val.compare(g_TRIGGER_EXP_EXPTM))
				{
					m_tgrAttr.nExpMode = TUCTE_EXPTM;
				}
				else if (0 == val.compare(g_TRIGGER_EXP_WIDTH))
				{
					m_tgrAttr.nExpMode = TUCTE_WIDTH;
				}

				if(m_tgrAttr.nTgrMode == TUCCM_TRIGGER_SYNCHRONOUS)
					m_tgrAttr.nExpMode = TUCTE_WIDTH;

				if (TUCCM_TRIGGER_STANDARD != m_tgrAttr.nTgrMode || TUCTE_EXPTM != m_tgrAttr.nExpMode)
					m_tgrAttr.nFrames = 1;

				TUCAM_Cap_SetTrigger(m_opCam.hIdxTUCam, m_tgrAttr);

				OnPropertyChanged(g_PropNameMdExp, val.c_str());

				ret = DEVICE_OK;
			}
		}
		break;
	case MM::BeforeGet:
		{
			TUCAM_Cap_GetTrigger(m_opCam.hIdxTUCam, &m_tgrAttr);

			if (TUCTE_EXPTM == m_tgrAttr.nExpMode)
			{
				pProp->Set(g_TRIGGER_EXP_EXPTM);
			}
			else if (TUCTE_WIDTH == m_tgrAttr.nExpMode)
			{
				pProp->Set(g_TRIGGER_EXP_WIDTH);
			}

			ret = DEVICE_OK;
		}
		break;
	default:
		break;
	}

	return ret; 
}

int CMMTUCam::OnTriggerEdgeMode(MM::PropertyBase* pProp, MM::ActionType eAct)
{
	if (NULL == m_opCam.hIdxTUCam)
		return DEVICE_NOT_CONNECTED;

	int ret = DEVICE_ERR;
	switch(eAct)
	{
	case MM::AfterSet:
		{
			if(IsCapturing())
				return DEVICE_CAMERA_BUSY_ACQUIRING;

			// the user just set the new value for the property, so we have to
			// apply this value to the 'hardware'.

			string val;
			pProp->Get(val);

			if (val.length() != 0)
			{
				TUCAM_Cap_GetTrigger(m_opCam.hIdxTUCam, &m_tgrAttr);

				if (0 == val.compare(g_TRIGGER_EDGE_RISING))
				{
					m_tgrAttr.nEdgeMode = TUCTD_RISING;
				}
				else if (0 == val.compare(g_TRIGGER_EDGE_FALLING))
				{
					m_tgrAttr.nEdgeMode = TUCTD_FAILING;
				}

				TUCAM_Cap_SetTrigger(m_opCam.hIdxTUCam, m_tgrAttr);

				OnPropertyChanged(g_PropNameMdEdg, val.c_str());

				ret = DEVICE_OK;
			}
		}
		break;
	case MM::BeforeGet:
		{
			TUCAM_Cap_GetTrigger(m_opCam.hIdxTUCam, &m_tgrAttr);

			if (TUCTD_RISING == m_tgrAttr.nEdgeMode)
			{
				pProp->Set(g_TRIGGER_EDGE_RISING);
			}
			else if (TUCTD_FAILING == m_tgrAttr.nEdgeMode)
			{
				pProp->Set(g_TRIGGER_EDGE_FALLING);
			}

			ret = DEVICE_OK;
		}
		break;
	default:
		break;
	}

	return ret; 
}

int CMMTUCam::OnTriggerDelay(MM::PropertyBase* pProp, MM::ActionType eAct)
{
	if (NULL == m_opCam.hIdxTUCam)
		return DEVICE_NOT_CONNECTED;

	int ret = DEVICE_ERR;
	switch(eAct)
	{
	case MM::AfterSet:
		{
			long lVal = 0;
			pProp->Get(lVal);

			m_tgrAttr.nDelayTm = lVal;
			TUCAM_Cap_SetTrigger(m_opCam.hIdxTUCam, m_tgrAttr);

			ret = DEVICE_OK;
		}
		break;
	case  MM::BeforeGet:
		{
			TUCAM_Cap_GetTrigger(m_opCam.hIdxTUCam, &m_tgrAttr);

			pProp->Set((long)(m_tgrAttr.nDelayTm));

			ret = DEVICE_OK;
		}
		break;
	default:
		break;
	}

	return ret;
}

int CMMTUCam::OnTriggerFilter(MM::PropertyBase* pProp, MM::ActionType eAct)
{
	if (NULL == m_opCam.hIdxTUCam)
		return DEVICE_NOT_CONNECTED;

	int ret = DEVICE_ERR;
	switch (eAct)
	{
	case MM::AfterSet:
	{
		long lVal = 0;
		pProp->Get(lVal);

		TUCAM_Capa_SetValue(m_opCam.hIdxTUCam, TUIDC_SIGNALFILTER, lVal);

		ret = DEVICE_OK;
	}
	break;
	case  MM::BeforeGet:
	{
		int nVal = 0;
		TUCAM_Capa_GetValue(m_opCam.hIdxTUCam, TUIDC_SIGNALFILTER, &nVal);

		pProp->Set((long)(nVal));

		ret = DEVICE_OK;
	}
	break;
	default:
		break;
	}

	return ret;
}

int CMMTUCam::OnTriggerFrames(MM::PropertyBase* pProp, MM::ActionType eAct)
{
	if (NULL == m_opCam.hIdxTUCam)
		return DEVICE_NOT_CONNECTED;

	int ret = DEVICE_ERR;
	switch (eAct)
	{
	case MM::AfterSet:
	{
		long lVal = 0;
		pProp->Get(lVal);

		if (TUCCM_TRIGGER_STANDARD == m_tgrAttr.nTgrMode  && TUCTE_EXPTM == m_tgrAttr.nExpMode)
			m_tgrAttr.nFrames = lVal;
		else
			m_tgrAttr.nFrames = 1;

		TUCAM_Cap_SetTrigger(m_opCam.hIdxTUCam, m_tgrAttr);

		ret = DEVICE_OK;
	}
	break;
	case  MM::BeforeGet:
	{
		TUCAM_Cap_GetTrigger(m_opCam.hIdxTUCam, &m_tgrAttr);

		pProp->Set((long)(m_tgrAttr.nFrames));

		ret = DEVICE_OK;
	}
	break;
	default:
		break;
	}

	return ret;
}

int CMMTUCam::OnTriggerTotalFrames(MM::PropertyBase* pProp, MM::ActionType eAct)
{
    if (NULL == m_opCam.hIdxTUCam)
        return DEVICE_NOT_CONNECTED;

    int ret = DEVICE_ERR;
    switch (eAct)
    {
    case MM::AfterSet:
    {
        long lVal = 0;
        pProp->Get(lVal);

        if (TUCCM_SEQUENCE < m_tgrAttr.nTgrMode && m_tgrAttr.nTgrMode < TUCCM_TRIGGER_SOFTWARE)
        {
            m_tgrAttr.nFrames = lVal;
        }

		TUCAM_Cap_SetTrigger(m_opCam.hIdxTUCam, m_tgrAttr);

		ret = DEVICE_OK;
	}
	break;
	case  MM::BeforeGet:
	{
		TUCAM_Cap_GetTrigger(m_opCam.hIdxTUCam, &m_tgrAttr);

		pProp->Set((long)(m_tgrAttr.nFrames));

		ret = DEVICE_OK;
	}
	break;
	default:
		break;
	}

	return ret;
}

int CMMTUCam::OnTriggerDoSoftware(MM::PropertyBase* pProp, MM::ActionType eAct)
{
	if (NULL == m_opCam.hIdxTUCam)
		return DEVICE_NOT_CONNECTED;

	int ret = DEVICE_ERR;
	switch(eAct)
	{
	case MM::AfterSet:
		{
			string val;
			pProp->Get(val);

			if (val.length() != 0)
			{
				TUCAM_Cap_DoSoftwareTrigger(m_opCam.hIdxTUCam);
			}

			ret = DEVICE_OK;
		}
		break;
	case MM::BeforeGet:
		{
			pProp->Set(g_Format_RAW);

			ret = DEVICE_OK;
		}
		break;
	default:
		break;
	}

	return ret;
}

/**
* Handles "Sharpness" property.
*/
int CMMTUCam::OnSharpness(MM::PropertyBase* pProp, MM::ActionType eAct)
{
	if (NULL == m_opCam.hIdxTUCam)
		return DEVICE_NOT_CONNECTED;

	int ret = DEVICE_ERR;
	switch(eAct)
	{
	case MM::AfterSet:
		{
			long lVal = 0;
			pProp->Get(lVal);

			TUCAM_Prop_SetValue(m_opCam.hIdxTUCam, TUIDP_SHARPNESS, lVal);

			ret = DEVICE_OK;
		}
		break;
	case  MM::BeforeGet:
		{
			double dblVal = 0.0f;
			TUCAM_Prop_GetValue(m_opCam.hIdxTUCam, TUIDP_SHARPNESS, &dblVal);

			pProp->Set((long)(dblVal));

			ret = DEVICE_OK;
		}
		break;
	default:
		break;
	}

	return ret;
}

/**
* Handles "OnDPCAdjust" property.
*/
int CMMTUCam::OnDPCAdjust(MM::PropertyBase* pProp, MM::ActionType eAct)
{
	if (NULL == m_opCam.hIdxTUCam)
		return DEVICE_NOT_CONNECTED;

	int ret = DEVICE_ERR;
	switch(eAct)
	{
	case MM::AfterSet:
		{
			long lVal = 0;
			pProp->Get(lVal);

			TUCAM_Prop_SetValue(m_opCam.hIdxTUCam, TUIDP_DPCLEVEL, lVal);

			ret = DEVICE_OK;
		}
		break;
	case  MM::BeforeGet:
		{
			double dblVal = 0.0f;
			TUCAM_Prop_GetValue(m_opCam.hIdxTUCam, TUIDP_DPCLEVEL, &dblVal);

			pProp->Set((long)(dblVal));

			ret = DEVICE_OK;
		}
		break;
	default:
		break;
	}

	return ret;
}

/**
* Handles "OnBlackLevel" property.
*/
int CMMTUCam::OnBlackLevel(MM::PropertyBase* pProp, MM::ActionType eAct)
{
	if (NULL == m_opCam.hIdxTUCam)
		return DEVICE_NOT_CONNECTED;

	int ret = DEVICE_ERR;
	switch(eAct)
	{
	case MM::AfterSet:
		{
			long lVal = 0;
			pProp->Get(lVal);

			TUCAM_Prop_SetValue(m_opCam.hIdxTUCam, TUIDP_BLACKLEVEL, lVal);

			ret = DEVICE_OK;
		}
		break;
	case  MM::BeforeGet:
		{
			double dblVal = 0.0f;
			TUCAM_Prop_GetValue(m_opCam.hIdxTUCam, TUIDP_BLACKLEVEL, &dblVal);

			pProp->Set((long)(dblVal));

			ret = DEVICE_OK;
		}
		break;
	default:
		break;
	}

	return ret;
}

/**
* Handles "DPC Level" property.
*/
int CMMTUCam::OnDPCLevel(MM::PropertyBase* pProp, MM::ActionType eAct)
{
	if (NULL == m_opCam.hIdxTUCam)
		return DEVICE_NOT_CONNECTED;

	int ret = DEVICE_ERR;
	switch(eAct)
	{
	case MM::AfterSet:
		{
			if(IsCapturing())
				return DEVICE_CAMERA_BUSY_ACQUIRING;

			// the user just set the new value for the property, so we have to
			// apply this value to the 'hardware'.

			string val;
			pProp->Get(val);

			if (val.length() != 0)
			{
				int nVal = 0;

				if (0 == val.compare(g_DPC_OFF))
				{
					nVal = 0;
				}
				else if (0 == val.compare(g_DPC_LOW))
				{
					nVal = 1;
				}
				else if (0 == val.compare(g_DPC_MEDIUM))
				{
					nVal = 2;
				}
				else if (0 == val.compare(g_DPC_HIGH))
				{
					nVal = 3;
				}

				TUCAM_Prop_SetValue(m_opCam.hIdxTUCam, TUIDP_NOISELEVEL, nVal);

				OnPropertyChanged(g_PropNameDPC, val.c_str());

				ret = DEVICE_OK;
			}
		}
		break;
	case MM::BeforeGet:
		{
			int nVal = 0;
			double dblVal = 0.0f;

			TUCAM_Prop_GetValue(m_opCam.hIdxTUCam, TUIDP_NOISELEVEL, &dblVal);

			nVal = (int)dblVal;

			if (0 == nVal)
			{
				pProp->Set(g_DPC_OFF);
			}
			else if (1 == nVal)
			{
				pProp->Set(g_DPC_LOW);
			}
			else if (2 == nVal)
			{
				pProp->Set(g_DPC_MEDIUM);
			}
			else if (3 == nVal)
			{
				pProp->Set(g_DPC_HIGH);
			}

			ret = DEVICE_OK;
		}
		break;
	default:
		break;
	}

	return ret; 
}

/**
* Handles "OutputTriggerPort" property.
*/
int CMMTUCam::OnTrgOutPortMode(MM::PropertyBase* pProp, MM::ActionType eAct)
{
	if (NULL == m_opCam.hIdxTUCam)
		return DEVICE_NOT_CONNECTED;

	int ret = DEVICE_ERR;
	switch(eAct)
	{
	case MM::AfterSet:
		{

			// the user just set the new value for the property, so we have to
			// apply this value to the 'hardware'.
			string val;
			pProp->Get(val);

			if (val.length() != 0)
			{
				TUCAM_Cap_GetTriggerOut(m_opCam.hIdxTUCam, &m_tgrOutAttr);

				if (0 == val.compare(g_TRIGGER_PORT1))
				{
					m_tgrOutAttr.nTgrOutPort = 0;
				}
				else if (0 == val.compare(g_TRIGGER_PORT2))
				{
					m_tgrOutAttr.nTgrOutPort = 1;
				}
				else if (0 == val.compare(g_TRIGGER_PORT3))
				{
					m_tgrOutAttr.nTgrOutPort = 2;
				}
				
				m_tgrOutPara.nTgrOutPort = m_tgrOutAttr.nTgrOutPort;
				switch(m_tgrOutAttr.nTgrOutPort)
				{
				case TUPORT_ONE:
					m_tgrOutAttr.nTgrOutMode = m_tgrOutPara.TgrPort1.nTgrOutMode ;
					m_tgrOutAttr.nEdgeMode   = m_tgrOutPara.TgrPort1.nEdgeMode;
					m_tgrOutAttr.nDelayTm    = m_tgrOutPara.TgrPort1.nDelayTm ;
					m_tgrOutAttr.nWidth      = m_tgrOutPara.TgrPort1.nWidth;
					break;

				case TUPORT_TWO:
					m_tgrOutAttr.nTgrOutMode = m_tgrOutPara.TgrPort2.nTgrOutMode ;
					m_tgrOutAttr.nEdgeMode   = m_tgrOutPara.TgrPort2.nEdgeMode;
					m_tgrOutAttr.nDelayTm    = m_tgrOutPara.TgrPort2.nDelayTm ;
					m_tgrOutAttr.nWidth      = m_tgrOutPara.TgrPort2.nWidth;
					break;

				case TUPORT_THREE:
					m_tgrOutAttr.nTgrOutMode = m_tgrOutPara.TgrPort3.nTgrOutMode ;
					m_tgrOutAttr.nEdgeMode   = m_tgrOutPara.TgrPort3.nEdgeMode;
					m_tgrOutAttr.nDelayTm    = m_tgrOutPara.TgrPort3.nDelayTm ;
					m_tgrOutAttr.nWidth      = m_tgrOutPara.TgrPort3.nWidth;
					break;
				default:
					break;
				}

				TUCAM_Cap_SetTriggerOut(m_opCam.hIdxTUCam, m_tgrOutAttr);

				OnPropertyChanged(g_PropNamePort, val.c_str());

				ret = DEVICE_OK;
			}
		}
		break;
	case MM::BeforeGet:
		{
			TUCAM_Cap_GetTriggerOut(m_opCam.hIdxTUCam, &m_tgrOutAttr);

			if (0 == m_tgrOutAttr.nTgrOutPort)
			{
				pProp->Set(g_TRIGGER_PORT1);
			}
			else if (1 == m_tgrOutAttr.nTgrOutPort)
			{
				pProp->Set(g_TRIGGER_PORT2);
			}
			else if (2 == m_tgrOutAttr.nTgrOutPort)
			{
				pProp->Set(g_TRIGGER_PORT3);
			}

			ret = DEVICE_OK;
		}
		break;
	default:
		break;
	}

	return ret; 
}

/**
* Handles "OutputTriggerKind" property.
*/
int CMMTUCam::OnTrgOutKindMode(MM::PropertyBase* pProp, MM::ActionType eAct)
{
	if (NULL == m_opCam.hIdxTUCam)
		return DEVICE_NOT_CONNECTED;

	int ret = DEVICE_ERR;
	switch(eAct)
	{
	case MM::AfterSet:
		{

			// the user just set the new value for the property, so we have to
			// apply this value to the 'hardware'.
			string val;
			pProp->Get(val);

			if (val.length() != 0)
			{
				TUCAM_Cap_GetTriggerOut(m_opCam.hIdxTUCam, &m_tgrOutAttr);

				if (0 == val.compare(g_TRIGGER_EXPSTART))
				{
					m_tgrOutAttr.nTgrOutMode = 3;
				}
				else if (0 == val.compare(g_TRIGGER_READEND))
				{
					m_tgrOutAttr.nTgrOutMode = 5;
				}
				else if (0 == val.compare(g_TRIGGER_GLBEXP))
				{
					m_tgrOutAttr.nTgrOutMode = 4;
				}
				else if (0 == val.compare(g_TRIGGER_TRIREADY))
				{
					m_tgrOutAttr.nTgrOutMode = 6;
				}
				else if (0 == val.compare(g_TRIGGER_LOW))
				{
					m_tgrOutAttr.nTgrOutMode = 0;
				}
				else if (0 == val.compare(g_TRIGGER_HIGH))
				{
					m_tgrOutAttr.nTgrOutMode = 1;
				}

				switch(m_tgrOutAttr.nTgrOutPort)
				{
				case TUPORT_ONE:
					m_tgrOutPara.TgrPort1.nTgrOutMode = m_tgrOutAttr.nTgrOutMode;
					m_tgrOutPara.TgrPort1.nEdgeMode = m_tgrOutAttr.nEdgeMode;
					m_tgrOutPara.TgrPort1.nDelayTm = m_tgrOutAttr.nDelayTm;
					m_tgrOutPara.TgrPort1.nWidth = m_tgrOutAttr.nWidth;
					break;

				case TUPORT_TWO:
					m_tgrOutPara.TgrPort2.nTgrOutMode = m_tgrOutAttr.nTgrOutMode;
					m_tgrOutPara.TgrPort2.nEdgeMode = m_tgrOutAttr.nEdgeMode;
					m_tgrOutPara.TgrPort2.nDelayTm = m_tgrOutAttr.nDelayTm;
					m_tgrOutPara.TgrPort2.nWidth = m_tgrOutAttr.nWidth;
					break;

				case TUPORT_THREE:
					m_tgrOutPara.TgrPort3.nTgrOutMode = m_tgrOutAttr.nTgrOutMode;
					m_tgrOutPara.TgrPort3.nEdgeMode = m_tgrOutAttr.nEdgeMode;
					m_tgrOutPara.TgrPort3.nDelayTm = m_tgrOutAttr.nDelayTm;
					m_tgrOutPara.TgrPort3.nWidth = m_tgrOutAttr.nWidth;
					break;
				default:
					break;
				}

				TUCAM_Cap_SetTriggerOut(m_opCam.hIdxTUCam, m_tgrOutAttr);

				OnPropertyChanged(g_PropNameKind, val.c_str());

				ret = DEVICE_OK;
			}
		}
		break;
	case MM::BeforeGet:
		{
			TUCAM_Cap_GetTriggerOut(m_opCam.hIdxTUCam, &m_tgrOutAttr);

			if (0 == m_tgrOutAttr.nTgrOutMode)
			{
				pProp->Set(g_TRIGGER_LOW);
			}
			else if (1 == m_tgrOutAttr.nTgrOutMode)
			{
				pProp->Set(g_TRIGGER_HIGH);
			}
			else if (3 == m_tgrOutAttr.nTgrOutMode)
			{
				pProp->Set(g_TRIGGER_EXPSTART);
			}
			else if (4 == m_tgrOutAttr.nTgrOutMode)
			{
				pProp->Set(g_TRIGGER_GLBEXP);
			}
			else if (5 == m_tgrOutAttr.nTgrOutMode)
			{
				pProp->Set(g_TRIGGER_READEND);
			}
			else if (6 == m_tgrOutAttr.nTgrOutMode)
			{
				pProp->Set(g_TRIGGER_TRIREADY);
			}

			ret = DEVICE_OK;
		}
		break;
	default:
		break;
	}

	return ret; 
}

/**
* Handles "OutputTriggerEdge" property.
*/
int CMMTUCam::OnTrgOutEdgeMode(MM::PropertyBase* pProp, MM::ActionType eAct)
{
	if (NULL == m_opCam.hIdxTUCam)
		return DEVICE_NOT_CONNECTED;

	int ret = DEVICE_ERR;
	switch(eAct)
	{
	case MM::AfterSet:
		{
			// the user just set the new value for the property, so we have to
			// apply this value to the 'hardware'.

			string val;
			pProp->Get(val);

			if (val.length() != 0)
			{
				TUCAM_Cap_GetTriggerOut(m_opCam.hIdxTUCam, &m_tgrOutAttr);

				if (0 == val.compare(g_TRIGGER_EDGE_RISING))
				{
					m_tgrOutAttr.nEdgeMode = TUCTD_FAILING;
				}
				else if (0 == val.compare(g_TRIGGER_EDGE_FALLING))
				{
					m_tgrOutAttr.nEdgeMode = TUCTD_RISING;
				}

				switch(m_tgrOutAttr.nTgrOutPort)
				{
				case TUPORT_ONE:
					m_tgrOutPara.TgrPort1.nTgrOutMode = m_tgrOutAttr.nTgrOutMode;
					m_tgrOutPara.TgrPort1.nEdgeMode = m_tgrOutAttr.nEdgeMode;
					m_tgrOutPara.TgrPort1.nDelayTm = m_tgrOutAttr.nDelayTm;
					m_tgrOutPara.TgrPort1.nWidth = m_tgrOutAttr.nWidth;
					break;

				case TUPORT_TWO:
					m_tgrOutPara.TgrPort2.nTgrOutMode = m_tgrOutAttr.nTgrOutMode;
					m_tgrOutPara.TgrPort2.nEdgeMode = m_tgrOutAttr.nEdgeMode;
					m_tgrOutPara.TgrPort2.nDelayTm = m_tgrOutAttr.nDelayTm;
					m_tgrOutPara.TgrPort2.nWidth = m_tgrOutAttr.nWidth;
					break;

				case TUPORT_THREE:
					m_tgrOutPara.TgrPort3.nTgrOutMode = m_tgrOutAttr.nTgrOutMode;
					m_tgrOutPara.TgrPort3.nEdgeMode = m_tgrOutAttr.nEdgeMode;
					m_tgrOutPara.TgrPort3.nDelayTm = m_tgrOutAttr.nDelayTm;
					m_tgrOutPara.TgrPort3.nWidth = m_tgrOutAttr.nWidth;
					break;
				default:
					break;
				}

				TUCAM_Cap_SetTriggerOut(m_opCam.hIdxTUCam, m_tgrOutAttr);

				OnPropertyChanged(g_PropNameEdge, val.c_str());

				ret = DEVICE_OK;
			}
		}
		break;
	case MM::BeforeGet:
		{
			TUCAM_Cap_GetTriggerOut(m_opCam.hIdxTUCam, &m_tgrOutAttr);

			if (TUCTD_RISING == m_tgrOutAttr.nEdgeMode)
			{
				pProp->Set(g_TRIGGER_EDGE_FALLING);
			}
			else if (TUCTD_FAILING == m_tgrOutAttr.nEdgeMode)
			{
				pProp->Set(g_TRIGGER_EDGE_RISING);
			}

			ret = DEVICE_OK;
		}
		break;
	default:
		break;
	}

	return ret; 
}

/**
* Handles "OutputTriggerDelay" property.
*/
int CMMTUCam::OnTrgOutDelay(MM::PropertyBase* pProp, MM::ActionType eAct)
{
	if (NULL == m_opCam.hIdxTUCam)
		return DEVICE_NOT_CONNECTED;

	int ret = DEVICE_ERR;
	switch(eAct)
	{
	case MM::AfterSet:
		{
			long lVal = 0;
			pProp->Get(lVal);
            TUCAM_Cap_GetTriggerOut(m_opCam.hIdxTUCam, &m_tgrOutAttr);
			m_tgrOutAttr.nDelayTm= lVal;
			switch(m_tgrOutAttr.nTgrOutPort)
			{
			case TUPORT_ONE:
				m_tgrOutPara.TgrPort1.nTgrOutMode = m_tgrOutAttr.nTgrOutMode;
				m_tgrOutPara.TgrPort1.nEdgeMode = m_tgrOutAttr.nEdgeMode;
				m_tgrOutPara.TgrPort1.nDelayTm = m_tgrOutAttr.nDelayTm;
				m_tgrOutPara.TgrPort1.nWidth = m_tgrOutAttr.nWidth;
				break;

			case TUPORT_TWO:
				m_tgrOutPara.TgrPort2.nTgrOutMode = m_tgrOutAttr.nTgrOutMode;
				m_tgrOutPara.TgrPort2.nEdgeMode = m_tgrOutAttr.nEdgeMode;
				m_tgrOutPara.TgrPort2.nDelayTm = m_tgrOutAttr.nDelayTm;
				m_tgrOutPara.TgrPort2.nWidth = m_tgrOutAttr.nWidth;
				break;

			case TUPORT_THREE:
				m_tgrOutPara.TgrPort3.nTgrOutMode = m_tgrOutAttr.nTgrOutMode;
				m_tgrOutPara.TgrPort3.nEdgeMode = m_tgrOutAttr.nEdgeMode;
				m_tgrOutPara.TgrPort3.nDelayTm = m_tgrOutAttr.nDelayTm;
				m_tgrOutPara.TgrPort3.nWidth = m_tgrOutAttr.nWidth;
				break;
			default:
				break;
			}
			TUCAM_Cap_SetTriggerOut(m_opCam.hIdxTUCam, m_tgrOutAttr);

			ret = DEVICE_OK;
		}
		break;
	case  MM::BeforeGet:
		{
			TUCAM_Cap_GetTriggerOut(m_opCam.hIdxTUCam, &m_tgrOutAttr);

			pProp->Set((long)(m_tgrOutAttr.nDelayTm));

			ret = DEVICE_OK;
		}
		break;
	default:
		break;
	}

	return ret;
}

/**
* Handles "OutputTriggerWidth" property.
*/
int CMMTUCam::OnTrgOutWidth(MM::PropertyBase* pProp, MM::ActionType eAct)
{
	if (NULL == m_opCam.hIdxTUCam)
		return DEVICE_NOT_CONNECTED;

	int ret = DEVICE_ERR;
	switch(eAct)
	{
	case MM::AfterSet:
		{
			long lVal = 0;
			pProp->Get(lVal);
            TUCAM_Cap_GetTriggerOut(m_opCam.hIdxTUCam, &m_tgrOutAttr);
			m_tgrOutAttr.nWidth= lVal;
			switch(m_tgrOutAttr.nTgrOutPort)
			{
			case TUPORT_ONE:
				m_tgrOutPara.TgrPort1.nTgrOutMode = m_tgrOutAttr.nTgrOutMode;
				m_tgrOutPara.TgrPort1.nEdgeMode = m_tgrOutAttr.nEdgeMode;
				m_tgrOutPara.TgrPort1.nDelayTm = m_tgrOutAttr.nDelayTm;
				m_tgrOutPara.TgrPort1.nWidth = m_tgrOutAttr.nWidth;
				break;

			case TUPORT_TWO:
				m_tgrOutPara.TgrPort2.nTgrOutMode = m_tgrOutAttr.nTgrOutMode;
				m_tgrOutPara.TgrPort2.nEdgeMode = m_tgrOutAttr.nEdgeMode;
				m_tgrOutPara.TgrPort2.nDelayTm = m_tgrOutAttr.nDelayTm;
				m_tgrOutPara.TgrPort2.nWidth = m_tgrOutAttr.nWidth;
				break;

			case TUPORT_THREE:
				m_tgrOutPara.TgrPort3.nTgrOutMode = m_tgrOutAttr.nTgrOutMode;
				m_tgrOutPara.TgrPort3.nEdgeMode = m_tgrOutAttr.nEdgeMode;
				m_tgrOutPara.TgrPort3.nDelayTm = m_tgrOutAttr.nDelayTm;
				m_tgrOutPara.TgrPort3.nWidth = m_tgrOutAttr.nWidth;
				break;
			default:
				break;
			}
			TUCAM_Cap_SetTriggerOut(m_opCam.hIdxTUCam, m_tgrOutAttr);

			ret = DEVICE_OK;
		}
		break;
	case  MM::BeforeGet:
		{
			TUCAM_Cap_GetTriggerOut(m_opCam.hIdxTUCam, &m_tgrOutAttr);

			pProp->Set((long)(m_tgrOutAttr.nWidth));

			ret = DEVICE_OK;
		}
		break;
	default:
		break;
	}

	return ret;
}

/**
* Handles "ReadoutTime" property.
*/
int CMMTUCam::OnReadoutTime(MM::PropertyBase* pProp, MM::ActionType eAct)
{
   if (eAct == MM::AfterSet)
   {
      double readoutMs;
      pProp->Get(readoutMs);

      readoutUs_ = readoutMs * 1000.0;
   }
   else if (eAct == MM::BeforeGet)
   {
      pProp->Set(readoutUs_ / 1000.0);
   }

   return DEVICE_OK;
}

int CMMTUCam::OnDropPixels(MM::PropertyBase* pProp, MM::ActionType eAct)
{
   if (eAct == MM::AfterSet)
   {
      long tvalue = 0;
      pProp->Get(tvalue);
		dropPixels_ = (0==tvalue)?false:true;
   }
   else if (eAct == MM::BeforeGet)
   {
      pProp->Set(dropPixels_?1L:0L);
   }

   return DEVICE_OK;
}

int CMMTUCam::OnFastImage(MM::PropertyBase* pProp, MM::ActionType eAct)
{
   if (eAct == MM::AfterSet)
   {
      long tvalue = 0;
      pProp->Get(tvalue);
		fastImage_ = (0==tvalue)?false:true;
   }
   else if (eAct == MM::BeforeGet)
   {
      pProp->Set(fastImage_?1L:0L);
   }

   return DEVICE_OK;
}

int CMMTUCam::OnSaturatePixels(MM::PropertyBase* pProp, MM::ActionType eAct)
{
   if (eAct == MM::AfterSet)
   {
      long tvalue = 0;
      pProp->Get(tvalue);
		saturatePixels_ = (0==tvalue)?false:true;
   }
   else if (eAct == MM::BeforeGet)
   {
      pProp->Set(saturatePixels_?1L:0L);
   }

   return DEVICE_OK;
}

int CMMTUCam::OnFractionOfPixelsToDropOrSaturate(MM::PropertyBase* pProp, MM::ActionType eAct)
{
   if (eAct == MM::AfterSet)
   {
      double tvalue = 0;
      pProp->Get(tvalue);
		fractionOfPixelsToDropOrSaturate_ = tvalue;
   }
   else if (eAct == MM::BeforeGet)
   {
      pProp->Set(fractionOfPixelsToDropOrSaturate_);
   }

   return DEVICE_OK;
}

int CMMTUCam::OnShouldRotateImages(MM::PropertyBase* pProp, MM::ActionType eAct)
{
   if (eAct == MM::AfterSet)
   {
      long tvalue = 0;
      pProp->Get(tvalue);
      shouldRotateImages_ = (tvalue != 0);
   }
   else if (eAct == MM::BeforeGet)
   {
      pProp->Set((long) shouldRotateImages_);
   }

   return DEVICE_OK;
}

int CMMTUCam::OnShouldDisplayImageNumber(MM::PropertyBase* pProp, MM::ActionType eAct)
{
   if (eAct == MM::AfterSet)
   {
      long tvalue = 0;
      pProp->Get(tvalue);
      shouldDisplayImageNumber_ = (tvalue != 0);
   }
   else if (eAct == MM::BeforeGet)
   {
      pProp->Set((long) shouldDisplayImageNumber_);
   }

   return DEVICE_OK;
}

int CMMTUCam::OnStripeWidth(MM::PropertyBase* pProp, MM::ActionType eAct)
{
   if (eAct == MM::AfterSet)
   {
      pProp->Get(stripeWidth_);
   }
   else if (eAct == MM::BeforeGet)
   {
      pProp->Set(stripeWidth_);
   }

   return DEVICE_OK;
}
/*
* Handles "ScanMode" property.
* Changes allowed Binning values to test whether the UI updates properly
*/
int CMMTUCam::OnScanMode(MM::PropertyBase* pProp, MM::ActionType eAct)
{ 
   if (eAct == MM::AfterSet) {
      pProp->Get(scanMode_);
      SetAllowedBinning();
      if (initialized_) {
         int ret = OnPropertiesChanged();
         if (ret != DEVICE_OK)
            return ret;
      }
   } else if (eAct == MM::BeforeGet) {
      LogMessage("Reading property ScanMode", true);
      pProp->Set(scanMode_);
   }
   return DEVICE_OK;
}


int CMMTUCam::OnCameraCCDXSize(MM::PropertyBase* pProp , MM::ActionType eAct)
{
   if (eAct == MM::BeforeGet)
   {
		pProp->Set(cameraCCDXSize_);
   }
   else if (eAct == MM::AfterSet)
   {
      long value;
      pProp->Get(value);
		if ( (value < 16) || (33000 < value))
			return DEVICE_ERR;  // invalid image size
		if( value != cameraCCDXSize_)
		{
			cameraCCDXSize_ = value;
			img_.Resize(cameraCCDXSize_/binSize_, cameraCCDYSize_/binSize_);
		}
   }
	return DEVICE_OK;

}

int CMMTUCam::OnCameraCCDYSize(MM::PropertyBase* pProp, MM::ActionType eAct)
{
   if (eAct == MM::BeforeGet)
   {
		pProp->Set(cameraCCDYSize_);
   }
   else if (eAct == MM::AfterSet)
   {
      long value;
      pProp->Get(value);
		if ( (value < 16) || (33000 < value))
			return DEVICE_ERR;  // invalid image size
		if( value != cameraCCDYSize_)
		{
			cameraCCDYSize_ = value;
			img_.Resize(cameraCCDXSize_/binSize_, cameraCCDYSize_/binSize_);
		}
   }
	return DEVICE_OK;

}

int CMMTUCam::OnTriggerDevice(MM::PropertyBase* pProp, MM::ActionType eAct)
{
   if (eAct == MM::BeforeGet)
   {
      pProp->Set(triggerDevice_.c_str());
   }
   else if (eAct == MM::AfterSet)
   {
      pProp->Get(triggerDevice_);
   }
   return DEVICE_OK;
}


int CMMTUCam::OnCCDTemp(MM::PropertyBase* pProp, MM::ActionType eAct)
{
   if (eAct == MM::BeforeGet)
   {
      pProp->Set(ccdT_);
   }
   else if (eAct == MM::AfterSet)
   {
      pProp->Get(ccdT_);
   }
   return DEVICE_OK;
}

int CMMTUCam::OnIsSequenceable(MM::PropertyBase* pProp, MM::ActionType eAct)
{
   std::string val = "Yes";
   if (eAct == MM::BeforeGet)
   {
      if (!isSequenceable_) 
      {
         val = "No";
      }
      pProp->Set(val.c_str());
   }
   else if (eAct == MM::AfterSet)
   {
      isSequenceable_ = false;
      pProp->Get(val);
      if (val == "Yes") 
      {
         isSequenceable_ = true;
      }
   }

   return DEVICE_OK;
}


///////////////////////////////////////////////////////////////////////////////
// Private CMMTUCam methods
///////////////////////////////////////////////////////////////////////////////

/**
* Sync internal image buffer size to the chosen property values.
*/
int CMMTUCam::ResizeImageBuffer()
{
    if (NULL == m_opCam.hIdxTUCam)
        return DEVICE_NOT_CONNECTED;

    if (NULL == m_frame.pBuffer)
        return DEVICE_OUT_OF_MEMORY;

    char buf[MM::MaxStrLength];
    //int ret = GetProperty(MM::g_Keyword_Binning, buf);
    //if (ret != DEVICE_OK)
    //   return ret;
    //binSize_ = atol(buf);

    int ret = GetProperty(MM::g_Keyword_PixelType, buf);
    if (ret != DEVICE_OK)
        return ret;

    std::string pixelType(buf);

    int byteDepth = 0;

    if (pixelType.compare(g_PixelType_8bit) == 0)
    {
        byteDepth = 1;
    }
    else if (pixelType.compare(g_PixelType_16bit) == 0)
    {
        byteDepth = 2;
    }
    else if ( pixelType.compare(g_PixelType_32bitRGB) == 0)
    {
        byteDepth = 4;
    }
    else if ( pixelType.compare(g_PixelType_32bit) == 0)
    {
        byteDepth = 4;
    }
    else if ( pixelType.compare(g_PixelType_64bitRGB) == 0)
    {
        byteDepth = 8;
    }

    TUCAM_VALUE_INFO valWidth;
    TUCAM_VALUE_INFO valHeight;
    int nChnnels = (1 == m_frame.ucChannels) ? 1 : 4;

    int nIdxRes = 0;
    if (TUCAMRET_SUCCESS == TUCAM_Capa_GetValue(m_opCam.hIdxTUCam, TUIDC_RESOLUTION, &nIdxRes))
    {
        valWidth.nTextSize = nIdxRes;
        valHeight.nTextSize= nIdxRes;
    }
    
    valWidth.nID = TUIDI_CURRENT_WIDTH;
    if (TUCAMRET_SUCCESS != TUCAM_Dev_GetInfo(m_opCam.hIdxTUCam, &valWidth))
        return DEVICE_NATIVE_MODULE_FAILED;  

    valHeight.nID = TUIDI_CURRENT_HEIGHT;
    if (TUCAMRET_SUCCESS != TUCAM_Dev_GetInfo(m_opCam.hIdxTUCam, &valHeight))
        return DEVICE_NATIVE_MODULE_FAILED; 

    if (PID_FL_9BW == m_nPID || PID_FL_9BW_LT == m_nPID)
    {
        ResizeBinImageBufferFL9BW(valWidth.nValue, valHeight.nValue);
    }
    else if (PID_FL_26BW == m_nPID)
    {
        ResizeBinImageBufferFL26BW(valWidth.nValue, valHeight.nValue);
    }

    char sz[256] = {0};
    sprintf(sz, "[ResizeImageBuffer]:Width:%d, Height:%d, BytesPerPixel:%d\n", valWidth.nValue, valHeight.nValue, m_frame.ucElemBytes * nChnnels);
    OutputDebugString(sz);

    if (!m_bROI)
    {
        m_nMaxHeight = valHeight.nValue;
    }

#ifdef _WIN64
    img_.Resize(valWidth.nValue, valHeight.nValue, (m_frame.ucElemBytes * nChnnels));
#else
    img_.Resize(valWidth.nValue, valHeight.nValue, (4 == nChnnels ? 4 : (m_frame.ucElemBytes * nChnnels)));
#endif


// #ifdef _WIN64
//     img_.Resize(valWidth.nValue, valHeight.nValue, (m_frame.ucElemBytes * nChnnels));
// #else
//     // We don't use the 16bit data in this app, because of the win32 memory not allowed to create large buffer.
//     img_.Resize(valWidth.nValue, valHeight.nValue, (1/*m_frame.ucElemBytes*/ * nChnnels));
// #endif
//     
    return DEVICE_OK;
}

void CMMTUCam::ResizeBinImageBufferFL9BW(int &width, int &height)
{
    int bin = 0;
    TUCAM_Capa_GetValue(m_opCam.hIdxTUCam, TUIDC_BINNING_SUM, &bin);

    if (5 == bin)
        bin = 8;
    else if (4 == bin)
        bin = 6;
    else
        bin += 1;

    if (!m_bROI)
    {
        int nMaxWid = width / bin;
        int nMaxHei = height / bin;

        width = (nMaxWid >> 2) << 2;
        height = (nMaxHei >> 2) << 2;
    }
}

void CMMTUCam::ResizeBinImageBufferFL26BW(int &width, int &height)
{
    int bin = 0;
    TUCAM_Capa_GetValue(m_opCam.hIdxTUCam, TUIDC_BINNING_SUM, &bin);

    if (6 == bin)
        bin = 8;
    else if (7 == bin)
        bin = 16;
    else
        bin += 1;

    if (!m_bROI)
    {
        int nMaxWid = width / bin;
        int nMaxHei = height / bin;

        width = (nMaxWid >> 2) << 2;
        height = (nMaxHei >> 2) << 2;
    }
}

void CMMTUCam::GenerateEmptyImage(ImgBuffer& img)
{
   MMThreadGuard g(imgPixelsLock_);
   if (img.Height() == 0 || img.Width() == 0 || img.Depth() == 0)
      return;

   unsigned char* pBuf = const_cast<unsigned char*>(img.GetPixels());
   memset(pBuf, 0, img.Height()*img.Width()*img.Depth());
}

void CMMTUCam::TestImage(ImgBuffer& img, double exp)
{
	//std::string pixelType;
	char buf[MM::MaxStrLength];
	GetProperty(MM::g_Keyword_PixelType, buf);
	std::string pixelType(buf);

	if (img.Height() == 0 || img.Width() == 0 || img.Depth() == 0)
		return;

	double lSinePeriod = 3.14159265358979 * stripeWidth_;
	unsigned imgWidth = img.Width();
	unsigned int* rawBuf = (unsigned int*) img.GetPixelsRW();
	double maxDrawnVal = 0;
	long lPeriod = (long) imgWidth / 2;
	double dLinePhase = 0.0;
	const double dAmp = exp;
	double cLinePhaseInc = 2.0 * lSinePeriod / 4.0 / img.Height();
	if (shouldRotateImages_) {
		// Adjust the angle of the sin wave pattern based on how many images
		// we've taken, to increase the period (i.e. time between repeat images).
		cLinePhaseInc *= (((int) dPhase_ / 6) % 24) - 12;
	}

	static bool debugRGB = false;
#ifdef TIFFDEMO
	debugRGB = true;
#endif
	static  unsigned char* pDebug  = NULL;
	static unsigned long dbgBufferSize = 0;
	static long iseq = 1;



	// for integer images: bitDepth_ is 8, 10, 12, 16 i.e. it is depth per component
	long maxValue = (1L << bitDepth_)-1;

	long pixelsToDrop = 0;
	if( dropPixels_)
		pixelsToDrop = (long)(0.5 + fractionOfPixelsToDropOrSaturate_*img.Height()*imgWidth);
	long pixelsToSaturate = 0;
	if( saturatePixels_)
		pixelsToSaturate = (long)(0.5 + fractionOfPixelsToDropOrSaturate_*img.Height()*imgWidth);

	unsigned j, k;
	if (pixelType.compare(g_PixelType_8bit) == 0)
	{
		double pedestal = 127 * exp / 100.0 * GetBinning() * GetBinning();
		unsigned char* pBuf = const_cast<unsigned char*>(img.GetPixels());
		for (j=0; j<img.Height(); j++)
		{
			for (k=0; k<imgWidth; k++)
			{
				long lIndex = imgWidth*j + k;
				unsigned char val = (unsigned char) (g_IntensityFactor_ * min(255.0, (pedestal + dAmp * sin(dPhase_ + dLinePhase + (2.0 * lSinePeriod * k) / lPeriod))));
				if (val > maxDrawnVal) {
					maxDrawnVal = val;
				}
				*(pBuf + lIndex) = val;
			}
			dLinePhase += cLinePhaseInc;
		}
		for(int snoise = 0; snoise < pixelsToSaturate; ++snoise)
		{
			j = (unsigned)( (double)(img.Height()-1)*(double)rand()/(double)RAND_MAX);
			k = (unsigned)( (double)(imgWidth-1)*(double)rand()/(double)RAND_MAX);
			*(pBuf + imgWidth*j + k) = (unsigned char)maxValue;
		}
		int pnoise;
		for(pnoise = 0; pnoise < pixelsToDrop; ++pnoise)
		{
			j = (unsigned)( (double)(img.Height()-1)*(double)rand()/(double)RAND_MAX);
			k = (unsigned)( (double)(imgWidth-1)*(double)rand()/(double)RAND_MAX);
			*(pBuf + imgWidth*j + k) = 0;
		}

	}
	else if (pixelType.compare(g_PixelType_16bit) == 0)
	{
		double pedestal = maxValue/2 * exp / 100.0 * GetBinning() * GetBinning();
		double dAmp16 = dAmp * maxValue/255.0; // scale to behave like 8-bit
		unsigned short* pBuf = (unsigned short*) const_cast<unsigned char*>(img.GetPixels());
		for (j=0; j<img.Height(); j++)
		{
			for (k=0; k<imgWidth; k++)
			{
				long lIndex = imgWidth*j + k;
				unsigned short val = (unsigned short) (g_IntensityFactor_ * min((double)maxValue, pedestal + dAmp16 * sin(dPhase_ + dLinePhase + (2.0 * lSinePeriod * k) / lPeriod)));
				if (val > maxDrawnVal) {
					maxDrawnVal = val;
				}
				*(pBuf + lIndex) = val;
			}
			dLinePhase += cLinePhaseInc;
		}         
		for(int snoise = 0; snoise < pixelsToSaturate; ++snoise)
		{
			j = (unsigned)(0.5 + (double)img.Height()*(double)rand()/(double)RAND_MAX);
			k = (unsigned)(0.5 + (double)imgWidth*(double)rand()/(double)RAND_MAX);
			*(pBuf + imgWidth*j + k) = (unsigned short)maxValue;
		}
		int pnoise;
		for(pnoise = 0; pnoise < pixelsToDrop; ++pnoise)
		{
			j = (unsigned)(0.5 + (double)img.Height()*(double)rand()/(double)RAND_MAX);
			k = (unsigned)(0.5 + (double)imgWidth*(double)rand()/(double)RAND_MAX);
			*(pBuf + imgWidth*j + k) = 0;
		}

	}
	else if (pixelType.compare(g_PixelType_32bit) == 0)
	{
		double pedestal = 127 * exp / 100.0 * GetBinning() * GetBinning();
		float* pBuf = (float*) const_cast<unsigned char*>(img.GetPixels());
		float saturatedValue = 255.;
		memset(pBuf, 0, img.Height()*imgWidth*4);
		// static unsigned int j2;
		for (j=0; j<img.Height(); j++)
		{
			for (k=0; k<imgWidth; k++)
			{
				long lIndex = imgWidth*j + k;
				double value =  (g_IntensityFactor_ * min(255.0, (pedestal + dAmp * sin(dPhase_ + dLinePhase + (2.0 * lSinePeriod * k) / lPeriod))));
				if (value > maxDrawnVal) {
					maxDrawnVal = value;
				}
				*(pBuf + lIndex) = (float) value;
				if( 0 == lIndex)
				{
					std::ostringstream os;
					os << " first pixel is " << (float)value;
					LogMessage(os.str().c_str(), true);

				}
			}
			dLinePhase += cLinePhaseInc;
		}

		for(int snoise = 0; snoise < pixelsToSaturate; ++snoise)
		{
			j = (unsigned)(0.5 + (double)img.Height()*(double)rand()/(double)RAND_MAX);
			k = (unsigned)(0.5 + (double)imgWidth*(double)rand()/(double)RAND_MAX);
			*(pBuf + imgWidth*j + k) = saturatedValue;
		}
		int pnoise;
		for(pnoise = 0; pnoise < pixelsToDrop; ++pnoise)
		{
			j = (unsigned)(0.5 + (double)img.Height()*(double)rand()/(double)RAND_MAX);
			k = (unsigned)(0.5 + (double)imgWidth*(double)rand()/(double)RAND_MAX);
			*(pBuf + imgWidth*j + k) = 0;
		}

	}
	else if (pixelType.compare(g_PixelType_32bitRGB) == 0)
	{
		double pedestal = 127 * exp / 100.0;
		unsigned int * pBuf = (unsigned int*) rawBuf;

		unsigned char* pTmpBuffer = NULL;

		if(debugRGB)
		{
			const unsigned long bfsize = img.Height() * imgWidth * 3;
			if(  bfsize != dbgBufferSize)
			{
				if (NULL != pDebug)
				{
					free(pDebug);
					pDebug = NULL;
				}
				pDebug = (unsigned char*)malloc( bfsize);
				if( NULL != pDebug)
				{
					dbgBufferSize = bfsize;
				}
			}
		}

		// only perform the debug operations if pTmpbuffer is not 0
		pTmpBuffer = pDebug;
		unsigned char* pTmp2 = pTmpBuffer;
		if( NULL!= pTmpBuffer)
			memset( pTmpBuffer, 0, img.Height() * imgWidth * 3);

		for (j=0; j<img.Height(); j++)
		{
			unsigned char theBytes[4];
			for (k=0; k<imgWidth; k++)
			{
				long lIndex = imgWidth*j + k;
				unsigned char value0 =   (unsigned char) min(255.0, (pedestal + dAmp * sin(dPhase_ + dLinePhase + (2.0 * lSinePeriod * k) / lPeriod)));
				theBytes[0] = value0;
				if( NULL != pTmpBuffer)
					pTmp2[2] = value0;
				unsigned char value1 =   (unsigned char) min(255.0, (pedestal + dAmp * sin(dPhase_ + dLinePhase*2 + (2.0 * lSinePeriod * k) / lPeriod)));
				theBytes[1] = value1;
				if( NULL != pTmpBuffer)
					pTmp2[1] = value1;
				unsigned char value2 = (unsigned char) min(255.0, (pedestal + dAmp * sin(dPhase_ + dLinePhase*4 + (2.0 * lSinePeriod * k) / lPeriod)));
				theBytes[2] = value2;

				if( NULL != pTmpBuffer){
					pTmp2[0] = value2;
					pTmp2+=3;
				}
				theBytes[3] = 0;
				unsigned long tvalue = *(unsigned long*)(&theBytes[0]);
				if (tvalue > maxDrawnVal) {
					maxDrawnVal = tvalue;
				}
				*(pBuf + lIndex) =  tvalue ;  //value0+(value1<<8)+(value2<<16);
			}
			dLinePhase += cLinePhaseInc;
		}


		// ImageJ's AWT images are loaded with a Direct Color processor which expects BGRA, that's why we swapped the Blue and Red components in the generator above.
		if(NULL != pTmpBuffer)
		{
			// write the compact debug image...
			char ctmp[12];
			snprintf(ctmp,12,"%ld",iseq++);
			writeCompactTiffRGB(imgWidth, img.Height(), pTmpBuffer, ("democamera" + std::string(ctmp)).c_str());
		}

	}

	// generate an RGB image with bitDepth_ bits in each color
	else if (pixelType.compare(g_PixelType_64bitRGB) == 0)
	{
		double pedestal = maxValue/2 * exp / 100.0 * GetBinning() * GetBinning();
		double dAmp16 = dAmp * maxValue/255.0; // scale to behave like 8-bit

		double maxPixelValue = (1<<(bitDepth_))-1;
		unsigned long long * pBuf = (unsigned long long*) rawBuf;
		for (j=0; j<img.Height(); j++)
		{
			for (k=0; k<imgWidth; k++)
			{
				long lIndex = imgWidth*j + k;
				unsigned long long value0 = (unsigned short) min(maxPixelValue, (pedestal + dAmp16 * sin(dPhase_ + dLinePhase + (2.0 * lSinePeriod * k) / lPeriod)));
				unsigned long long value1 = (unsigned short) min(maxPixelValue, (pedestal + dAmp16 * sin(dPhase_ + dLinePhase*2 + (2.0 * lSinePeriod * k) / lPeriod)));
				unsigned long long value2 = (unsigned short) min(maxPixelValue, (pedestal + dAmp16 * sin(dPhase_ + dLinePhase*4 + (2.0 * lSinePeriod * k) / lPeriod)));
				unsigned long long tval = value0+(value1<<16)+(value2<<32);
				if (tval > maxDrawnVal) {
					maxDrawnVal = static_cast<double>(tval);
				}
				*(pBuf + lIndex) = tval;
			}
			dLinePhase += cLinePhaseInc;
		}
	}

	if (shouldDisplayImageNumber_) {
		// Draw a seven-segment display in the upper-left corner of the image,
		// indicating the image number.
		int divisor = 1;
		int numDigits = 0;
		while (imageCounter_ / divisor > 0) {
			divisor *= 10;
			numDigits += 1;
		}
		int remainder = imageCounter_;
		for (int i = 0; i < numDigits; ++i) {
			// Black out the background for this digit.
			// TODO: for now, hardcoded sizes, which will cause buffer
			// overflows if the image size is too small -- but that seems
			// unlikely.
			int xBase = (numDigits - i - 1) * 20 + 2;
			int yBase = 2;
			for (int x = xBase; x < xBase + 20; ++x) {
				for (int y = yBase; y < yBase + 20; ++y) {
					long lIndex = imgWidth*y + x;

					if (pixelType.compare(g_PixelType_8bit) == 0) {
						*((unsigned char*) rawBuf + lIndex) = 0;
					}
					else if (pixelType.compare(g_PixelType_16bit) == 0) {
						*((unsigned short*) rawBuf + lIndex) = 0;
					}
					else if (pixelType.compare(g_PixelType_32bit) == 0 ||
						pixelType.compare(g_PixelType_32bitRGB) == 0) {
							*((unsigned int*) rawBuf + lIndex) = 0;
					}
				}
			}
			// Draw each segment, if appropriate.
			int digit = remainder % 10;
			for (int segment = 0; segment < 7; ++segment) {
				if (!((1 << segment) & SEVEN_SEGMENT_RULES[digit])) {
					// This segment is not drawn.
					continue;
				}
				// Determine if the segment is horizontal or vertical.
				int xStep = SEVEN_SEGMENT_HORIZONTALITY[segment];
				int yStep = (xStep + 1) % 2;
				// Calculate starting point for drawing the segment.
				int xStart = xBase + SEVEN_SEGMENT_X_OFFSET[segment] * 16;
				int yStart = yBase + SEVEN_SEGMENT_Y_OFFSET[segment] * 8 + 1;
				// Draw one pixel at a time of the segment.
				for (int pixNum = 0; pixNum < 8 * (xStep + 1); ++pixNum) {
					long lIndex = imgWidth * (yStart + pixNum * yStep) + (xStart + pixNum * xStep);
					if (pixelType.compare(g_PixelType_8bit) == 0) {
						*((unsigned char*) rawBuf + lIndex) = static_cast<unsigned char>(maxDrawnVal);
					}
					else if (pixelType.compare(g_PixelType_16bit) == 0) {
						*((unsigned short*) rawBuf + lIndex) = static_cast<unsigned short>(maxDrawnVal);
					}
					else if (pixelType.compare(g_PixelType_32bit) == 0 ||
						pixelType.compare(g_PixelType_32bitRGB) == 0) {
							*((unsigned int*) rawBuf + lIndex) = static_cast<unsigned int>(maxDrawnVal);
					}
				}
			}
			remainder /= 10;
		}
	}
	dPhase_ += lSinePeriod / 4.;
}

/**
* Generate a spatial sine wave.
*/
void CMMTUCam::GenerateSyntheticImage(ImgBuffer& img, double exp)
{ 
    MMThreadGuard g(imgPixelsLock_);

    TestImage(img, exp);
    OutputDebugString("[GenerateSyntheticImage]\n");
}


void CMMTUCam::TestResourceLocking(const bool recurse)
{
    if(recurse)
        TestResourceLocking(false);
}

void __cdecl CMMTUCam::GetTemperatureThread(LPVOID lParam)
{
    CMMTUCam *pCam = (CMMTUCam *)lParam;

    if (NULL != pCam)
    {
        pCam->RunTemperature();
    }

    SetEvent(pCam->m_hThdTempEvt);
    OutputDebugString("Leave get the value of temperature thread!\n");
    _endthread();
}

void CMMTUCam::RunTemperature()
{
    DWORD dw = GetTickCount();

    while (m_bTemping)
    {
        if (GetTickCount() - dw > 1000)
        {
            double dblVal = 0.0f;
            TUCAM_Prop_GetValue(m_opCam.hIdxTUCam, TUIDP_TEMPERATURE, &dblVal);

            m_fCurTemp = (float)dblVal;

            dw = GetTickCount();

			if (isSupportSoftProtect())   // 400BSIV2 BCD = 0x05, 0x07, 0x09 ²»¿Éµ÷·çÉÈÏà»ú
			{    
				int nFan = 0;
				TUCAM_Capa_GetValue(m_opCam.hIdxTUCam, TUIDC_FAN_GEAR, &nFan);
				if (m_fCurTemp >= 0.0f && nFan == 0x03)
				{
					TUCAM_Capa_SetValue(m_opCam.hIdxTUCam, TUIDC_FAN_GEAR, 0);
				}
			}
        }
        else
        {
            Sleep(100);
        }
    }
}

int CMMTUCam::InitTUCamApi()
{
    m_itApi.pstrConfigPath = "./";
    m_itApi.uiCamCount     = 0;

    TUCAMRET nRet = TUCAM_Api_Init(&m_itApi);

	if (TUCAMRET_SUCCESS == nRet || TUCAMRET_INIT == nRet)
	{
		if (TUCAMRET_SUCCESS == nRet)
		{
			s_nNumCam = m_itApi.uiCamCount;
		}
		else
		{
			m_itApi.uiCamCount = s_nNumCam;
		}
	}
	else
		return DEVICE_NOT_CONNECTED;

	if (0 == m_itApi.uiCamCount)
		return DEVICE_NOT_CONNECTED;

	m_opCam.uiIdxOpen = s_nCntCam;

	if (TUCAMRET_SUCCESS !=  TUCAM_Dev_Open(&m_opCam))
		return DEVICE_NOT_CONNECTED;

	s_nCntCam++;
  
    if (NULL == m_opCam.hIdxTUCam)
        return DEVICE_NOT_CONNECTED;

	if (s_nCntCam > 1)
	{
		for (int i=0; i<s_nCntCam; ++i)
		{
			TUCAM_Capa_SetValue(m_opCam.hIdxTUCam, TUIDC_CAM_MULTIPLE, s_nNumCam);
		}
	}

    return DEVICE_OK;
}

int CMMTUCam::UninitTUCamApi()
{
    ReleaseBuffer();

    if (NULL != m_opCam.hIdxTUCam)
    {
        OutputDebugString("[TUCAM_Dev_Close]\n");
        TUCAM_Dev_Close(m_opCam.hIdxTUCam);     // close camera
        m_opCam.hIdxTUCam = NULL;
    }

    TUCAM_Api_Uninit();                         // release SDK resource

    return DEVICE_OK;
}

int CMMTUCam::AllocBuffer()
{
    if (NULL == m_opCam.hIdxTUCam)
        return DEVICE_NOT_CONNECTED;

    // TUCam resource
    m_frame.pBuffer     = NULL;
    m_frame.ucFormatGet = TUFRM_FMT_USUAl;
    m_frame.uiRsdSize   = 1;                    // how many frames do you want

    // Alloc buffer after set resolution or set ROI attribute
    if (TUCAMRET_SUCCESS != TUCAM_Buf_Alloc(m_opCam.hIdxTUCam, &m_frame))
    {
        return DEVICE_OUT_OF_MEMORY; 
    }

    if (3 == m_frame.ucChannels)
    {
        nComponents_ = 4;   // channels RGBA

#ifdef _WIN64
        // Do not have enough memory 
        if (2 == m_frame.ucElemBytes)
        {
            bitDepth_ = 16;
            SetProperty(MM::g_Keyword_PixelType, g_PixelType_64bitRGB);
        }  
        else
        {
            bitDepth_ = 8;
            SetProperty(MM::g_Keyword_PixelType, g_PixelType_32bitRGB);
        } 
#else
        bitDepth_ = 8;
        SetProperty(MM::g_Keyword_PixelType, g_PixelType_32bitRGB);
#endif

    }
    else
    {
        nComponents_ = 1;   // channels Gray

        if (2 == m_frame.ucElemBytes)
        {
            bitDepth_ = 16;
            SetProperty(MM::g_Keyword_PixelType, g_PixelType_16bit);
        }  
        else
        {
            bitDepth_ = 8;
            SetProperty(MM::g_Keyword_PixelType, g_PixelType_8bit);
        }
    }

    
    //SetProperty(MM::g_Keyword_PixelType, g_PixelType_16bit);
    return DEVICE_OK;
}

int CMMTUCam::ResizeBuffer()
{
    if (NULL == m_opCam.hIdxTUCam)
        return DEVICE_NOT_CONNECTED;

    TUCAM_Buf_Release(m_opCam.hIdxTUCam);
    AllocBuffer();

    return DEVICE_OK;
}

int CMMTUCam::ReleaseBuffer()
{
    if (NULL == m_opCam.hIdxTUCam)
        return DEVICE_NOT_CONNECTED;

    TUCAM_Buf_Release(m_opCam.hIdxTUCam);                   // Release alloc buffer after stop capture and quit drawing thread

    return DEVICE_OK;
}

int CMMTUCam::StopCapture()
{
    m_bLiving = false;

    TUCAM_Buf_AbortWait(m_opCam.hIdxTUCam);                 // If you called TUCAM_Buf_WaitForFrames()

    TUCAM_Cap_Stop(m_opCam.hIdxTUCam);                      // Stop capture   
    ReleaseBuffer();

    return DEVICE_OK;

}

int CMMTUCam::StartCapture()
{
    if (m_bLiving)
        return DEVICE_OK;

    m_bLiving = true;

    int nRet = AllocBuffer();
    if (nRet != DEVICE_OK)
    {
        return nRet;
    }

    // Start capture
    if (TUCAMRET_SUCCESS == TUCAM_Cap_Start(m_opCam.hIdxTUCam, m_tgrAttr.nTgrMode))
    {
        return nRet;
    }


    return DEVICE_ERR;

}

int CMMTUCam::WaitForFrame(ImgBuffer& img)
{
    MMThreadGuard g(imgPixelsLock_);

    m_frame.ucFormatGet = TUFRM_FMT_USUAl;  // Set usual format
    if (TUCAMRET_SUCCESS == TUCAM_Buf_WaitForFrame(m_opCam.hIdxTUCam, &m_frame, 1000))
    {
        if (img.Height() == 0 || img.Width() == 0 || img.Depth() == 0)
            return DEVICE_OUT_OF_MEMORY;

        int nWid = m_frame.usWidth;
        int nHei = m_frame.usHeight;
        int nPix = nWid * nHei;

//#ifdef _WIN64
        
        if (2 == m_frame.ucElemBytes)
        {
            if (3 == m_frame.ucChannels)
            {
#ifdef _WIN64
                unsigned short* pSrc = (unsigned short *)(m_frame.pBuffer + m_frame.usHeader);
                unsigned short* pDst = (unsigned short *)(img.GetPixelsRW());

                for (int i=0; i<nPix; ++i) 
                {
                    *pDst++ = *pSrc++;
                    *pDst++ = *pSrc++;
                    *pDst++ = *pSrc++;
                    *pDst++ = 0;
                }
#else
                unsigned short* pSrc = (unsigned short *)(m_frame.pBuffer + m_frame.usHeader);
                unsigned char* pDst  = (unsigned char *)(img.GetPixelsRW());

                for (int i=0; i<nPix; ++i) 
                {
                    *pDst++ = (*pSrc++) >> 8;
                    *pDst++ = (*pSrc++) >> 8;
                    *pDst++ = (*pSrc++) >> 8;
                    *pDst++ = 0;
                }
#endif
            }
            else
            {
                unsigned short* pSrc = (unsigned short *)(m_frame.pBuffer + m_frame.usHeader);
                unsigned short* pDst = (unsigned short *)(img.GetPixelsRW());

                memcpy(pDst, pSrc, m_frame.uiImgSize);
            }   
        }
        else
        {
            unsigned char* pSrc = (unsigned char *)(m_frame.pBuffer + m_frame.usHeader);
            unsigned char* pDst = (unsigned char *)(img.GetPixelsRW());

            if (3 == m_frame.ucChannels)
            {
                for (int i=0; i<nPix; ++i) 
                {
                    *pDst++ = *pSrc++;
                    *pDst++ = *pSrc++;
                    *pDst++ = *pSrc++;
                    *pDst++ = 0;
                }
            }
            else
            {
                memcpy(pDst, pSrc, m_frame.uiImgSize);
            }  
        }

  
// #else
//         unsigned char* pSrc = (unsigned char *)(m_frame.pBuffer + m_frame.usHeader + m_frame.ucElemBytes / 2);
//         unsigned char* pDst = (unsigned char *)(img.GetPixelsRW());
// 
//         if (3 == m_frame.ucChannels)
//         {
//             for (int i=0; i<nPix; ++i) 
//             {
//                 *pDst++ = *pSrc;
//                 pSrc += m_frame.ucElemBytes;
//                 *pDst++ = *pSrc;
//                 pSrc += m_frame.ucElemBytes;
//                 *pDst++ = *pSrc;
//                 pSrc += m_frame.ucElemBytes;
//                 *pDst++ = 0;
//             }
//         }
//         else
//         {
//             for (int i=0; i<nPix; ++i) 
//             {
//                 *pDst++ = *pSrc;
//                 pSrc += m_frame.ucElemBytes;
//             }
//         }
//#endif

        if (m_bSaving)
        {
            m_frame.ucFormatGet = TUFRM_FMT_RAW;
            TUCAM_Buf_CopyFrame(m_opCam.hIdxTUCam, &m_frame);
            SaveRaw(m_szImgPath, m_frame.pBuffer, m_frame.uiImgSize + m_frame.usHeader);

            m_bSaving = false;
        }

//         if (3 == m_frame.ucChannels)
//         {
//             if (2 == m_frame.ucElemBytes)
//             {
//                 unsigned short* pSrc = (unsigned short *)(m_frame.pBuffer + m_frame.usHeader);
//                 unsigned short* pDst = (unsigned short *)(img.GetPixelsRW());
// 
//                 for (int i=0; i<nPix; ++i) 
//                 {
//                     *pDst++ = *pSrc++;
//                     *pDst++ = *pSrc++;
//                     *pDst++ = *pSrc++;
//                     *pDst++ = 0;
//                 }
//             }
//             else
//             {
//                 unsigned char* pSrc = m_frame.pBuffer + m_frame.usHeader;
//                 unsigned char* pDst = img.GetPixelsRW();
// 
//                 for (int i=0; i<nPix; ++i) 
//                 {
//                     *pDst++ = *pSrc++;
//                     *pDst++ = *pSrc++;
//                     *pDst++ = *pSrc++;
//                     *pDst++ = 0;
//                 }
//             }
//         }
//         else 
//         {
//             if (2 == m_frame.ucElemBytes)
//             {
//                 unsigned char* pSrc = m_frame.pBuffer + m_frame.usHeader;
//                 unsigned char* pDst = img.GetPixelsRW();
// 
//                 for (int i=0; i<nPix; ++i) 
//                 {
//                     *pDst++ = *(pSrc+1);
//                     *pDst++ = *pSrc;
// 
//                     pSrc += 2;
//                 }
//             }
//             else
//             {
//                 memcpy(img.GetPixelsRW(), m_frame.pBuffer+m_frame.usHeader, m_frame.uiImgSize); 
//             }           
//         }
        

        return DEVICE_OK;
    }

    return DEVICE_NATIVE_MODULE_FAILED;
}

bool CMMTUCam::SaveRaw(char *pfileName, unsigned char *pData, unsigned long ulSize)
{
    FILE *pfile = NULL;
    string szPath = pfileName;
    szPath += ".raw";

    OutputDebugString(szPath.c_str());

    pfile = fopen(szPath.c_str(), "wb");

    if(NULL != pfile) 
    {
        fwrite(pData, 1, ulSize, pfile);
        fclose(pfile);

//        delete pfile;

        pfile = NULL;
        OutputDebugString("[SaveRaw]:NULL!\n");

        return true;
    }

    return false;
}

bool CMMTUCam::isSupportFanCool()
{
	bool bSupport = false;
	bSupport = isSupportFanWaterCool();
    if (m_nPID == PID_FL_9BW || PID_FL_9BW_LT == m_nPID || PID_FL_20BW == m_nPID || m_nPID == PID_FL_26BW)
	{
		return true;
	}
	if (IsSupportAries16())
	{
		return true;
	}
	return bSupport;
}

bool CMMTUCam::isSupportFanWaterCool()
{
	if ((DHYANA_400BSIV2 == m_nPID && (0x04 == m_nBCD || 0x06 == m_nBCD || 0x08 == m_nBCD || m_nBCD > 0x09)))      // 400BSIV2 BCD = 0x05, 0x07, 0x09 不可调风扇相机
	{
		return true;
	}

	if (DHYANA_D95_V2 == m_nPID || DHYANA_400BSIV3 == m_nPID) 
	{
		return true;
	}

	if (DHYANA_4040V2 == m_nPID || DHYANA_4040BSI == m_nPID || DHYANA_XF4040BSI == m_nPID)
	{
		return true;
	}

	return false;
}

bool  CMMTUCam::isSupportSoftProtect()
{
	bool bSupport = false;
	bSupport = isSupportFanWaterCool();
	if (IsSupport400BSIV3New() || IsSupport95V2New())
	{
		return false;
	}
	return bSupport;
}

void CMMTUCam::UpdateSlitHeightRange()
{
	int nImgMode = 0;
	TUCAM_Capa_GetValue(m_opCam.hIdxTUCam, TUIDC_ROLLINGSCANSLIT, &m_rsPara.nSlitHeight);
	TUCAM_Capa_GetValue(m_opCam.hIdxTUCam, TUIDC_IMGMODESELECT, &nImgMode);
	if ((0x03 == nImgMode || 0x04 == nImgMode) && 0x00 != m_rsPara.nMode)  // HighSpeed Mode
	{
		m_rsPara.nSlitHeightMin = 2;
		m_rsPara.nSlitHeightStep = 2;
		m_rsPara.nSlitHeight = ((m_rsPara.nSlitHeight + 1) >> 1) << 1;
		m_rsPara.nSlitHeight = min(max(m_rsPara.nSlitHeight, m_rsPara.nSlitHeightMin), m_rsPara.nSlitHeightMax);
	}
	else
	{
		m_rsPara.nSlitHeightMin = 1;
		m_rsPara.nSlitHeightStep = 1;
	}
	
	if (0x02 == m_rsPara.nMode)
	{
		m_rsPara.nSlitHeight = min(max(m_rsPara.nSlitHeight, m_rsPara.nSlitHeightMin), m_rsPara.nSlitHeightMax);
		TUCAM_Capa_SetValue(m_opCam.hIdxTUCam, TUIDC_ROLLINGSCANSLIT, m_rsPara.nSlitHeight);
		m_rsPara.nLTDelay = max(min(LineIntervalCal(m_rsPara.nSlitHeight / m_rsPara.nSlitHeightStep), m_rsPara.nLTDelayMax), m_rsPara.nLTDelayMin);
		TUCAM_Capa_SetValue(m_opCam.hIdxTUCam, TUIDC_ROLLINGSCANLTD, m_rsPara.nLTDelay);
		m_rsPara.dbLineInvalTm = LineIntervalTime(m_rsPara.nLTDelay);
	}
}

void CMMTUCam::UpdateExpRange()
{
	TUCAM_PROP_ATTR  propAttr;
	propAttr.nIdxChn = 0;
	propAttr.idProp = TUIDP_EXPOSURETM;
	TUCAM_Prop_GetAttr(m_opCam.hIdxTUCam, &propAttr);
	exposureMinimum_ = propAttr.dbValMin;
	exposureMaximum_ = propAttr.dbValMax;

    if (PID_FL_9BW == m_nPID || PID_FL_9BW_LT == m_nPID || PID_FL_20BW == m_nPID || PID_FL_26BW == m_nPID)
	{
		exposureMaximum_ = 3600000;
	}

	if (DHYANA_400BSIV3 == m_nPID || DHYANA_400BSIV2 == m_nPID || DHYANA_D95_V2 == m_nPID)
	{
		exposureMaximum_ = 10000;
	}

	SetPropertyLimits(MM::g_Keyword_Exposure, exposureMinimum_, exposureMaximum_);
}

void CMMTUCam::UpdateLevelsRange()
{
    TUCAM_PROP_ATTR  propAttr;
    propAttr.nIdxChn = 0;
    propAttr.idProp = TUIDP_LFTLEVELS;
    if (TUCAMRET_SUCCESS == TUCAM_Prop_GetAttr(m_opCam.hIdxTUCam, &propAttr))
    {
        SetPropertyLimits(g_PropNameLLev, (int)propAttr.dbValMin, (int)propAttr.dbValMax);

        SetProperty(g_PropNameLLev, CDeviceUtils::ConvertToString((int)propAttr.dbValMin));
    }

    propAttr.idProp = TUIDP_RGTLEVELS;
    if (TUCAMRET_SUCCESS == TUCAM_Prop_GetAttr(m_opCam.hIdxTUCam, &propAttr))
    {
        SetPropertyLimits(g_PropNameRLev, (int)propAttr.dbValMin, (int)propAttr.dbValMax);

        SetProperty(g_PropNameRLev, CDeviceUtils::ConvertToString((int)propAttr.dbValMax));
    }
}
