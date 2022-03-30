#include "pch.h"
#include "module.h"
#include"timer.h"

Timer* timer = new Timer();
MatMul::MatMul(Variable* a, Variable* b, Variable*c, int m, int n, int p) :a(a),b(b),c(c),m(m),n(n),p(p){
}

void MatMul::forward(bool isTraining) {
	timer->timerStart(Timer::TMR_MATMUL_FW);
	c->setZero();
	for (int i = 0;i < m;i++) {
		for (int j = 0;j < n;j++) {
			for (int k = 0;k < p;k++) {
				c->data[i*p + k] += a->data[i*n+j] * b->data[j*p+k];
			}
		}
	}
	timer->timerStop(Timer::TMR_MATMUL_FW);

}

void MatMul::backward() {
	timer->timerStart(Timer::TMR_MATMUL_BW);
	a->setGradZero();b->setGradZero();
	for (int i = 0;i < m;i++) {
		for (int j = 0;j < n;j++) {
			float tmp = 0;
			for (int k = 0;k < p;k++) {
				tmp += c->grad[i*p + k] * b->data[j*p + k];
				b->grad[j*p + k] += c->grad[i*p + k] * a->data[i*n + j];
			}
			a->grad[i*n + j] = tmp;
		}
	}
	timer->timerStop(Timer::TMR_MATMUL_BW);
}

SparseMatMul::SparseMatMul(Variable*a, Variable*b, Variable*c, SparseIndex*sp, int m, int n, int p)
:a(a),b(b),c(c),sp(sp),m(m),n(n),p(p){

}

void SparseMatMul::forward(bool isTraining) {
	timer->timerStart(Timer::TMR_SPMATMUL_FW);
	c->setZero();
	for (int i = 0;i < sp->indptr.size() - 1;i++) {
		for (int jj = sp->indptr[i];jj< sp->indptr[i + 1];jj++) {
			int j = sp->indices[jj];
			for (int k = 0;k < p;k++) {
				c->data[i*p + k] += a->data[jj] * b->data[j*p+k];
			}
		}
	}
	timer->timerStop(Timer::TMR_SPMATMUL_FW);
}

void SparseMatMul::backward() {
	timer->timerStart(Timer::TMR_SPMATMUL_BW);
	b->setGradZero();
	int row = 0;

	timer->timerStop(Timer::TMR_SPMATMUL_BW);
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
