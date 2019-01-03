/*	General purpose queueing model  		*/
/*	Written by: pj bell						*/
/*	version one in fortran in 1971			*/
/*	version two in basic in 1987			*/
/*  version three in 'c' in 1998            */

#include <stdio.h>
#include <math.h>

char buf[128], r[128], variable[128];
char vv;

int k, i, l;

float X1, X2, X3;
float A1, A2, A3;
float C1, C2, C3;
float T1, T2, T3;
float S1, S2, S3;
float c9, h, a, n, b, p, c;
float s9, v, u, s, x, t;

double fabs(double x);
int get_arguments();
float compute();

main(argc, argv)
int argc;
char *argv[];

{
	A1=0.0; 
	A2=0.0; 
	A3=0.0;
	C1=0.0; 
	C2=0.0; 
	C3=0.0;
	T1=0.0; 
	T2=0.0; 
	T3=0.0;
	S1=0.0; 
	S2=0.0; 
	S3=0.0;

	fprintf(stderr, "Input Parameters in any order('cr' to complete input, 'q' to quit)\n");
	fprintf(stderr, "(Parameter = starting value, ending value, increment value)\n");
	fprintf(stderr, "c = Call volume (Hourly)\n");
	fprintf(stderr, "a = Agents\n");
	fprintf(stderr, "t = Service Standard\n");
	fprintf(stderr, "s = Holding Time\n");

	for (;;) {
		while (get_arguments() == 0){
			;
			vv = variable[0];
			switch (vv) {
			case 'a' :
				A1 = fabs(X1); 
				A2 = fabs(X2); 
				A3 = fabs(X3);
				break;
			case 'c' :
				C1 = fabs(X1); 
				C2 = fabs(X2); 
				C3 = fabs(X3);
				break;
			case 't' :
				T1 = fabs(X1); 
				T2 = fabs(X2); 
				T3 = fabs(X3);
				break;
			case 's' :
				S1 = fabs(X1); 
				S2 = fabs(X2); 
				S3 = fabs(X3);
				break;
			default:
				fprintf(stderr, "\nActually, only c-a-t-s & q are allowed as variables. Try again.\n\n");
				break;
			}

		}
		compute();
	}
}
float compute()
{
	/*			Main Line		*/
	for (t = T1; t <= T2; t += T3) {				/* time standard loop	*/

		fprintf(stdout, "Calls/	Holding		Agents	Occu.	%%In %.0f	Mean Waiting\n", t);
		fprintf(stdout, " Hour	Time (sec)                %%     seconds   Time (sec)\n", t);
		fprintf(stdout, "------	---------	------	-----	-------	 ------------\n", t);

		for ( c9 = C1; c9 <= C2; c9 += C3) {		/* call volume loop		*/
			for ( h = S1; h <= S2; h += S3) {   	/* holding time loop	*/
				for ( a = A1; a <= A2; a += A3) {	/* agent loop			*/
					p = 0.0;
					c = c9/3600;
					s9 = 1; 
					x =1;
					v = c * h;
					u = v / a;
					if ( u+.000001 >= 1 ) {
						fprintf(stdout, "  %3.0f	  %3.0f    	%4.0f 	>100%%  Undefined when Occupancy >= 100%%\n",
						    c9, h, a);
					}
					else {
						for ( n = 1; n <= a; ++n) {
							x = x / n * v;
							s9 = s9 + x;
							for (;;) {		/* scaling to prevent overflow	*/
								if ( s9 >= 1E+08 ) {
/* code folded from here */
	s9 /= 10;
	x /= 10;
/* unfolding */
								}
								else {
/* code folded from here */
	break;
/* unfolding */
								}
							}
						}

						s = 1 - x / s9;
						b = (1 - s) / (1 - u * s);
						p = b * exp(-t * ( a / h - c));
						fprintf(stdout, "  %3.0f	  %3.0f    	%4.0f	%3.1f	%3.2f	    %.1f    \n",\
	
						    c9, h, a, 100*u, (100*(1-p)), (b/a*h/(1-u)));
					}

					if (A3 == 0) {
						break;
					}
				}
				if (S3 == 0) {
					break;
				}
			}
			fprintf(stdout, "\n");

			if (C3 == 0) {
				break;
			}
		}
		fprintf(stdout, "\n");
		if (T3 == 0) {
			break;
		}
	}
	fprintf(stdout, "Done.\n\n");
	return(0);
}

int get_arguments()		/* get and parse the arguments	*/
{

	char c, *m;
	int j, l;

	fflush(stderr);
	m = buf;
	for (l = 0; l <=  127; ++l){	/* smear nulls throughout input buffer	*/
		*m++ = '\0';
	}

	fprintf(stdout, "? ");
	fflush(stdout);

	j = 0;
	while ((c = gnc()) != EOF) {
		buf[j++] = c;
		if ( c == '\n') {
			break;
		}

	}

	X1 = 0.0; 
	X2 = 0.0; 
	X3 = 0.0;
	k=0; 
	i=0;

	if ( buf[0] == 'q' || buf[0] == 'Q' ){
		exit(0);
	}

	if ( buf[0] == '\n' ){
		return (1);
	}

	/* get the parameter name	*/

	while (buf[k] != '=' ){
		;

		if (isupper (buf[k])) {
			tolower (buf[k]);
		}

		variable[i] = buf[k];
		k++; 
		i++;
		if ( k >= 128 ) {
			fprintf(stderr, "\nEnough Already! Without an '=' I can't figure out what you mean.\n\n");
			return(0);
		}
	}

	variable[i+1] = '\0';

	/* got the name, now get the comma-seperated arguments */
	clear_r();
	k++; 
	i = 0;
	while (buf[k] != ',' && buf[k] != '\0' ){
		;
		r[i] = buf[k];
		k++; 
		i++;
	}
	sscanf(r, "%f", &X1);	/* got the 1st, now convert it  */
	/* actually, the 2nd and/or 3rd arguments may be omitted */
	clear_r();
	k++; 
	i =0;
	while (buf[k] != ',' && buf[k] != '\0' ){
		;
		r[i] = buf[k];
		k++; 
		i++;
	}
	sscanf(r, "%f", &X2);	/* got the 2nd, now convert it  */

	clear_r();
	k++; 
	i =0;
	while (buf[k] != ',' && buf[k] != '\0' ){
		;
		r[i] = buf[k];
		k++; 
		i++;
	}
	sscanf(r, "%f", &X3);	/* got the 3rd, now convert it  */

	/* if there's no ending value make it the same as the starting value */
	if ( X2 == 0.0 ){
		X2 = X1;
	}

	/* fiddle with the increment value  */
	if ( X2 != X1 && X3==0 ) {
		X3 = 1;
	}

	return (0);
}
clear_r()
{

	unsigned char *m;
	int l;

	m = r;
	for (l = 0; l<= 127; ++l){
		*m++ = '\0';
	}
	return;
}
gnc()		/* get next character (surely there's a better way)	*/
{
	unsigned char c;
	return (read (0, &c, 1) == 1) ? (unsigned char) c : EOF;
}
