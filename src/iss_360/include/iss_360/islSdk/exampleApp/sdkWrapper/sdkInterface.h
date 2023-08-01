#ifndef SDKINTERFACE_H_
#define SDKINTERFACE_H_

//------------------------------------------ Includes ----------------------------------------------

#include "islSdk.h"
#include "sysPort.h"
#include "logger.h"
#include "device.h"	
#include "iss360.h"

//--------------------------------------- Class Definition -----------------------------------------

namespace IslSdk
{
	uint16_t initialise(void(*newEvent)(void), void(*newPort)(SysPort& port), void(*newDevice)(SysPort& port, Device& device));
	void cleanUp();
	void setDeviceRediscoveryTimings(uint32_t count, uint32_t timeoutMs);
}

//--------------------------------------------------------------------------------------------------
#endif
