#include "pch.h"
#include "timer.h"


Timer::Timer()
{
}


Timer::~Timer()
{
}

void Timer::timerStart(timerInstance t) {
	tmr_t0[t] = std::chrono::high_resolution_clock::now();
}

float Timer::timerStop(timerInstance t) {
	float cnt = std::chrono::duration_cast<std::chrono::duration<float>>(std::chrono::high_resolution_clock::now()-tmr_t0[t]).count();
	tmrSum[t] += cnt;
	return cnt;
}

float Timer::timerTotal(timerInstance t) {
	return tmrSum[t];
}
