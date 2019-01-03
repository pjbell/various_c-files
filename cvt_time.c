/*		Convert a UNIX time value of type 'long' to a 26 byte string */
/*		pjbell 26 April, 1993	*/

#include <stdio.h>
#include <time.h>
#include <stdlib.h>

#define BUFFER_SIZE 128

main(argc, argv)
    char *argv[];
{
   char timeString[BUFFER_SIZE];
   long clock;

    if(argc < 2) {
        fprintf(stderr, "usage: Supply Numeric Time Value\n");
        exit(1);
	}

	clock = atol (argv[1]);

    (void) strcpy(timeString, ctime(&clock));
  
    fprintf(stdout,"The UNIX Systems time is: %ld \n", clock);
    fprintf(stdout,"The REAL Time is: %s \n", timeString); 

	fprintf(stdout," %s", timeString);

}
