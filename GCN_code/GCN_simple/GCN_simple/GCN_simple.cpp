// GCN_simple.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "pch.h"
#include <iostream>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#define MY_RAND_MAX 0x7fffffff

typedef struct SparseIndex {
	int *indices, *indptr;
	int indices_size, indptr_size;
}SparseIndex;

typedef struct GCNData {
	SparseIndex feature_index, graph;
	int *split, *label, split_size, label_size, feature_value_size;
	float *feature_value;
}GCNData;

GCNData Data;
int num_nodes = 2708, input_dim = 1433, hidden_dim = 16, output_dim = 7;
float dropout = 0.5f, learning_rate = 0.01f, weight_decay = 5e-4f;
int epochs = 100, early_stopping = 0;
int *truth;

typedef struct Variable {
	float *data, *grad;
	int data_size, grad_size;
}Variable;

typedef struct Adam {
	float lr, beta1, beta2, eps, weight_decay;
	int step_count;
}Adam;

Variable input, layer1_var1, layer1_weight, layer1_var2, layer2_var1, layer2_weight, output;
int *maskD1, *maskD2, *maskR1;
Variable tmp1, tmp2;
Adam adam;
unsigned long long rand_state[2];
int res = 0;

void init_rand_state() {
	int x = 0, y = 0;
	while (x == 0 || y == 0) {
		x = rand();
		y = rand();
	}
	rand_state[0] = x;
	rand_state[1] = y;
}

int RAND() {
	unsigned long long t = rand_state[0];
	unsigned long long s = rand_state[1];
	rand_state[0] = s;
	t ^= t << 23;
	t ^= t >> 17;
	t ^= s ^ (s >> 26);
	rand_state[1] = t;
	res = (t + s) & 0x7fffffff;
	return res;
}

float loss;

void init() {
	float range;
	int i;
	init_rand_state();

	input.data_size = Data.feature_index.indices_size;
	input.data = (float*)calloc(input.data_size, sizeof(float));
	input.grad_size = 0;

	layer1_var1.data_size = num_nodes * hidden_dim;
	layer1_var1.data = (float*)calloc(layer1_var1.data_size, sizeof(float));
	layer1_var1.grad_size = num_nodes * hidden_dim;
	layer1_var1.grad = (float*)calloc(layer1_var1.grad_size, sizeof(float));

	layer1_weight.data_size = input_dim * hidden_dim;
	layer1_weight.data = (float*)calloc(layer1_weight.data_size, sizeof(float));
	layer1_weight.grad_size = input_dim * hidden_dim;
	layer1_weight.grad = (float*)calloc(layer1_weight.grad_size, sizeof(float));
	range = sqrtf(6.0f / (input_dim + hidden_dim));
	for (i = 0; i < layer1_weight.data_size; i++)
		layer1_weight.data[i] = (((float)(RAND())) / MY_RAND_MAX - 0.5f)*range * 2;

	layer1_var2.data_size = num_nodes * hidden_dim;
	layer1_var2.data = (float*)calloc(layer1_var2.data_size, sizeof(float));
	layer1_var2.grad_size = num_nodes * hidden_dim;
	layer1_var2.grad = (float*)calloc(layer1_var2.grad_size, sizeof(float));

	layer2_var1.data_size = num_nodes * output_dim;
	layer2_var1.data = (float*)calloc(layer2_var1.data_size, sizeof(float));
	layer2_var1.grad_size = num_nodes * output_dim;
	layer2_var1.grad = (float*)calloc(layer2_var1.grad_size, sizeof(float));

	layer2_weight.data_size = output_dim * hidden_dim;
	layer2_weight.data = (float*)calloc(layer2_weight.data_size, sizeof(float));
	layer2_weight.grad_size = output_dim * hidden_dim;
	layer2_weight.grad = (float*)calloc(layer2_weight.grad_size, sizeof(float));
	range = sqrtf(6.0f / (output_dim + hidden_dim));
	for (i = 0; i < layer2_weight.data_size; i++)
		layer2_weight.data[i] = (((float)(RAND())) / MY_RAND_MAX - 0.5f)*range * 2;

	output.data_size = num_nodes * output_dim;
	output.data = (float*)calloc(output.data_size, sizeof(float));
	output.grad_size = num_nodes * output_dim;
	output.grad = (float*)calloc(output.grad_size, sizeof(float));

	truth = (int*)calloc(num_nodes, sizeof(int));

	maskD1 = NULL;
	maskR1 = (int*)calloc(layer1_var2.data_size, sizeof(int));
	maskD2 = (int*)calloc(layer1_var2.data_size, sizeof(int));

	tmp1.data_size = layer1_weight.data_size;
	tmp1.data = (float*)calloc(tmp1.data_size, sizeof(float));
	tmp1.grad_size = layer1_weight.grad_size;
	tmp1.grad = (float*)calloc(tmp1.grad_size, sizeof(float));

	tmp2.data_size = layer2_weight.data_size;
	tmp2.data = (float*)calloc(tmp2.data_size, sizeof(float));
	tmp2.grad_size = layer2_weight.grad_size;
	tmp2.grad = (float*)calloc(tmp2.grad_size, sizeof(float));

	adam.lr = learning_rate;
	adam.beta1 = 0.9;
	adam.beta2 = 0.999;
	adam.eps = 1e-8;
	adam.weight_decay = weight_decay;
	adam.step_count = 0;
}

