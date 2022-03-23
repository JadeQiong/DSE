#include "pch.h"
#include "sparse.h"


void SparseIndex::print() {
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