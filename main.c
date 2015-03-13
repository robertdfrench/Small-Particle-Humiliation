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
#define zero_vector create_vector(0.0, 0.0, 0.0)
#define unit_vector create_vector(1.0, 1.0, 1.0)
#define scale_vector(a, v) create_vector(a * v.x, a * v.y, a * v.z)
#define add_vectors(v, w) create_vector(v.x + w.x, v.y + w.y, v.z + w.z)

vector_t cross_product(vector_t u, vector_t v) {
	float x = u.y * v.z - u.z * v.y;
	float y = u.z * v.x - u.x * v.z;
	float z = u.x * v.y - u.y * v.x;
	return create_vector(x, y, z);
}

float dot_product(vector_t u, vector_t v) {
	return u.x * v.x + u.y * v.y + u.z * v.z;
}

float vector_norm(vector_t u) {
	return sqrt(dot_product(u,u));
}

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
	float timestep;
} config_t;

config_t read_config(int argc, char** argv) {
	config_t c;
	c.cube_size = 10;
	c.num_elements = 1000;
	c.timestep = 1;
	return c;
}

void set_initial_velocities(vector_container_t vc) {
	int current_vector;
	vector_t one = unit_vector;
	vector_t deltaX = scale_vector(0.01, one);
	for(current_vector = 0; current_vector < vc.num_elements; current_vector++) {
		vc.vector_array[current_vector] = deltaX;
	}
}

void predict_positions(float timestep, vector_container_t predicted_positions, vector_container_t current_positions, vector_container_t current_velocities) {
	int current_vector;
	vector_t position;
	vector_t velocity;
	vector_t predicted_position;
	vector_t change_in_position;
	for(current_vector = 0; current_vector < predicted_positions.num_elements; current_vector++) {
		position = current_positions.vector_array[current_vector];
		velocity = current_velocities.vector_array[current_vector];

		change_in_position = scale_vector(timestep, velocity);
		predicted_position = add_vectors(position, change_in_position);

		predicted_positions.vector_array[current_vector] = predicted_position;
	}
}

#define CUBE_LENGTH 10
#define NUM_PARTICLES CUBE_LENGTH * CUBE_LENGTH * CUBE_LENGTH 
int main(int argc, char** argv) {
	config_t conf = read_config(argc, argv);

	vector_container_t positions = allocate_vectors(conf.num_elements);
	set_initial_positions(positions, conf.cube_size);
	store_vector_container(positions, "initial_positions.csv");

	vector_container_t velocities = allocate_vectors(conf.num_elements);
	set_initial_velocities(velocities);
	store_vector_container(velocities, "initial_velocities.csv");

	vector_container_t predicted_positions = allocate_vectors(conf.num_elements);
	predict_positions(conf.timestep, predicted_positions, positions, velocities);
	store_vector_container(predicted_positions, "predicted_positions.csv");
	return 0;
}
