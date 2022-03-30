#include "pch.h"
#include "variable.h"
# define RAND_MAX 0x7ffffff

Variable::Variable(int size, bool requiresGrad, bool localGrad)
:data(size),grad(requiresGrad?size:0){
}

Variable::~Variable()
{
}

void Variable::setZero() {
	for (int i = 0;i < data.size();i++) {
		data[i] = 0;
	}
}

void Variable::setGradZero() {
	for (int i = 0;i < grad.size();i++) {
		grad[i] = 0;
	}
}

//?
void Variable::glorot(int inSize, int outSize) {
	float range = sqrtf(6.0f / (inSize + outSize));
	for (int i = 0;i < data.size();i++) {
		data[i] = (float(rand()) / RAND_MAX - 0.5)*range * 2;
	}
}

void Variable::print(int col) {
	int cnt = 0;
	for (auto x : data) {
		printf("%.4f ", x);
		cnt++;
		if (cnt%col == 0) printf("\n");
	}
}

float Variable::gradNorm() {
	float norm = 0;
	for (auto x : grad){
		norm += x * x;
	}
	return sqrtf(norm);
}