/*input->Dropout->SparseMatmul->GraphSum->ReLU->Dropout->Matmul->GraphSum->CrossEntropyLoss*/
void forward(int training) {
	int i, j, jj, k;
	float coef, scale, total_loss, *logit, max_logit, prob, sum_exp;
	int threshold, keep, count;
	if (training) {
		threshold = (int)(dropout*MY_RAND_MAX);
		scale = 1.0 / (1 - dropout);
		for (i = 0; i < input.data_size; i++) {
			keep = (int)RAND() >= threshold;
			input.data[i] *= keep ? scale : 0;
			if (maskD1)maskD1[i] = keep;
		}
	}
	for (i = 0; i < layer1_var1.data_size; i++)
		layer1_var1.data[i] = 0;
	for (i = 0; i < Data.feature_index.indptr_size - 1; i++)
		for (j = Data.feature_index.indptr[i]; j < Data.feature_index.indptr[i + 1]; j++) {
			jj = Data.feature_index.indices[j];
			for (k = 0; k < hidden_dim; k++)
				layer1_var1.data[i*hidden_dim + k] += input.data[j] * layer1_weight.data[jj*hidden_dim + k];
		}
	for (i = 0; i < layer1_var2.data_size; i++)
		layer1_var2.data[i] = 0;
	for (i = 0; i < Data.graph.indptr_size - 1; i++)
		for (j = Data.graph.indptr[i]; j < Data.graph.indptr[i + 1]; j++) {
			jj = Data.graph.indices[j];
			coef = 1.0f / sqrtf(
				(Data.graph.indptr[i + 1] - Data.graph.indptr[i])*(Data.graph.indptr[jj + 1] - Data.graph.indptr[jj])
			);
			for (k = 0; k < hidden_dim; k++)
				layer1_var2.data[i*hidden_dim + k] += coef * layer1_var1.data[jj*hidden_dim + k];
		}
	for (i = 0; i < layer1_var2.data_size; i++) {
		keep = (layer1_var2.data[i] > 0);
		if (training) maskR1[i] = keep;
		if (!keep)layer1_var2.data[i] = 0;
	}
	if (training) {
		threshold = (int)(dropout*MY_RAND_MAX);
		scale = 1 / (1 - dropout);
		for (i = 0; i < layer1_var2.data_size; i++) {
			keep = (int)RAND() >= threshold;
			layer1_var2.data[i] *= keep ? scale : 0;
			if (maskD2)maskD2[i] = keep;
		}
	}
	for (i = 0; i < layer2_var1.data_size; i++)
		layer2_var1.data[i] = 0;
	for (i = 0; i < num_nodes; i++)
		for (j = 0; j < hidden_dim; j++)
			for (k = 0; k < output_dim; k++)
				layer2_var1.data[i*output_dim + k] += layer1_var2.data[i*hidden_dim + j] * layer2_weight.data[j*output_dim + k];
	for (i = 0; i < output.data_size; i++)
		output.data[i] = 0;
	for (i = 0; i < Data.graph.indptr_size - 1; i++)
		for (j = Data.graph.indptr[i]; j < Data.graph.indptr[i + 1]; j++) {
			jj = Data.graph.indices[j];
			coef = 1.0f / sqrtf(
				(Data.graph.indptr[i + 1] - Data.graph.indptr[i])*(Data.graph.indptr[jj + 1] - Data.graph.indptr[jj])
			);
			for (k = 0; k < output_dim; k++)
				output.data[i*output_dim + k] += coef * layer2_var1.data[jj*output_dim + k];
		}
	total_loss = 0;
	count = 0;
	if (training)
		for (i = 0; i < output.grad_size; i++)
			output.grad[i] = 0.0;
	for (i = 0; i < output.data_size / output_dim; i++) {
		if (truth[i] < 0)continue;
		count++;
		logit = &(output.data[i*output_dim]);
		max_logit = -1e30f;
		sum_exp = 0.0;
		for (j = 0; j < output_dim; j++)
			max_logit = fmax(max_logit, logit[j]);
		for (j = 0; j < output_dim; j++) {
			logit[j] -= max_logit;
			sum_exp += expf(logit[j]);
		}
		total_loss += logf(sum_exp) - logit[truth[i]];
		if (training) {
			for (j = 0; j < output_dim; j++) {
				prob = expf(logit[j]) / sum_exp;
				output.grad[i*output_dim + j] = prob;
			}
			output.grad[i*output_dim + truth[i]] -= 1.0;
		}
	}
	loss = total_loss / count;
	if (training)
		for (i = 0; i < output.grad_size; i++)
			output.grad[i] /= count;
}

