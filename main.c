#include <stdlib.h>
#include <stdio.h>
#include <math.h>

typedef struct vector_t {
	float x;
	float y;
	float z;
} vector_t;

typedef struct vector_container_t {
	vector_t* vector_array;
	size_t num_elements;
} vector_container_t;

vector_t create_vector(float x, float y, float z) {
	vector_t v;
	v.x = x;
	v.y = y;
	v.z = z;
	return v;
}
#define zero_vector create_vector(0.0, 0.0, 0.0);
#define unit_vector create_vector(1.0, 1.0, 1.0);
#define scale_vector(a, v) create_vector(a * v.x, a * v.y, a * v.z);

void write_vector(vector_t v, FILE* f) {
	fprintf(f, "%f,%f,%f\n", v.x, v.y, v.z);
}

void store_vector_container(vector_container_t vc, char* path) {
	int current_vector;
	FILE* f = fopen(path, "w");
	fprintf(f, "x,y,z\n");
	for(current_vector = 0; current_vector < vc.num_elements; current_vector++) {
		write_vector(vc.vector_array[current_vector], f);
	}
	fclose(f);
}

void set_initial_positions(vector_container_t vc, size_t cube_length) {
	int i,j,k;
	int current_vector = 0;
	float h = 1.0 / (cube_length - 1);
	for(i = 0; i < cube_length; i++) {
		for(j = 0; j < cube_length; j++) {
			for(k = 0; k < cube_length; k++) {
				vc.vector_array[current_vector] = create_vector(h * i, h * j, h * k);
				current_vector++;
			}
		}
	}
}

vector_container_t allocate_vectors(size_t num_elements) {
	vector_container_t vc;
	vc.vector_array = (vector_t *)malloc(sizeof(vector_t) * num_elements);
	vc.num_elements = num_elements;
	return vc;
}

typedef struct config_t {
	int cube_size;
	int num_elements;
} config_t;

config_t read_config(int argc, char** argv) {
	config_t c;
	c.cube_size = 10;
	c.num_elements = 1000;
	return c;
}

int main(int argc, char** argv) {
	config_t conf = read_config(argc, argv);
	vector_container_t vc = allocate_vectors(conf.num_elements);
	set_initial_positions(vc, conf.cube_size);
	store_vector_container(vc, "initial_positions.csv");
	return 0;
}
