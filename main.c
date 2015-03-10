#include <stdlib.h>
#include <stdio.h>
#include <math.h>

typedef struct vector_t {
  float x;
  float y;
  float z;
} vector_t;

typedef struct vector_container_t {
  vector_t* particle_array;
  size_t num_elements;
} vector_container_t;

vector_t create_vector(float x, float y, float z) {
  vector_t p;
  p.x = x;
  p.y = y;
  p.z = z;
  return p;
}

void set_initial_positions(vector_container_t pc, size_t cube_length) {
  int i,j,k;
  int current_particle = 0;
  float h = 1.0 / (cube_length - 1);
  for(i = 0; i < cube_length; i++) {
    for(j = 0; j < cube_length; j++) {
      for(k = 0; k < cube_length; k++) {
        pc.particle_array[current_particle] = create_vector(h * i, h * j, h * k);
        current_particle++;
      }
    }
  }
}

vector_container_t allocate_vectors(size_t num_elements) {
  vector_container_t vc;
  vc.particle_array = (vector_t *)malloc(sizeof(vector_t) * num_elements);
  vc.num_elements = num_elements;
  return vc;
}

void save_vector_to_file(vector_t p, FILE* f) {
  fprintf(f, "%f,%f,%f\n", p.x, p.y, p.z);
}

void save_vectors_to_file(vector_container_t pc, FILE* f) {
  fprintf(f, "x,y,z\n");
  int current_particle;
  for(current_particle = 0; current_particle < pc.num_elements; current_particle++) {
    save_vector_to_file(pc.particle_array[current_particle], f);
  }
}

#define CUBE_LENGTH 10
#define NUM_PARTICLES CUBE_LENGTH * CUBE_LENGTH * CUBE_LENGTH 
int main(int argc, char** argv) {
  vector_container_t positions = allocate_vectors(NUM_PARTICLES);
  set_initial_positions(positions, CUBE_LENGTH);
  FILE* f = fopen("sph_output.csv","w");
  save_vectors_to_file(positions, f);
  return 0;
}