/*input->Dropout->SparseMatmul->GraphSum->ReLU->Dropout->Matmul->GraphSum->CrossEntropyLoss*/
void backward() {
	int i, j, jj, k;
	float coef, tmp, scale;
	for (i = 0; i < layer2_var1.grad_size; i++)
		layer2_var1.grad[i] = 0;
	for (i = 0; i < Data.graph.indptr_size - 1; i++)
		for (j = Data.graph.indptr[i]; j < Data.graph.indptr[i + 1]; j++) {
			jj = Data.graph.indices[j];
			coef = 1.0f / sqrtf(
				(Data.graph.indptr[i + 1] - Data.graph.indptr[i])*(Data.graph.indptr[jj + 1] - Data.graph.indptr[jj])
			);
			for (k = 0; k < output_dim; k++)
				layer2_var1.grad[i*output_dim + k] += coef * output.grad[jj*output_dim + k];
		}
	for (i = 0; i < layer1_var2.grad_size; i++)
		layer1_var2.grad[i] = 0;
	for (i = 0; i < layer2_weight.grad_size; i++)
		layer2_weight.grad[i] = 0;
	for (i = 0; i < num_nodes; i++)
		for (j = 0; j < hidden_dim; j++) {
			tmp = 0;
			for (k = 0; k < output_dim; k++) {
				tmp += layer2_var1.grad[i*output_dim + k] * layer2_weight.data[j*output_dim + k];
				layer2_weight.grad[j*output_dim + k] += layer2_var1.grad[i*output_dim + k] * layer1_var2.data[i*hidden_dim + j];
			}
			layer1_var2.grad[i*hidden_dim + j] = tmp;
		}
	if (maskD2) {
		scale = 1 / (1 - dropout);
		for (i = 0; i < layer1_var2.data_size; i++)
			layer1_var2.grad[i] *= maskD2[i] ? scale : 0;
	}
	for (i = 0; i < layer1_var2.data_size; i++)
		if (!maskR1[i])
			layer1_var2.grad[i] = 0;
	for (i = 0; i < layer1_var1.grad_size; i++)
		layer1_var1.grad[i] = 0;
	for (i = 0; i < Data.graph.indptr_size - 1; i++)
		for (j = Data.graph.indptr[i]; j < Data.graph.indptr[i + 1]; j++) {
			jj = Data.graph.indices[j];
			coef = 1.0 / sqrtf(
				(Data.graph.indptr[i + 1] - Data.graph.indptr[i])*(Data.graph.indptr[jj + 1] - Data.graph.indptr[jj])
			);
			for (k = 0; k < hidden_dim; k++)
				layer1_var1.grad[i*output_dim + k] += coef * layer1_var2.grad[jj*output_dim + k];
		}
	for (i = 0; i < layer1_weight.grad_size; i++)
		layer1_weight.grad[i] = 0;
	for (i = 0; i < Data.feature_index.indptr_size - 1; i++)
		for (j = Data.feature_index.indptr[i]; j < Data.feature_index.indptr[i + 1]; j++) {
			jj = Data.feature_index.indices[j];
			for (k = 0; k < hidden_dim; k++)
				layer1_weight.grad[jj*hidden_dim + k] += layer1_var1.grad[i*hidden_dim + k] * input.data[j];
		}
	if (maskD1) {
		scale = 1 / (1 - dropout);
		for (i = 0; i < input.data_size; i++)
			input.grad[i] *= maskD1[i] ? scale : 0;
	}
}

