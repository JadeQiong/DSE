#pragma once
#include<iostream>
#include<vector>
class SparseIndex
{
public:
	std::vector<int> indices;
	std::vector<int> indptr;
	void print();
};

