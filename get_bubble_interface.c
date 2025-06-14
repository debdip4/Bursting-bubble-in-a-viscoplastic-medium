/* file: get_bubble_interface.c */
#include "navier-stokes/centered.h"
#include "fractions.h"

scalar f[];
char filename[80];

int main(int a, char const *arguments[]) {
  sprintf (filename, "%s", arguments[1]);
  restore (file = filename);
  
  // Set the boundary condition for the interface fraction 'f'
  // This needs to match the main simulation
  f[left] = dirichlet(1.0); // Liquid on the axis for bursting bubble
  
  boundary((scalar *){f});

  // Print the interface facet coordinates to standard output
  output_facets(f, stdout);
  
  return 0;
}