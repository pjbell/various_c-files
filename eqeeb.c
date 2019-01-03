/*          Traffic Calculations: Erlang 'B', Extended Erlang 'B'           */
/*          Equivalent Queued Extended Erlang 'B'                           */
/*          Ported to 'c' by  pjbell (19 June, 1997)                        */
/*          from basic, including some 'magic numbers', by gefriend         */
/*          based on original ideas by Professor Erlang, and others.        */


#include <stdio.h>
#include <pwd.h> 
#include <ctype.h>
#include <sys/file.h>

float utilization[5000], blockage[5000];
float first_attempt_traffic, num_trunks_in_group, desired_grade_of_service;
float retry_overflowed_traffic, max_time_in_queue, overflow_traffic;
float bk0, bk1, bk4, x, OverFlow3, OverFlow4;
float computed_grade_of_service, total_traffic_carried;

int num_circuits_required, k;

float get_next_float();

main(argc, argv)
int argc;
char *argv[];

{

while (1) {
     first_attempt_traffic=0;         /* erlangs        */
     num_trunks_in_group=0;           /* ##'s           */
     desired_grade_of_service=0;      /* .001 - .999    */
     retry_overflowed_traffic=.8;     /* .01 - 1.00     */
     max_time_in_queue=0;             /* minutes        */

     OverFlow4=0.0;

     fprintf(stdout, "\n");
     fprintf(stdout, "Enter First Attempt Traffic in Erlangs: ");
     first_attempt_traffic = get_next_float();
     if (first_attempt_traffic == 0.0) {
          fprintf(stdout, "\n\nDone, cheers.\n\n\n");
          exit(0);
     }
     fprintf(stdout, "Enter Number of Trunks in Trunk Group: ");
     num_trunks_in_group = get_next_float();
     if (num_trunks_in_group == 0 ) {
          fprintf(stdout, "Enter Desired Grade of Service [.001-.999]: ");
          desired_grade_of_service = get_next_float();

          if (desired_grade_of_service == 0 ) {
               desired_grade_of_service = .01;
          }
     }

     fprintf(stdout, "Enter Fraction of Overflowed Calls that Retry: ");
     retry_overflowed_traffic = get_next_float();

     if (retry_overflowed_traffic == 0 ) {
          fprintf(stdout, "Enter Maximum Time in Queue for Queued Calls: ");
          max_time_in_queue = get_next_float();
          if (max_time_in_queue != 0) {
               retry_overflowed_traffic = 
                   (max_time_in_queue / ((.94412 * max_time_in_queue) + 1.1467));
          }
     }

     compute();

     /*                    Retrial Traffic                */
     OverFlow3 = ((first_attempt_traffic * computed_grade_of_service) 
                               - OverFlow4) * retry_overflowed_traffic;

     OverFlow4 = first_attempt_traffic * computed_grade_of_service;
     first_attempt_traffic = first_attempt_traffic + OverFlow3;

     while (OverFlow3 > .001) {
          compute();
          OverFlow3 = ((first_attempt_traffic * computed_grade_of_service) 
                                      - OverFlow4) * retry_overflowed_traffic;

          OverFlow4 = first_attempt_traffic * computed_grade_of_service;
          first_attempt_traffic = first_attempt_traffic + OverFlow3;
     }

     overflow_traffic= (( first_attempt_traffic - total_traffic_carried) * 
                            (1 - retry_overflowed_traffic));

     fprintf(stdout, "\n");
     fprintf(stdout, " Offered Traffic <erlangs> =  %.0f\n", first_attempt_traffic);
     fprintf(stdout, " Grade of Service =  %0.2f\n", computed_grade_of_service);
     fprintf(stdout, " Number of Trunks = %i\n",  num_circuits_required);
     fprintf(stdout, " Carried Traffic <erlangs> = %.0f\n", total_traffic_carried);
     fprintf(stdout, " Overflow Traffic <erlangs> = %.0f\n", overflow_traffic);
}
}

compute()
{
   /*Function to compute circuit blockage and utilization
     Entered with first_attempt_traffic = Offered Traffic in Erlangs, and either:
     Desired Grade of Service for the Group, or:
     Number of Trunks in the Trunk Group.
     Computes Utilization and Blockage on each Trunk,
     Total Traffic Carried for the Group, Grade of Service,
     and Number of Circuits Required, up to a max. of 5000 circuits.
*/


     bk1 = 1;
     total_traffic_carried = 0;
     k = num_trunks_in_group;
     if (num_trunks_in_group == 0) {
          k = 5000;
     }
     for (num_circuits_required = 1; num_circuits_required <= k; 
                                        ++num_circuits_required) {

          bk0 = bk1;

          x = bk0 * first_attempt_traffic;

          bk1 = x / (num_circuits_required + x);

          utilization[num_circuits_required] = first_attempt_traffic 
                                             * (bk0 - bk1);

          blockage[num_circuits_required] = bk1;

          total_traffic_carried = 
              total_traffic_carried + utilization[num_circuits_required];


          computed_grade_of_service = 
              (1 - (total_traffic_carried / first_attempt_traffic));

          if (num_trunks_in_group == 0 && 
              desired_grade_of_service >= computed_grade_of_service ) {
               return;
          }

          if (num_circuits_required >= k ) {
               return;
          }

     }
}
dumpall()          /* debugging dumper     */
{

int j;

fprintf(stdout, "\n");
for (j = 1; j < 5000; ++j) {
     if (utilization[j] != 0) {
          fprintf(stdout, "Utilization[%i]= %.4f\n", j, utilization[j]);
     }
}
for (j = 1; j < 5000; ++j) {
     if (blockage[j] != 0) {
          fprintf(stdout, "Blockage[%i]= %.4f\n", j, blockage[j]);
     }
}
fprintf(stdout, "first_attempt_traffic= %.4f\n", first_attempt_traffic); 
fprintf(stdout, "num_trunks_in_group= %.4f\n", num_trunks_in_group); 
fprintf(stdout, "desired_grade_of_service= %.4f\n", desired_grade_of_service); 
fprintf(stdout, "retry_overflowed_traffic= %.4f\n", retry_overflowed_traffic); 
fprintf(stdout, "max_time_in_queue= %.4f\n", max_time_in_queue); 
fprintf(stdout, "computed_grade_of_service= %.4f\n", computed_grade_of_service); 
fprintf(stdout, "total_traffic_carried= %.4f\n", total_traffic_carried); 
fprintf(stdout, "bk0= %.4f bk1= %.4f bk4= %.4f x= %.4f\n", bk0, bk1, bk4, x);
fprintf(stdout, "OverFlow3= %.4f OverFlow4= %.4f\n", OverFlow3, OverFlow4);
fprintf(stdout, "num_circuits_required= %i k= %i\n", num_circuits_required, k);
fprintf(stdout, "\n");
}

float get_next_float()     /* function to get the next real number from standard input */
{

int i, j, p;
unsigned char c;
float X;
unsigned char buf[128];

fflush(stdout);
for (p = 0; p <=  128; ++p){
     buf[p] = '\0';
}
j = 0;
while ((c = gnc()) != EOF) {
     if ( c == '\n') {
          break;
     }

     if ( isdigit (c) || c == '.' ) {
          buf[j++] = c;

          if ( j > 127 ) {
               fprintf(stderr, " Enough already..\n");
               exit(1);
          }
     }
}
X = 0.0;
i = sscanf(buf, "%f", &X);
return(X) ;
}

gnc()     /* function to get the next character from standard input */
{
unsigned char c;
return (read (0, &c, 1) == 1) ? (unsigned char) c : EOF;
}
