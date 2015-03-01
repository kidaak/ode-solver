/* Interpolation WORK IN PROGRESS
 *
 * TODO: create function with r_grid, z_grid, B, and vector
 * xyz (cartesian coordinates) as argument. Call this from
 * magnetic_field_get in magnetic_field.c
 *
 * ALSO: make interp2.h file
 */

/* GNU Scientific Library */
#include <gsl/gsl_errno.h>
#include <gsl/gsl_ieee_utils.h>
/* Interp2d header */
#include <interp2d.h>
/* Interp2d spline support */
#include <interp2d_spline.h>

/* GSL accelerators that help speed up interpolations */
gsl_interp_accel *ra, *za;
/* interp2d interpolation objects */
interp2d *interp;
interp2d_spline *interp_s;

/* Our test data arrays. xarr and yarr are just coordinates
   and zarr specify the value. Note that zarr has N^2 values
   (if xarr and yarr each has N points) but is still an array!
   This is exactly what we want, since it's how we planned :-)
*/
double xarr[] = {0.0,1.0,2.0,3.0}; // remove 
double yarr[] = {0.0,1.0,2.0,3.0};
double zarr[] = {1.0,1.1,1.2,1.3,
                 1.1,1.2,1.3,1.4,
                 1.2,1.3,1.4,1.5,
                 1.3,1.4,1.5,1.6};


size_t r_size  = sizeof(r_grid) / sizeof(r_grid[0]);
size_t z_size  = sizeof(z_grid) / sizeof(z_grid[0]);

/* A simple test of interpolation. Note the system must be initialized first! */
void test(double x0, double y0) {
  /* Get the corresponding value at this point */
  double z0 = interp2d_eval(interp, xarr, yarr, zarr, x0, y0, xa, ya);
	
  /* Print the result */
  printf("f(%f,%f) = %f\n", x0, y0, z0);
}

/* Initialization. This is to prepare GSL for what's to come  OK I THINK*/
void init(void) {
  /* Prepare the accelerators for both X and Y */
  ra = gsl_interp_accel_alloc();
  za = gsl_interp_accel_alloc();
  /* Create interpolation objects */
  interp = interp2d_alloc(interp2d_bicubic, r_size, z_size);
  interp_s = interp2d_spline_alloc(interp2d_bicubic, r_size, z_size);

  /* Prepare the interpolation objects for our situation */
  /* B_r */
  interp2d_init(interp, r_grid, z_grid, zarr, r_size, z_size);
  interp2d_spline_init(interp_s, r_grid, z_grid, B->B_r, r_size, z_size);
  /* B_phi */
  interp2d_init(interp, r_grid, z_grid, zarr, r_size, z_size);
  interp2d_spline_init(interp_s, r_grid, z_grid, B->B_phi, r_size, z_size);
  /* B_z */
  interp2d_init(interp, r_grid, z_grid, zarr, r_size, z_size);
  interp2d_spline_init(interp_s, r_grid, z_grid, B->B_z, r_size, z_size);
}
int main(void) {
  init();
  test(2.0, 3.0);
  test(0.0, 2.0);
  test(1.0, 0.0);

  return 0;
}
