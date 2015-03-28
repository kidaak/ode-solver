/* Interpolation of magnetic_field object
 * input given in cylindrical coordinates
 */
#include "magnetic_field.h"
#include "vector.h"
/* GNU Scientific Library */
#include <gsl/gsl_errno.h>
#include <gsl/gsl_ieee_utils.h>
/* Interp2d header */
#include <interp2d.h>
/* Interp2d spline support */
#include <interp2d_spline.h>
#include <math.h>

/* GSL accelerators that help speed up interpolations */
gsl_interp_accel *ra, *za;
/* interp2d interpolation objects */
interp2d_spline *Br;
interp2d_spline *Bphi;
interp2d_spline *Bz;

double rmin, rmax, zmin, zmax;

/**
 * Initializatlize magnetic_field for interpolation.
 * This is to prepare GSL for what's to come.
 * Must be called before interpolation!
 */
void interp2_init_interpolation(magnetic_field *B) {
  size_t r_size = B->nr;
  size_t z_size = B->nz;
  /* Prepare the accelerators for both r and z */
  ra = gsl_interp_accel_alloc();
  za = gsl_interp_accel_alloc();
  /* Create interpolation objects */
  Br = interp2d_spline_alloc(interp2d_bicubic, z_size, r_size);
  Bphi = interp2d_spline_alloc(interp2d_bicubic, z_size, r_size);
  Bz = interp2d_spline_alloc(interp2d_bicubic, z_size, r_size);
  /* Prepare the interpolation objects for our situation */
  /* B_r */
  interp2d_spline_init(Br, B->z_grid, B->r_grid, B->B_r, z_size, r_size);
  /* B_phi */
  interp2d_spline_init(Bphi, B->z_grid, B->r_grid, B->B_phi, z_size, r_size);
  /* B_z */
  interp2d_spline_init(Bz, B->z_grid, B->r_grid, B->B_z, z_size, r_size);

  rmin = B->rmin;
  rmax = B->rmax;
  zmin = B->zmin;
  zmax = B->zmax;
}

/**
 * main interpolation function
 *
 * xyz: The point (in cartesian coordinates) in which the field
 * strength should be evaluated.
 */
vector* interp2_interpolate(vector *xyz) {
  /*
   * cylindrical, for testing purposes
   double r = xyz->val[0]; 
   double z = xyz->val[1]; */
  
 	double x = xyz->val[0],
    y = xyz->val[1],
    z = xyz->val[2];
  /* Transform vector coordinates from cartesian to cylindrical */
	double  r = sqrt(x*x + y*y);
	
	/* Make sure the point is within our definied are */
	if (r < rmin) r = rmin;
	else if (r > rmax) r = rmax;

	if (z < zmin) z = zmin;
	else if (z > zmax) z = zmax;
  /*
   * Interpolate
   */
  double B_r_interp = interp2d_spline_eval(Br, z, r, za, ra);
  double B_phi_interp = interp2d_spline_eval(Bphi, z, r, za, ra);
  double B_z_interp = interp2d_spline_eval(Bz, z, r, za, ra);
  /*
   * Transform field to cartesian coordinates
   */
  double sinp = y/r, cosp = x/r;
  double B_x_interp = B_r_interp * cosp - B_phi_interp * sinp;
  double B_y_interp = B_r_interp * sinp + B_phi_interp * cosp; 
  /*
   * Store interpolation values in vector
   */
  vector *B_interp = vinit(3, B_x_interp, B_y_interp, B_z_interp);
 
  return B_interp;
}

/**
 * Calculate the Jacobian of the B-field
 * at the given point
 *
 * xyz: The point (in cartesian coordinates) in which the field
 * strength should be evaluated.
 */
double **interp2_jacobian(vector *xyz) {
 	double x = xyz->val[0],
    y = xyz->val[1],
    z = xyz->val[2];

	/* Transform vector coordinates from cartesian to cylindrical */
	double  r = sqrt(x*x + y*y);
	
	/* Make sure the point is within our definied are */
	if (r < rmin) r = rmin;
	else if (r > rmax) r = rmax;

	if (z < zmin) z = zmin;
	else if (z > zmax) z = zmax;

	/* Get magnetic field for future transform use */
	vector *B = magnetic_field_get(xyz);

	double cylJ[3][2] = {{0.,0.}, {0.,0.}, {0.,0.}};
	/* dBr/dr */
	cylJ[0][0] = interp2d_spline_eval_deriv_y(Br, z, r, za, ra);
	/* dBr/dz */
	cylJ[0][1] = interp2d_spline_eval_deriv_x(Br, z, r, za, ra);
	/* dB0/dr */
	cylJ[1][0] = interp2d_spline_eval_deriv_y(Bphi, z, r, za, ra);
	/* dB0/dz */
	cylJ[1][1] = interp2d_spline_eval_deriv_x(Bphi, z, r, za, ra);
	/* dBz/dr (probably not used) */
	cylJ[2][0] = interp2d_spline_eval_deriv_y(Bz, z, r, za, ra);
	/* dBz/dz */
	cylJ[2][1] = interp2d_spline_eval_deriv_x(Bz, z, r, za, ra);

	double **jacobian = malloc(3*sizeof(double*));
	jacobian[0] = malloc(3*sizeof(double));
	jacobian[1] = malloc(3*sizeof(double));
	jacobian[2] = malloc(3*sizeof(double));

	double y_div_r = y/r;	/* Cache this value for faster computations */

	double
		dr_dx = x/r,  dr_dy = y_div_r,
		/* Partial derivatives of cylindrical components */
		dBr_dx = cylJ[0][0]*dr_dx, dBr_dy = cylJ[0][0]*dr_dy,
		dB0_dx = cylJ[1][0]*dr_dx, dB0_dy = cylJ[1][0]*dr_dy,
		/* Make B components more readable */
		Br = B->val[0], B0 = B->val[1], Bz = B->val[2],
		/* Cache jacobian component factors */
		sin0 = y_div_r, cos0 = sqrt(1-y_div_r*y_div_r),
		dsin0_dx = -y_div_r*x/(r*r), dcos0_dx = (y_div_r*y_div_r)/(x*x*r),
		dsin0_dy = 0, dcos0_dy = 0;

	jacobian[0][0] = dBr_dx*cos0 + Br*dcos0_dx - dB0_dx*sin0 - B0*dsin0_dx;
	jacobian[0][1] = dBr_dy*cos0 + Br*dcos0_dy - dB0_dy*sin0 - B0*dsin0_dy;
	jacobian[0][2] = cylJ[0][1]*cos0 - cylJ[1][1]*sin0;
	jacobian[1][0] = dBr_dx*sin0 + Br*dsin0_dx + dB0_dx*cos0 - B0*dcos0_dx;
	jacobian[1][1] = dBr_dy*sin0 + Br*dsin0_dy + dB0_dy*cos0 - B0*dcos0_dy;
	jacobian[1][2] = cylJ[0][1]*sin0 + cylJ[1][1]*cos0;
	jacobian[2][0] = cylJ[2][0]*dr_dx;
	jacobian[2][1] = cylJ[2][1]*dr_dy;
	jacobian[2][2] = cylJ[2][1];

	return jacobian;
}
