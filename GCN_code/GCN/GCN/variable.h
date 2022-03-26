#pragma once
#include<vector>
class Variable
{

public:
	std::vector<float> data, grad;
	std::vector<std::vector<float>> local_grad;
	Variable(int size, bool requireGrad = true, bool threadLocalGrad = false);
	void glorot(int inSize, int outSize);
	void zero();
	void zero_grad();
	void print(int col=0x7fffffff);
	float gradNorm();
	~Variable();
};

