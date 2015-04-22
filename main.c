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

vector_t scale_vector(float a, vector_t v) {
	return create_vector(a * v.x, a * v.y, a * v.z);
}

vector_t add_vectors(vector_t v, vector_t w) {
	return create_vector(v.x + w.x, v.y + w.y, v.z + w.z);
}

vector_t subtract_vectors(vector_t v, vector_t w) {
	return add_vectors(v, scale_vector(-1, w));
}

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
	int solverIterations;
} config_t;

config_t read_config(int argc, char** argv) {
	config_t c;
	c.cube_size = 2;
	c.num_elements = 8;
	c.timestep = 0.1;
	c.solverIterations = 100;
	return c;
}

void set_initial_velocities(vector_container_t vc) {
	int current_vector;
	vector_t one = unit_vector;
	vector_t deltaX = scale_vector(0.00, one);
	for(current_vector = 0; current_vector < vc.num_elements; current_vector++) {
		vc.vector_array[current_vector] = deltaX;
	}
}

#define GRAVITY 9.8
#define PARTICLE_MASS 1.0
void apply_forces(float timestep, vector_container_t velocities) {
	size_t index;
	for(index = 0; index < velocities.num_elements; index++) {
		// Line 2: Apply Forces
		vector_t vi = velocities.vector_array[index];
		vector_t f = create_vector(0,0,-1 * GRAVITY * PARTICLE_MASS);
		vector_t dt_times_f = scale_vector(timestep, f);
		velocities.vector_array[index] = add_vectors(vi, dt_times_f);
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

vector_t compute_location_vector(vector_t v) {
	float norm = vector_norm(v);
	return scale_vector(1.0/norm,v);
}

/* This comes from Mueller Equation 16 */
vector_t calculate_vorticity(float epsilon, vector_t N, vector_t omega) {
	vector_t n_cross_omega = cross_product(N, omega);
	return scale_vector(epsilon, n_cross_omega);
}

float calculate_distance(vector_t u, vector_t v) {
	return vector_norm(subtract_vectors(u,v));
}

// Equation 21 from Muller, Charypar, and Gross 2003
#define PI 3.1415
float W_spiky(vector_t r, float h) {
	float r_norm = vector_norm(r);
	if (0 <= r_norm && r_norm <= h) {
		float h_to_the_6th = h * h * h * h * h * h;
		float coefficient = 15.0 / (PI * h_to_the_6th);

		float diff = h - r_norm;
		float diff_cubed = diff * diff * diff;
	
		return coefficient * diff_cubed;
	} else {
		return 0.0;
	}
}

float W_poly6(vector_t r, float h) {
	float r_norm = vector_norm(r);
	if (0 <= r_norm && r_norm <= h) {
		float h_to_the_9th = h * h * h * h * h * h * h * h * h;
		float coefficient = 315.0 / (64.0 * PI * h_to_the_9th);
		
		float diff = (h * h) - (r_norm * r_norm);
		float diff_cubed = diff * diff * diff;

		return coefficient * diff_cubed;
	} else {
		return 0.0;
	}
}

float calculate_rho(size_t i, float h, vector_container_t positions) {
	float rho = 0.0;
	vector_t pi = positions.vector_array[i];
	int j = 0;
	for(j = 0; j < positions.num_elements; j++) {
		vector_t pj = positions.vector_array[j];
		rho += W(vector_subtract(pi, pj), h);
	}
	return rho;
}

# define rho_0 1.0
float calculate_constraints(size_t i, float h, vector_container_t positions) {
	float rho_i = calculate_rho(i, h, positions);
	return rho_i / rho_0 - 1.0;
}

void calculate_lambdas(float* lambdas, vector_container_t positions, vector_container_t predicted_positions, config_t conf) {
	int i = 0;
	for(i = 0; i < conf.num_elements; i++) {
		float numerator = calculate_constraints(i, conf.timestep, positions);
		float denominator = 1.0;
		lambdas[i] = - numerator / denominator;
	}
}

void simulation_loop(config_t conf, vector_container_t positions, vector_container_t velocities) {
	vector_container_t predicted_positions = allocate_vectors(conf.num_elements);

	// Line 2
	apply_forces(conf.timestep, velocities);
	// Line 3
	predict_positions(conf.timestep, predicted_positions, positions, velocities);

	int i = 0;
	float* lambdas = (float*)malloc(sizeof(float) * conf.num_elements);
	for(i = 0; i < conf.solverIterations; i++) {
		calculate_lambas(lambdas, positions, predicted_positions, conf);
	}
}

int main(int argc, char** argv) {
	config_t conf = read_config(argc, argv);

	vector_container_t positions = allocate_vectors(conf.num_elements);
	set_initial_positions(positions, conf.cube_size);
	store_vector_container(positions, "initial_positions.csv");

	vector_container_t velocities = allocate_vectors(conf.num_elements);
	set_initial_velocities(velocities);

	apply_forces(conf.timestep, velocities);

	vector_container_t predicted_positions = allocate_vectors(conf.num_elements);
	predict_positions(conf.timestep, predicted_positions, positions, velocities);
	store_vector_container(predicted_positions, "predicted_positions.csv");
	return 0;
}
