#pragma once
#include<chrono>
#include<vector>
class Timer
{
private:

public:
	typedef enum {
		TMR_TRAIN = 0,
		TMR_TEST,
		TMR_MATMUL_FW,
		TMR_MATMUL_BW,
		TMR_SPMATMUL_FW,
		TMR_SPMATMUL_BW,
		TMR_GRAPHSUM_FW,
		TMR_GRAPHSUM_BW,
		TMR_LOSS_FW,
		TMR_RELU_FW,
		TMR_RELU_BW,
		TMR_DROPOUT_FW,
		TMR_DROPOUT_BW,
		__NUM_TMR
	}timerInstance;
	float tmrSum[__NUM_TMR];
	std::chrono::time_point<std::chrono::high_resolution_clock> tmr_t0[__NUM_TMR];
	Timer();
	~Timer();
	void timerStart(timerInstance t);
	float timerStop(timerInstance t);
	float timerTotal(timerInstance t);
};

