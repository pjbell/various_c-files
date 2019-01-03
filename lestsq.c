


/*
   Cartesian least-squares regression analysis
   read data points <x> <y> from stdin and print best-fit line parameters
   m, b, and rho for the equation y = mx + b,  where rho=correlation coef
*/
#define  X  0
#define  Y  1
#define MAXPOINTS 100

#include <stdio.h>
#include <math.h>

main()
{
   void lsq() ;	/* least-squares analysis routine */

   float x[MAXPOINTS] ;   /* x values */
   float y[MAXPOINTS] ;   /* y values */
   float *data[2] ;       /* 2-d array */

   char dpoint[80] ;      /* input buffer */
   int i ;                /* data-point counter */

   float m, b, rho ;   /* slope, intercept, correlation */

   /* set up 2-dimensional data array */
   data[X] = x ;
   data[Y] = y ;

   /* read data points from stdin */
   i = 0 ;
   printf("input data points <x> <y>, terminate with empty line\n") ;
   do
   {
      fgets(dpoint,80,stdin) ;
      if (*dpoint != '\n')
      {
         sscanf(dpoint," %f %f ",&data[X][i],&data[Y][i]) ;
         i++ ;
      }
   }
   while (*dpoint != '\n') ;

   printf("%d data points read\n",i) ;

   /* get best-fit line parameters */
   lsq(data,i,&m,&b,&rho) ;

   printf("\nslope: %f    intercept: %f    correlation: %f\n",m,b,rho) ;
}


/* lsq computes (using the least-squares technique) the best-fit straight
   line through a set of Cartesian data points
*/
void lsq(data,npoints,m,b,rho)
float *data[] ;   /* array of Cartesian data points (x,y) */
int npoints ;     /* number of data points */
float *m ;        /* returned best-fit slope */
float *b ;        /* returned best-fit y-intercept */
float *rho ;      /* returned correlation coefficient */
{
   float xbar, ybar, cvar, varx, vary ;
   int i ;

   /* find mean of each variable */
   ybar = 0 ;
   xbar = 0 ;
   for (i = 0 ; i < npoints ; i++)
   {
      ybar += data[Y][i] ;
      xbar += data[X][i] ;
   }
   ybar = ybar/npoints ;
   xbar = xbar/npoints ;

   /* find covariance */
   cvar = 0 ;
   for (i = 0 ; i < npoints ; i++)
      cvar += (data[Y][i] - ybar) * (data[X][i] - xbar) ;
   cvar = cvar/(npoints - 1) ;

   /* find variance of each variable */
   varx = 0 ;
   vary = 0 ;
   for (i = 0 ; i < npoints ; i++)
   {
      varx += (data[X][i] - xbar) * (data[X][i] - xbar) ;
      vary += (data[Y][i] - ybar) * (data[Y][i] - ybar) ;
   }
   varx = varx/(npoints - 1) ;
   vary = vary/(npoints - 1) ;

   /* find slope (m) and y-intercept (b) */
   *m = cvar/varx ;
   *b = ybar - *m * xbar ;

   /* find correlation coefficient */
   *rho = cvar/(sqrt(varx)*sqrt(vary)) ;

   /* that's it! */
}






