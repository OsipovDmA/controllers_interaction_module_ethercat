#include "ethercat_timer.h"
#include <chrono>
#include <thread>
#include <iostream>

#define SIGN(T) ((T > 0) - (T < 0))

// Basic EthercatTimer
EthercatTimer::EthercatTimer() : frequency{0} {}

EthercatTimer::~EthercatTimer() = default;

void EthercatTimer::SetFrequency(uint32_t frequency)
{
	this->frequency = frequency;
	this->period_us = MICROSECS_PER_SEC / frequency;
}

void EthercatTimer::SetPeriodMicroseconds(uint32_t period_us)
{	
	this->period_us = period_us;
	this->frequency = MICROSECS_PER_SEC / period_us;
}

uint32_t EthercatTimer::GetFrequency()
{
	return this->frequency;
}

uint32_t EthercatTimer::GetPeriodMicroseconds()
{
	return this->period_us;
}


// DistributedClocksTimer
DCTimerMasterToReference::DCTimerMasterToReference() : 
	EthercatTimer(),
	shift{0},
	is_timer_launched{false},	
	dc_start_time_ns{0},
	dc_time_ns{0},
	dc_started{0},
	dc_diff_ns{0},
	prev_dc_diff_ns{0},
	dc_diff_total_ns{0},
	dc_delta_total_ns{0},
	dc_filter_idx{0},
	dc_adjust_ns{0},
 	system_time_base{0},
 	wakeup_time{0},
 	overruns{0}
{}

void DCTimerMasterToReference::Sleep()
{
	bool wake_up_flag = false;
	uint64_t period_nanoseconds = EthercatTimer::GetPeriodMicroseconds()*NANOSECS_PER_MICROSEC;

	while(!wake_up_flag)
	{
		if(!is_timer_launched) 
		{
	        clock_gettime(CLOCK_REALTIME, &dcTime_ref);
	        AddNanosecondsToTimespec(period_nanoseconds, &dcTime_ref);
	        is_timer_launched  = true;
	    }

	    int64_t dcTime_curr_ns = SystemTimeNanoseconds();
	    int64_t cycle_diff_ns = TimespecToNanoseconds(&dcTime_ref) - dcTime_curr_ns;

	    if(cycle_diff_ns <= 0)
	    {
	    	AddNanosecondsToTimespec(period_nanoseconds, &dcTime_ref);
			wake_up_flag = true;	
	    }
	}
}

void DCTimerMasterToReference::ConfigureClocks()
{
	auto slaves_map = this->slaves->GetMap();
	for (auto it = slaves_map->begin(); it != slaves_map->end(); ++it)
	{
		ecrt_slave_config_dc(
			it->second->GetConfig(),
			it->second->GetAssignActivate(),
			this->EthercatTimer::GetPeriodMicroseconds() * NANOSECS_PER_MICROSEC, 
			this->GetShiftMicroseconds() * NANOSECS_PER_MICROSEC,
			0, 
			0);
	}
	if(this->master && this->reference_slave)
	{
		if(ecrt_master_select_reference_clock(
			this->master->GetRequest(),
			this->reference_slave->GetConfig()))
		{
			std::cerr << ">>> Ethercat: failed to select reference clock.\n";
		}
	}
}

void DCTimerMasterToReference::SyncDistributedClocks(EthercatMaster* master)
{
    uint32_t ref_time = 0;
    uint64_t prev_app_time = dc_time_ns;
    dc_time_ns = this->SystemTimeNanoseconds();

    if (this->master)
    {
    	ecrt_master_application_time(master->GetRequest(), dc_time_ns);
    	ecrt_master_reference_clock_time(master->GetRequest(), &ref_time);
	}
    dc_diff_ns = (uint32_t)prev_app_time - ref_time;

    if (this->master)
    {
    	ecrt_master_sync_slave_clocks(master->GetRequest());
    }
}

