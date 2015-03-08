#include <stdio.h>
#include <stdlib.h>
#include "magnetic_field.h"
#include "vector.h"
#include "interp2.h"

#define SIZE_R (B->nr)
#define SIZE_Z (B->nz)

int main(void) {
  vector *point = vnew(2);
  vector *field_in_point = vnew(2);

  FILE *fp;
  fp = fopen("Output_B_z_short", "w");

  //point->val[1] = 0;

  magnetic_field *B = magnetic_field_load("iter2d.bkg");

  double **B_r, **B_z;
  unsigned int i, k;

  /* Stack appears to get full when allocating B_r and B_z
     on the stack. Therefore we put them in regular memory */
  B_r = malloc(sizeof(double*)*SIZE_Z);
  B_z = malloc(sizeof(double*)*SIZE_Z);
  for (i = 0; i < SIZE_Z; i++) {
    B_r[i] = malloc(sizeof(double)*SIZE_R);
    B_z[i] = malloc(sizeof(double)*SIZE_R);
  }

  double *r_grid = magnetic_field_create_grid(SIZE_R, 3.5, 8.9);
  double *z_grid = magnetic_field_create_grid(SIZE_Z, -5.5, 5.5);

  interp2_init_interpolation(B);

  for (i=0; i < SIZE_R ; i++) { // rows
    point->val[0] = r_grid[i];
                              
    for (k=0; k < SIZE_Z ; k++) { // columns
      point->val[1] = z_grid[k];
      /* Get field value in point */
      field_in_point = magnetic_field_get(point);
      /* store r-value in B_r array */
      B_r[i][k] = field_in_point->val[1];
      /* print to file */
      fprintf(fp, "%f \t", B_r[i][k]);
    }
    fprintf(fp, "\n");
  }
  fclose(fp);

  return 0;
}
