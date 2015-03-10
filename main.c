#include <stdlib.h>
#include <stdio.h>
#include <math.h>

typedef struct particle_t {
  float x;
  float y;
  float z;
} particle_t;

typedef struct particle_container_t {
  particle_t* particle_array;
  size_t num_elements;
} particle_container_t;

particle_t create_particle(float x, float y, float z) {
  particle_t p;
  p.x = x;
  p.y = y;
  p.z = z;
  return p;
}

void set_initial_conditions(particle_container_t pc, size_t cube_length) {
  int i,j,k;
  int current_particle = 0;
  float h = 1.0 / cube_length;
  for(i = 0; i < cube_length; i++) {
    for(j = 0; j < cube_length; j++) {
      for(k = 0; k < cube_length; k++) {
        pc.particle_array[current_particle] = create_particle(h * i, h * j, h * k);
        current_particle++;
      }
    }
  }
}

#define CUBED(X) X * X * X
// Allocates cube_length^3 particles along the unit cube
particle_container_t allocate_particles(size_t cube_length) {
  particle_container_t pc;
  pc.particle_array = (particle_t *)malloc(sizeof(particle_t) * CUBED(cube_length));
  pc.num_elements = CUBED(cube_length);
  return pc;
}

void save_particle_to_file(particle_t p, FILE* f) {
  fprintf(f, "%f,%f,%f\n", p.x, p.y, p.z);
}

void save_particles_to_file(particle_container_t pc, FILE* f) {
  fprintf(f, "x,y,z\n");
  int current_particle;
  for(current_particle = 0; current_particle < pc.num_elements; current_particle++) {
    save_particle_to_file(pc.particle_array[current_particle], f);
  }
}

int main(int argc, char** argv) {
  particle_container_t pc = allocate_particles(10);
  set_initial_conditions(pc, 10);
  FILE* f = fopen("sph_output.csv","w");
  save_particles_to_file(pc, f);
  return 0;
}
