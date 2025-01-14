#include "simple_timer.h"
#include <thread>

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

int64_t SimpleTimer::GetCurrentTime()
{
	timespec ts;
	clock_gettime(CLOCK_REALTIME, &ts);
    return (uint64_t) ts.tv_sec * 1e+9 + ts.tv_nsec;
}	