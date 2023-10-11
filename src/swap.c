#include "swap.h"

void swap_int(int* a, int* b) {
	int tmp = *a;
	*a = *b;
	*b = tmp;
}

void swap_float(float* a, float* b) {
	float tmp = *a;
	*a = *b;
	*b = tmp;
}