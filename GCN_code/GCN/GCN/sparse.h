#pragma once
#include<iostream>
#include<vector>
class Sparse
{
public:
	std::vector<int> indices;
	std::vector<int> indptr;
	Sparse();
	~Sparse();
	void print();
};

