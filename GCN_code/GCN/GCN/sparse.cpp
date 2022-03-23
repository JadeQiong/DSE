#include "pch.h"
#include "sparse.h"


Sparse::Sparse()
{
}


Sparse::~Sparse()
{
}

void Sparse::print() {
	std::cout << "---sparse index info---\n";
	std::cout << "indptr: ";
	for (auto i : indptr) {
		std::cout << i << " ";
	}
	std::cout << "\n";
	for (auto i : indices) {
		std::cout << i << " ";
	}
	std::cout << "\n";
}