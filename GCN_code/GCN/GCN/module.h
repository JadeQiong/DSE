#pragma once
#include<immintrin.h>
#include"variable.h"
#include"sparse.h"
class Module
{
public:
	virtual void forward(bool) = 0;
	virtual void backward() = 0;
	virtual ~Module() {};
};

class MatMul:public Module {
	Variable* a, *b, *c;
	int m, n, p;
public:
	MatMul(Variable*a, Variable*b, Variable*c, int m, int n, int p);
	~MatMul(){}
	void forward(bool);
	void backward();
};

class SparseMatMul:public Module
{
	Variable*a, *b, *c;
	SparseIndex *sp;
	int m, n, p;
public:
	SparseMatMul(Variable* a, Variable*b, Variable*c, SparseIndex* sp, int m, int n, int p);
	~SparseMatMul() {};
	void forward(bool);
	void backward();
};

class GraphSum :public Module {
	Variable* in, *out;
	SparseIndex*graph;
	int dim;
public:
	GraphSum(Variable*in, Variable*out, SparseIndex* graph,int dim);
	~GraphSum() {};
	void forward(bool);
	void backward();
};

class CrossEntropyLoss:public Module {
	Variable* logits;
	int* truth;	
	float * loss;
	int num_classes;
public:
	CrossEntropyLoss(Variable* logits, int* truth, float* loss, int num_classes);
	~CrossEntropyLoss() {};
	void forward(bool);
	void backward();
};

class ReLU :public Module {
	Variable* in;
	bool *mask;
public:
	ReLU(Variable*in);
	~ReLU() {};
	void forward(bool);
	void backward();
};

class Dropout :public Module {
	Variable* in;
	bool *mask;
	float p;
public:
	Dropout(Variable*in, float p);
	~Dropout() {};
	void forward(bool);
	void backward();
};