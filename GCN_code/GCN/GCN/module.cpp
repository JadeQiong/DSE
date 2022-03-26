#include "pch.h"
#include "module.h"


MatMul::MatMul(Variable* a, Variable* b, Variable*c, int m, int n, int p) {

}

void MatMul::forward(bool isTraining) {

}

void MatMul::backward() {

}

SparseMatMul::SparseMatMul(Variable*a, Variable*b, Variable*c, SparseIndex*sp, int m, int n, int p) {

}

void SparseMatMul::forward(bool isTraining) {

}

void SparseMatMul::backward() {

}

GraphSum::GraphSum(Variable* in, Variable* out, SparseIndex* graph,int dim) {

}

void GraphSum::forward(bool isTraining) {

}

void GraphSum::backward() {

}

CrossEntropyLoss::CrossEntropyLoss( Variable* in, int* truth, float* loss, int num_class) {

}

void CrossEntropyLoss::forward(bool isTraining) {

}

void CrossEntropyLoss::backward() {

}

ReLU::ReLU(Variable*in ) {

}

void ReLU::forward(bool isTraining) {
	//
}

void ReLU::backward() {

}

Dropout::Dropout(Variable*in, float p) {

}

void Dropout::forward(bool isTraining) {
	if (!isTraining) return;
	const int threshold = int(p * RAND_MAX);
	float scale = 1 / (1 - p);
	for (int i = 0; i < in->data.size(); i++) {
		bool keep = (int)rand() >= threshold;
		in->data[i] *= keep ? scale : 0;
		if (mask) mask[i] = keep;
	}
}

void Dropout::backward() {

}
