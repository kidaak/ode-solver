#ifndef _ARGUMENTS_H
#define _ARGUMENTS_H

typedef struct {
	double tstart, tend;	/* Start and end times */
	double *r0, *v0;		/* Initial position and velocity */
	char *magfield_file,	/* Name of magnetic field file */
		 *domain_file,		/* Name of domain file */
		 *output_file;		/* Name of output file */
	double particle_mass;	/* Mass of particle */
	double particle_charge;	/* Charge of particle */
	char print_settings;	/* Wether or not to print settings at top
							   of output file */
	char problem;			/* Which problem to solve (GC or PC) */
} arguments;

#define PROBLEM_GC 1		/* Solve the guiding center problem */
#define PROBLEM_NO_GC 0 	/* Solve the particle motion problem */

double *atodp3(const char*);
arguments *arguments_default(void);
arguments *parse_args(int, char*[]);

#endif/*_ARGUMENTS_H*/