void DCTimerMasterToReference::UpdateMasterClock()
{
	//printf("system_time_base: %i\n", system_time_base);
	uint64_t period_nanoseconds = EthercatTimer::GetPeriodMicroseconds()*NANOSECS_PER_MICROSEC;
    int32_t delta = dc_diff_ns - prev_dc_diff_ns;
    prev_dc_diff_ns = dc_diff_ns;

    // normalise the time diff
    dc_diff_ns =
        ((dc_diff_ns + (period_nanoseconds / 2)) % period_nanoseconds) - (period_nanoseconds / 2);
        
    //printf("dc_diff_ns: %i\n",dc_diff_ns); // akg 
        
    // only update if primary master
    if (dc_started)
    {
        // add to totals
        dc_diff_total_ns += dc_diff_ns;
        dc_delta_total_ns += delta;
        dc_filter_idx++;

        if (dc_filter_idx >= DC_FILTER_CNT)
        {
            // add rounded delta average
            dc_adjust_ns +=
                ((dc_delta_total_ns + (DC_FILTER_CNT / 2)) / DC_FILTER_CNT);

            // and add adjustment for general diff (to pull in drift)
            dc_adjust_ns += SIGN(dc_diff_total_ns / DC_FILTER_CNT);
            //printf("dc_adjust_ns: %i\n", dc_adjust_ns);

            // limit crazy numbers (0.1% of std cycle time)
            if (dc_adjust_ns < -1000)
            {
                dc_adjust_ns = -1000;
            }
            if (dc_adjust_ns > 1000)
            {
                dc_adjust_ns = 1000;
            }

            dc_diff_total_ns = 0LL;
            dc_delta_total_ns = 0LL;
            dc_filter_idx = 0;
        }
        system_time_base += dc_adjust_ns + 5*SIGN(dc_diff_ns);
    }
    else
    {
        dc_started = (dc_diff_ns != 0);
        if (dc_started)
        {
            dc_start_time_ns = dc_time_ns;
        }
    }
}	

int64_t DCTimerMasterToReference::SystemTimeNanoseconds()
{
	clock_gettime(CLOCK_REALTIME, &ts);
    return TimespecToNanoseconds(&ts) - system_time_base;
}

int64_t DCTimerMasterToReference::TimespecToNanoseconds(timespec* ts)
{
	return (uint64_t) ts->tv_sec * NANOSECS_PER_SEC + ts->tv_nsec;
}

void DCTimerMasterToReference::AddNanosecondsToTimespec(uint64_t ns, timespec* ts)
{
	ts->tv_sec += ns / NANOSECS_PER_SEC; // ts.tv_sec = ts.tv_sec + nanosecs / NSECS_IN_SEC
    ns = ns % NANOSECS_PER_SEC;
    
    if(ts->tv_nsec + ns > NANOSECS_PER_SEC) {
        ts->tv_sec += 1;
        ts->tv_nsec += (ns - NANOSECS_PER_SEC);
    }
    else
    {
        ts->tv_nsec += ns;
    }
}

void DCTimerMasterToReference::SetMaster(EthercatMaster* master) { this->master = master; }
void DCTimerMasterToReference::SetReferenceSlaveClock(EthercatSlave* slave) { this->reference_slave = slave; }
void DCTimerMasterToReference::SetSlavesClocks(EthercatSlavesContainer* slaves) { this->slaves = slaves; }
void DCTimerMasterToReference::SetShiftMicroseconds(uint32_t shift) { this->shift = shift; }
uint32_t DCTimerMasterToReference::GetShiftMicroseconds() { return this->shift; }

// SimpleTimer
void SimpleTimer::Sleep()
{
	std::chrono::microseconds us(
		EthercatTimer::GetPeriodMicroseconds()
		);
	std::this_thread::sleep_for(us);
}

void SimpleTimer::ConfigureClocks()
{}

void SimpleTimer::SyncDistributedClocks(EthercatMaster* master)
{}

void SimpleTimer::UpdateMasterClock()
{}