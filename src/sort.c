#include "sort.h"
#include "array.h"

void bubblesort_triangle(triangle_t triangles[]) {
	int length = array_length(triangles);

	for (int i = 0; i < length - 1; i++) {
		for (int j = 0; j < length - 1 - i; j++) {
			if (triangles[j].avg_depth < triangles[j + 1].avg_depth) {
				triangle_t tmp = triangles[j];
				triangles[j] = triangles[j + 1];
				triangles[j+1] = tmp;
			}
		}
	}

	// printf("------------ sorted triangles\n");
	// for (int i = 0; i< length; i++) {
	// 	printf("%f\n", triangles[i].avg_depth);
	// }

}

void merge(triangle_t triangles[], int l, int m, int r) {
	int i, j, k;
	int n1 = m - l + 1; // length left
	int n2 = r - m;	// length right

	triangle_t L[n1], R[n2];

	for (i = 0; i < n1; i++) {
		L[i] = triangles[l + i];
	}
	for (j = 0; j < n2; j++) {
		R[j] = triangles[m + 1 + j];
	}

	i = 0;
	j = 0;
	k = l;
	while (i < n1 && j < n2) {
		if (L[i].avg_depth > R[j].avg_depth) {
			triangles[k] = L[i];
			i++;
		} else {
			triangles[k] = R[j];
			j++;
		}
		k++;
	}
	while (i < n1) {
		triangles[k] = L[i];
		i++;
		k++;
	}
	while (j < n2) {
		triangles[k] = R[j];
		j++;
		k++;
	}
	
}

void merge_sort(triangle_t triangles[], int l, int r) {
	if (l >= r) return;

	int m = l + (r - l) / 2; // avoid overflow

	merge_sort(triangles, l, m);
	merge_sort(triangles, m + 1, r);

	merge(triangles, l, m, r);
}

// descending avg_depth
void mergesort_triangle(triangle_t triangles[], int start_index, int end_index) {
	// end is inclusive: index within array

	merge_sort(triangles, start_index, end_index);

	// printf("------------ sorted triangles\n");
	// for (int i = 0; i< length; i++) {
	// 	printf("%f\n", triangles[i].avg_depth);
	// }
}