void step() {
	adam.step_count++;
	int i;
	float grad;
	float step_size = adam.lr*sqrtf(1.0f - powf(adam.beta2, adam.step_count)) / (1.0f - powf(adam.beta1, adam.step_count));
	for (i = 0; i < layer1_weight.data_size; i++) {
		grad = layer1_weight.grad[i];
		grad += adam.weight_decay*layer1_weight.data[i];
		tmp1.data[i] = adam.beta1 * tmp1.data[i] + (1.0f - adam.beta1)*grad;
		tmp1.grad[i] = adam.beta2 * tmp1.grad[i] + (1.0f - adam.beta2)*grad*grad;
		layer1_weight.data[i] -= step_size * tmp1.data[i] / (sqrtf(tmp1.grad[i]) + adam.eps);
	}
	for (i = 0; i < layer2_weight.data_size; i++) {
		grad = layer2_weight.grad[i];
		tmp2.data[i] = adam.beta1 * tmp2.data[i] + (1.0f - adam.beta1)*grad;
		tmp2.grad[i] = adam.beta2 * tmp2.grad[i] + (1.0f - adam.beta2)*grad*grad;
		layer2_weight.data[i] -= step_size * tmp2.data[i] / (sqrtf(tmp2.grad[i]) + adam.eps);
	}
}

float train_loss, train_acc;
void train_epoch() {
	int i, j;
	float l2 = 0, x = 0;
	for (i = 0; i < input.data_size; i++)
		input.data[i] = Data.feature_value[i];
	for (i = 0; i < num_nodes; i++)
		truth[i] = Data.split[i] == 1 ? Data.label[i] : -1;
	forward(1);
	for (i = 0; i < layer1_weight.data_size; i++) {
		x = layer1_weight.data[i];
		l2 += x * x;
	}
	train_loss = loss + weight_decay * l2 / 2;
	int wrong = 0, total = 0;
	float truth_logit;
	for (i = 0; i < num_nodes; i++) {
		if (truth[i] < 0)continue;
		total++;
		truth_logit = output.data[i*output_dim + truth[i]];
		for (j = 0; j < output_dim; j++)
			if (output.data[i*output_dim + j] > truth_logit) {
				wrong++;
				break;
			}
	}
	train_acc = (float)(total - wrong) / total;
	backward();
	step();
}

void eval(int current_split) {
	int i, j;
	float l2 = 0, x = 0;
	for (i = 0; i < input.data_size; i++)
		input.data[i] = Data.feature_value[i];
	for (i = 0; i < num_nodes; i++)
		truth[i] = Data.split[i] == current_split ? Data.label[i] : -1;
	forward(0);
	for (i = 0; i < layer1_weight.data_size; i++) {
		x = layer1_weight.data[i];
		l2 += x * x;
	}
	train_loss = loss + weight_decay * l2 / 2;
	int wrong = 0, total = 0;
	float truth_logit;
	for (i = 0; i < num_nodes; i++) {
		if (truth[i] < 0)continue;
		total++;
		truth_logit = output.data[i*output_dim + truth[i]];
		for (j = 0; j < output_dim; j++)
			if (output.data[i*output_dim + j] > truth_logit) {
				wrong++;
				break;
			}
	}
	train_acc = (float)(total - wrong) / total;
}

