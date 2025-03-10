#ifndef ETHERCAT_TIMER_H
#define ETHERCAT_TIMER_H

#include <cstdint>
#include "ecrt.h"
#include "ethercat_slaves_container.h"
#include "ethercat_master.h"

class EthercatTimer
{
	static const int kMicrosecsPerSec = 1e+6;
	uint32_t frequency;
	uint32_t period_us;

public:
	EthercatTimer();
	virtual ~EthercatTimer();
	
	virtual void Sleep() = 0;
	virtual void ConfigureClocks() = 0;
	virtual void SyncDistributedClocks(EthercatMaster* master) = 0;
	virtual void UpdateMasterClock() = 0;	

	void SetFrequency(uint32_t);
	void SetPeriodMicroseconds(uint32_t);	
	uint32_t GetFrequency();
	uint32_t GetPeriodMicroseconds();

	virtual int64_t GetCurrentTime() = 0;	
};

#endif