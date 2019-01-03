/*	ldom: lastDayofMonth. based on an input argument, determine if today is the last day of the current month.
    if yes, exit with return code '0', otherwise exit with return code'1'.
	use within crontab to execute a script/program on the last day of a month.
	0 0 * * 0 ldom && command will execute "command" on the last day of the every month.
												pjbell - 10June2014
*/

#include <stdlib.h>
#include <locale.h>
#include <libintl.h>
#include <stdio.h>
#include <time.h>

main(argc, argv)
int argc;
char *argv[];
{

    time_t clock, time();
    struct tm *tm, *localtime();
    char buf[128];
    int i, ww, day, month, year, last, first;
    char smonth[3], sday[3], syear[3];

    int mlength[12];
        mlength[1]=31;  /* jan */
        mlength[2]=28;  /* feb */
        mlength[3]=31;  /* mar */
        mlength[4]=30;  /* apr */
        mlength[5]=31;  /* may */
        mlength[6]=30;  /* jun */
        mlength[7]=31;  /* jul */
        mlength[8]=31;  /* aug */
        mlength[9]=30;  /* sep */
        mlength[10]=31; /* oct */
        mlength[11]=30; /* nov */
        mlength[12]=31; /* dec */

    setlocale (LC_ALL,"");
    clock = time((time_t *)0);
    tm = localtime(&clock);

	strftime(buf, sizeof(buf), "%D", tm);       /* get the time and save in buf */
/*	fprintf(stdout, "%s \n", buf); */
	
	for (i=0; i < 2; ++i) smonth[i] = buf[i]; /* extract month from buf */
		smonth[2] = '\0';

	for (i=0; i < 2; ++i) sday[i] = buf[3+i];   /* extract day from buf */
		sday[2] = '\0';

	for (i=0; i < 2; ++i) syear[i] = buf[6+i]; /* extract year from buf */
		syear[2] = '\0';

    sscanf(smonth, "%d", &month);    /* convert month to numeric */
    sscanf(sday, "%d", &day);        /* convert day to numeric */
    sscanf(syear, "%d", &year);      /* convert year to numeric */
	year = year+2000;
	if ( year <2014 || day <1 || day >31 || month <1 || month > 12 ){
	  fprintf(stdout, "something's wrong here: day= %i, month= %i, year= %i \n", day, month, year);
		exit (99);
	}

	last = mlength[month];

	if ( month == 2) {
		last = 28;
		if ((year % 4 == 0 && year % 100 != 0) || year % 400 == 0) /* check for leap year */
			last = 29;		/* it's a leap year */
	}
	if ( day == last ){
fprintf(stdout, "today is the last day of the month! day= %i, last= %i \n", day, last);
		exit(0); /* today's EOM! */
	}
fprintf(stdout, "today is not the last day of the month! day= %i, last= %i \n", day, last);
	exit(1); /* not EOM! */
}	