void run() {
	int epoch = 1;
	for (; epoch <= epochs; epoch++) {
		train_epoch();
		printf("epoch=%d train_loss=%.5f train_acc=%.5f\n",
			epoch, train_loss, train_acc);
		if (early_stopping > 0 && epoch >= early_stopping)
			break;
	}
	eval(3);
	printf("test_loss=%.5f test_acc=%.5f\n", train_loss, train_acc);
}

int main(int argc, char **argv) {
	FILE *fp;
	getchar();
	///*
		fopen_s(&fp,"cora", "rb");
	//int len;
	int max_idx = 0, max_label = 0, i;

	fread(&Data.feature_index.indices_size, sizeof(int), 1, fp);
	//len = Data.feature_index.indices_size;
	//printf("%d\n",len);
	Data.feature_index.indices = (int *)malloc(Data.feature_index.indices_size * sizeof(int));
	fread(Data.feature_index.indices, sizeof(int), Data.feature_index.indices_size, fp);
	for (i = 0; i < Data.feature_index.indices_size; i++)
		if (Data.feature_index.indices[i] > max_idx)
			max_idx = Data.feature_index.indices[i];
	input_dim = max_idx + 1;
	//printf("%d\n",input_dim);

	fread(&Data.feature_index.indptr_size, sizeof(int), 1, fp);
	//len = Data.feature_index.indptr_size;
	//printf("%d\n",len);
	Data.feature_index.indptr = (int *)malloc(Data.feature_index.indptr_size * sizeof(int));
	fread(Data.feature_index.indptr, sizeof(int), Data.feature_index.indptr_size, fp);

	fread(&Data.graph.indices_size, sizeof(int), 1, fp);
	//len = Data.graph.indices_size;
	//printf("%d\n",len);
	Data.graph.indices = (int *)malloc(Data.graph.indices_size * sizeof(int));
	fread(Data.graph.indices, sizeof(int), Data.graph.indices_size, fp);

	fread(&Data.graph.indptr_size, sizeof(int), 1, fp);
	num_nodes = Data.graph.indptr_size - 1;
	//len = Data.graph.indptr_size;
	//printf("%d\n",len);
	Data.graph.indptr = (int *)malloc(Data.graph.indptr_size * sizeof(int));
	fread(Data.graph.indptr, sizeof(int), Data.graph.indptr_size, fp);

	fread(&Data.split_size, sizeof(int), 1, fp);
	//len = Data.split_size;
	//printf("%d\n",len);
	Data.split = (int *)malloc(Data.split_size * sizeof(int));
	fread(Data.split, sizeof(int), Data.split_size, fp);

	fread(&Data.label_size, sizeof(int), 1, fp);
	//len = Data.label_size;
	//printf("%d\n",len);
	Data.label = (int *)malloc(Data.label_size * sizeof(int));
	fread(Data.label, sizeof(int), Data.label_size, fp);
	for (i = 0; i < Data.label_size; i++)
		if (Data.label[i] > max_label)
			max_label = Data.label[i];
	output_dim = max_label + 1;
	//printf("%d\n",output_dim);

	fread(&Data.feature_value_size, sizeof(int), 1, fp);
	//len = Data.feature_value_size;
	//printf("%d\n",len);
	Data.feature_value = (float *)malloc(Data.feature_value_size * sizeof(float));
	fread(Data.feature_value, sizeof(float), Data.feature_value_size, fp);

	fclose(fp);

	init();
	run();
	//*/
	
	return 0;
}

// 运行程序: Ctrl + F5 或调试 >“开始执行(不调试)”菜单
// 调试程序: F5 或调试 >“开始调试”菜单

// 入门提示: 
//   1. 使用解决方案资源管理器窗口添加/管理文件
//   2. 使用团队资源管理器窗口连接到源代码管理
//   3. 使用输出窗口查看生成输出和其他消息
//   4. 使用错误列表窗口查看错误
//   5. 转到“项目”>“添加新项”以创建新的代码文件，或转到“项目”>“添加现有项”以将现有代码文件添加到项目
//   6. 将来，若要再次打开此项目，请转到“文件”>“打开”>“项目”并选择 .sln 文件
