#include "axi.h"
#include "navier-stokes/centered.h"
#define FILTERED
#include "two-phaseAxiVP.h"
#include "navier-stokes/conserving.h"
#include "tension.h"
#include "reduced.h"
#include "distance.h"

// #include "adapt_wavelet_limited.h"

#define tsnap (0.001)
#define fErr (1e-3)
#define KErr (1e-3)
#define VelErr (1e-2)
#define D2Err (1e-3)

#define RHO21 (1e-3)
#define MU21 (2e-2)
#define Ldomain 8

u.n[right] = neumann(0.);
p[right] = dirichlet(0.);

int MAXlevel;
double Oh, Bond, tmax;
char nameOut[80], dumpFile[80];

int main(int argc, char const *argv[]) {
  L0 = Ldomain;
  origin(-L0/2., 0.);
  init_grid(1 << 6);

  MAXlevel = atoi(argv[1]);
  tauy = atof(argv[2]);
  Bond = atof(argv[3]);
  Oh = atof(argv[4]);
  tmax = atof(argv[5]);

  if (argc < 6){
    fprintf(ferr, "Missing command line arguments.\n");
    return 1;
  }

  fprintf(ferr, "Level %d, Oh %2.1e, Tauy %4.3f, Bo %4.3f\n", MAXlevel, Oh, tauy, Bond);

  char comm[80];
  sprintf(comm, "mkdir -p intermediate");
  system(comm);
  sprintf(dumpFile, "dump");

  mumax = 1e8 * Oh;
  rho1 = 1., rho2 = RHO21;
  mu1 = Oh, mu2 = MU21 * Oh;
  f.sigma = 1.0;
  G.x = -Bond;

  TOLERANCE = 1e-4;
  CFL = 1e-1;
  run();
}

event init(t = 0) {
  if (!restore(file = dumpFile)) {
    char filename[60];
    sprintf(filename, "Bo%5.4f.dat", Bond);
    FILE *fp = fopen(filename, "rb");
    if (fp == NULL){
      fprintf(ferr, "File not found: %s\n", filename);
      return 1;
    }
    coord* InitialShape = input_xy(fp);
    fclose(fp);

    scalar d[];
    distance(d, InitialShape);
    while (adapt_wavelet((scalar *){f, d}, (double[]){1e-8, 1e-8}, MAXlevel).nf);

    vertex scalar phi[];
    foreach_vertex() {
      phi[] = -(d[] + d[-1] + d[0,-1] + d[-1,-1]) / 4.;
    }
    fractions(phi, f);
  }
}

event adapt(i++) {
  scalar KAPPA[];
  curvature(f, KAPPA);

  adapt_wavelet((scalar *){f, u.x, u.y, KAPPA},
                (double[]){fErr, VelErr, VelErr, KErr},
                MAXlevel);
}

event writingFiles(t = 0; t += tsnap; t <= tmax) {
  dump(file = dumpFile);
  sprintf(nameOut, "intermediate/snapshot-%5.4f", t);
  dump(file = nameOut);
}

event end(t = end) {
  fprintf(ferr, "Done: Level %d, Oh %2.1e, Tauy %4.3f, Bo %4.3f\n", MAXlevel, Oh, tauy, Bond);
}

event logWriting(i++) {
  double ke = 0.;
  foreach (reduction(+:ke)) {
    ke += (2*pi*y)*(0.5*rho(f[])*(sq(u.x[]) + sq(u.y[])))*sq(Delta);
  }

  if (pid() == 0) {
    static FILE *fp;
    if (i == 0) {
      fprintf(ferr, "i dt t ke\n");
      fp = fopen("log", "w");
      fprintf(fp, "i dt t ke\n");
    } else {
      fp = fopen("log", "a");
    }
    fprintf(fp, "%d %g %g %g\n", i, dt, t, ke);
    fclose(fp);
    fprintf(ferr, "%d %g %g %g\n", i, dt, t, ke);
  }

  if (ke > 1e3 || ke < 1e-6) {
    if (i > 1e2)
      return 1;
  }
}
