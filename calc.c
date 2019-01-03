/****************************************************************/
/*                                              		*/
/*                                 				*/
/*		calc - an interpretive algebraic compiler	*/
/*								*/
/*		written by:	pj bell				*/
/*			    december, 1988			*/
/*								*/
/*					 			*/
/*			 Copyright, 1988-2012		*/
/*			   Paul J. Bell				*/
/*			All Rights Reserved			*/
/*								*/
/*								*/
/****************************************************************/

	/*			include some stuff		*/

#include <stdio.h>
#include <ctype.h>
#include <math.h>
#include <sys/file.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>

	/*			define some other stuff		*/

#define NO_OPERATORS 73		/* size of operator stack */
#define NO_BARRIERS 11		/* number of barriers */
#define NO_FUNCTS 10		/* number of functions */
#define NO_NAMES 51		/* number of variables */
#define SYMBOL_SIZE 8		/* number of bytes in variable name */
#define MULTIPLY 4		/* multiply barrier code */
#define LEFTPARA 9              /* left para barrier code */
#define UNARY_MINUS 11          /* unary minus barrier code */
#define BEG 1                   /* $ barrier code (beginning barrier) */
#define POINT 10                /* function barrier code */
#define RIGHT_PARA 8		/* right para barrier code */
#define SUB 2			/* minus barrier code */
#define PLUS 3			/* plus barrier code */
#define COLON 11		/* colon barrier code */


	/*			declare storage			*/

	char copyright[]  ="           calc        ";    /*  name and    */
	char copyright1[] ="  Copyright, 1988-2012 ";    /*  copyright   */
	char copyright2[] ="        Paul J. Bell";       /*     for      */
	char copyright3[] ="     All Rights Reserved";   /* binary code  */


	    char	argument_buffer[81];
	    char        blanks[9] = "        ";
	    char        functions[NO_FUNCTS][SYMBOL_SIZE+1];
		char		fixed_name[SYMBOL_SIZE+1]="      lr";	/* name for last results in symbol table */
	    int         barrier;
	    int         barriers[NO_BARRIERS];
	    char        character;
	    int         character_count;
	    int		command_line_arg_flag;
	    double      current_number;
	    char	end_code[] = "    stop"; /* used to stop everything */
	    char	filename[] = "calc.sav";	/* name of file for saving name(s) table */
	    char        format_func[] = "     fmt";
	    int         fraction;
	    char        functions[NO_FUNCTS][SYMBOL_SIZE+1];
	    int         function_code[NO_FUNCTS];
	    int         i;
            char        image[81];
	    int		j;
	    int         k;
	    int         last_barrier;
            double      last_number;
	    char        name[NO_NAMES][SYMBOL_SIZE+1];
	    int         name_depth;
	    int         name_link;
            double      names[NO_NAMES];
            int         namm;
	    int         nchar;
            int         number;
            double      numbers[NO_OPERATORS];
	    double      operator[NO_OPERATORS];
	    int         operator_depth;
            char        output_format[] = " \n %15.3f \n";
	    int		out_ctl_flag;		/* output control flag */
	    char	out_ctl_cmd[] = "oggleout"; /* toggle output */
	    char        restore_func[] = " restore";
	    char        save_func[] = "    save";
	    int         semicolon_flag;
	    char        symbol[SYMBOL_SIZE+1];
            double      ty[NO_OPERATORS];
            int         type;
            double      value;
	    char	version_number[] = "version_number = 2.2";
	    char fname[200];

 	    struct stat stab;

/*		end of storage declarations 	*/

                 main (argc, argv)
                int argc;
                char *argv[];
{
		/* initilize table of barriers */

	barriers[1]='$';	/* <1> line terminating barrier */
	barriers[2]='-';	/* <2> subtract operator */
	barriers[3]='+';	/* <3> addition operator */
	barriers[4]='*';	/* <4> multiplication operator */
	barriers[5]='/';	/* <5> division operator */
	barriers[6]='=';	/* <6> assignment operator */
	barriers[7]='^';	/* <7> involution operator */
	barriers[8]=')';	/* <8> left parentheses */
	barriers[9]='(';	/* <9> right parentheses */
	barriers[10]='.';	/* <10> function call operator */
	barriers[11]=':';	/* <11> special function operator */

		/* set function table */

	strcpy(functions[0], "        ");	/*null function */
	strcpy(functions[1], "     sin");	/*   sin */
	strcpy(functions[2], "     cos");	/*   cos */
	strcpy(functions[3], "    sqrt");	/*  sqrt */
	strcpy(functions[4], "    atan");  	/*  atan */
	strcpy(functions[5], "     log");  	/*   log */
	strcpy(functions[6], "   log10");  	/* log10 */
	strcpy(functions[7], "     int");  	/*   int */
	strcpy(functions[8], "     exp");  	/*   exp */
	strcpy(functions[9], "     abs");  	/*   abs */

		/* set function codes */

        function_code[0] = 0;	/*  null */
	function_code[1] = 1;	/*   sin */
	function_code[2] = 2;	/*   cos */
	function_code[3] = 3;	/*  sqrt */
	function_code[4] = 4;	/*  atan */
	function_code[5] = 5;	/*   log */
	function_code[6] = 6;	/* log10 */
	function_code[7] = 7;	/*   int */
	function_code[8] = 8;	/*   exp */
	function_code[9] = 9;	/*   abs */


		/* initilize symbol table */

		for (i = 0; i < NO_NAMES; ++i) {
			names[i] = 0.0;
			strcpy (name[i], blanks);
			}

		/* initilize various other tables and stacks */

		for (i = 0; i < NO_OPERATORS; ++i) {
			operator[i] = 0.0;	/* operator stack */
			ty[i] = 0.0;            /* working stack */
			numbers[i] = 0.0;       /* operand stack */
		}
		out_ctl_flag = 1;		/* set output flag */

		/* end of initilazation code */

	command_line_arg_flag = 0;	/* first time only */

	if ( argc > 1 ) {		/* if arguments on command line,  */

		command_line_arg_flag = 1;	/* remember it,  */
		for (i = 1; i < argc; i++)	/* & move all arguments */
			strcat (argument_buffer, argv[i]); /* to buffer */
	}

	begin();	/* call begin to get it going */

	return(0);
}			/* end of function main */

void	usage()		/* output usage message */
{
	printf("Usage: \n");
	printf("      Expressions on the command line or at the \"?\" \
prompt, of the form:\n\n");
	printf("[name =] [(] operand <operator> operand [)] . . . . \
<operator> operand.\n\n");
	printf("Name may be a Variable name, from 1 to 8 characters, \
or a Function name.\n");
	printf("\nA Unary \":\" lists variable names and current values.\n");
	printf("\nIn the absense of parentheses, which may be used to \
control the evaluation,\n");
	printf("expressions are evaluated according to algebraic rules. \
That is: Unary Minus, \n");
	printf("Involution & Functions, Multiplication & Division, \
Addition & Subtraction.\n");
	printf("\nFunctions supported are: sin, cos, atan, sqrt, log, log10\
, int, exp, & abs.\n");
	printf("\tAn additional function, \"fmt(nn.m)\" allows the user to \
set the output\n\tformat for a field width of nn, with m places\
 after the decimal point.\n");
	printf("\nOperands may be names or digits\
 (including decimal point).\n");
	printf("\nOperators may be any from the set: +, -, *, / \
or ^ (involution operator).\n");
	printf("\nMultiple expressions on the same line are seperated\
 by the semicolon \";\".\n\n");
	printf("\tExample: abc = sin(1)^2 + cos(1)^2 ; efg = abc * 12.34\n");

	return;

}		/* end of function usage */

void	get_next_character()   	/* Get Next Character (from Image) */
{
	character = image[character_count];

	if (character == 0x04) {	/* ^D means exit bye-bye */
		exit(0);
	}

	if (character == '[' )  	/* translate [ to ( */
		character = '(';

	if (character == ']' )		/* likewise for ] to ) */
		character = ')';

	if (character == ' ') {
		character_count = character_count + 1;
		character = image[character_count];
	}

	if (character == '$') {
		character_count = character_count + 1;
		character = image[character_count];
	}

	character_count = character_count + 1;
	last_barrier = barrier;
	barrier =  number = 0;

	if (character >= 'a' && character <= 'z') { /* is lower case alpha */
		nchar = character;
		return;		/* alpha return, lower case */
	}

	if (character >= 'A' && character <= 'Z') {  /* if upper case, */
		character = (character + 'a' - 'A' ); /* map to lower case */
		nchar = character;
		return;		/* alpha return */
        }
		if (character >= '0' && character <= '9') {  /* is digit ? */			nchar = character - '0';
			nchar = character - '0';
			value = value * 10 + nchar;
			number = 1;
			return;
		}
		if (character == ';') {

			semicolon_flag = 1;
			character = '\0';
		}

		switch (character) {

		case '$' :
			barrier = 1;
			break;

		case '-' :
			barrier = 2;
			break;

		case '+' :
			barrier = 3;
			break;

		case '*' :
			barrier = 4;
			break;

		case '/' :
			barrier = 5;
			break;

		case '=' :
			barrier = 6;
			break;

		case '^' :
			barrier = 7;
			break;

		case ')' :
			barrier = 8;
			break;

		case '(' :
			barrier = 9;
			break;

		case '.' :
			barrier = 10;
			break;

		case ':' :
			barrier = 11;
			break;

		case '\0' :
			barrier = 1;
			character = '$';
			break;

		default:
			barrier = 1;
			break;
		}
	return;

}			/* end of get next character function */

void	shift_string()	/* function, shift string left one character position */
{

	for (j = 0; j <= 7; j++)
		symbol[j] = symbol [j+1];

}		/* end of function, shift string */

void	get_next_string()	/* Get Next String */
{
	value = 0.0;
	type = fraction = 0;
	strcpy (symbol, blanks);

	get_next_character();
		if (barrier != 0) {
			type = -1;	/* unary barrier */
		        goto test_for_decimal_point;
		}
		type = 0;

                if (number == 0)	/* test type of string */
                	goto symbolic_string;

	while (number == 1)		/* numeric string */
		get_next_character();

test_for_decimal_point:

		if (barrier == POINT)	/* test for decimal point */
			goto scan_fraction;

		if (barrier == 0)
			goto scan_error;   /* go announce syntax error */
		return;			/* end of numeric integer string */

scan_fraction:  	/* scan the fractional part of a numeric string */

		number = 1;		/* make sure it works the first time */
		while (number == 1) {
			fraction = fraction + 1;
			get_next_character();
        	}

		if (barrier == 0)
			goto scan_error;

		fraction = fraction - 1;
		if (fraction == 0) {
			type = 0;
			return; 	/* real numeric string return */
		}			/* of the form 123. */

			/* now adjust value for fractional part */
				/* value = value / 10^fraction */
		while (fraction != 0)  {
			value = value / 10.0;
			fraction = fraction - 1;
		}

		type =0;
		return;		/* return with fractional part */
				/* of the form 123.456 */

symbolic_string:
		type = 1;
		while (barrier == 0) {
		shift_string();
		symbol[7] = character;
			get_next_character();
		}
		if (character == '.')
			goto scan_error;

		return;		/* symbolic string return */

scan_error:	printf("syntax error in GNS\n");

			return;

}			/*	end of function, get_next_string 	*/


void	push_down_operators()	/* push down operators function */
{
		operator_depth = operator_depth + 1;
			if (operator_depth > NO_OPERATORS) {
				printf("too many operators in PDO\n");
				return;
			}
		operator[operator_depth] = barrier;
		return;
}			/* end of function, push_down_operators 	*/

void	push_down_names()	/* push down names function */
{
		name_depth = name_depth + 1;
			if (name_depth > NO_NAMES) {
				printf("too many names in PDN\n");
				return;
			}
		numbers[name_depth] = value;
		ty[name_depth] = name_link;
		return;

}			/* end of function, push_down_names	*/

void	pop_up_operators()	/* pop up operators function */
{
			operator_depth = operator_depth - 1;
			return;

}			/* end of function, pop_up_operators	*/

void	pop_up_names()		/* pop up names function */
{
			name_depth = name_depth - 1;
			if (name_depth < 1) {
				printf("too many names in PUN\n");
				return;
			}
			return;

}			/* end of function, pop_up_names	*/

void	nam()			/* symbol table look */
{

		i = 1;
		while ( i < NO_NAMES ) {

		/* first, look for a name that matches */

			if (strcmp(name[i], symbol)== 0) {
				value = names[i];
				namm = i;
				return;
			}

		/* then, look for an empty slot */

			if (strcmp(name[i], blanks)== 0) {
				strcpy (name[i], symbol);
				value = names[i];
				namm = i;
				return;
			}
		i = i + 1;
		}
		/* symbol table overflow */

			printf("too much to remember in NAM\n");
			return;

}			/* end of function, symbol table look NAM */

void	print_symbol_table()	/* loop thru symbol table & print */
{
	printf("     Name \t \t    Value \n");

	i = 1;
	while (strcmp(name[i], blanks) != 0) {
		strcpy(symbol, name[i]);
		printf("%s \t %18.5f \n", symbol, names[i]);
		++i;
		}
}			/* end of function, print symbol table 	*/


	begin()			/* main line */
{
    pre_load_symbol_table(); /* pre load symbol table */

	semicolon_flag = 0;             /* first time only */

input:

	if (command_line_arg_flag == 1) {
		strcpy (image, argument_buffer); /* copy args to image */
		command_line_arg_flag = 2;
		semicolon_flag = 2;
	}

		if (semicolon_flag == 0) {

			if (command_line_arg_flag == 2)
				exit(2);  /* if args on cmd line, exit now */

			for (i = 1; i <= 81; ++i) /* it all starts here */
				image[i] = ' ';

			if (out_ctl_flag == 1)
				printf("? ");		/* user prompt */

			for (i = 0; (character = getchar()) != '\n'; ++i)
				image[i] = character;

			image[80] = '\0';
			character_count = 0;
		}
	if (image[0] == '?' ) {
		usage();
		semicolon_flag = 0;
		goto input;
	}

	operator_depth = 1;
	operator[1] = BEG;
	name_depth = 1;
	semicolon_flag = 0;

loop:
	name_link = 1;			/* loops to here */
	get_next_string();
	if (type == -1) { 	/* type of -1 means unary barrier */
		switch (barrier) {

		case 1 :			/* $ decide to compile */
			oper();			/* expression scan */
		if (out_ctl_flag == 1)
			printf(output_format , current_number);
		save_last();
		goto input;

		case 2 :			/* - unary minus */
			barrier = UNARY_MINUS;
			push_down_operators();
			goto loop;

		case 3 :			/* loop  */
			goto loop;

		case 4 :			/* * push and loop */
			push_down_operators();
			goto loop;

		case 5 :			/* / push and loop */
			push_down_operators();
			goto loop;

		case 6 :			/* = push and loop */
			push_down_operators();
			goto loop;

		case 7 :			/* ^ push and loop */
			push_down_operators();
			goto loop;

		case 8 :			/* ) decide to compile */
			oper();			/* expression scan */
			if (character != '$')
				goto loop;
		if (out_ctl_flag == 1)
			printf(output_format,current_number);
		save_last();
		goto input;

		case 9 :			/* ( push and loop */
			push_down_operators();
			goto loop;

		case 10 :			/* . error */
			printf("syntax error - main line -\n");
			goto input;

		case 11 :			/* : print symbol table */
			print_symbol_table();
			goto input;

		default:
			goto input;
		}
	}

	if (type == 1) {	/* type of +1 means symbolic barrier */

/***** following is a crude hack to process a few commands  *****/

	if (barrier == COLON) {

	    if (strcmp(end_code, symbol) == 0)	/* "stop" means exit */
		exit(0);

	    if (strcmp(out_ctl_cmd, symbol) == 0) { /* toggle output flag */

		if (out_ctl_flag == 0) {
			out_ctl_flag = 1;
			goto input;
		}
		if (out_ctl_flag == 1) {
			out_ctl_flag = 0;
		goto input;
		}
	    }

/* what follows is a kludge to allow the user to set format for output 	*/

	    if (strcmp(format_func, symbol) == 0) {

		output_format[0] = ' ';		/* "( \n 		*/
		output_format[1] = '\n';        /* "( \n 		*/
		output_format[2] = ' ';         /* "( \n  		*/
		output_format[3] = '%';         /* "( \n % 		*/

		get_next_character();
		if (number != 1) {
			printf ("\n incorrect format for fmt. s/b nn.n \n");
			goto input;
		}
		output_format[4] = character;	/* "( \n %n 		*/

		get_next_character();
		if (number != 1) {
			printf("\n incorrect format for fmt. s/b nn.n \n");
			goto input;
		}
		output_format[5] = character;	/* "( \n %nn 		*/

		get_next_character();
		if (barrier != POINT) {
			printf("\n incorrect format for fmt. s/b nn.n \n");
			goto input;
		}
		output_format[6] =  character;	/* "( \n %nn.  		*/

		get_next_character();
		if (number != 1) {
			printf("\n incorrect format for fmt. s/b nn.n \n");
			goto input;
		}
		output_format[7] =  character;	/* "(\n %nn.n  		*/
		output_format[8] = 'f';		/* "(\n %nn.nf  	*/
		output_format[9] =  ' ';	/* "(\n %nn.n  		*/
		output_format[10] = '\n';       /* "(\n %nn.nf \n 	*/
		output_format[11] = '\0';	/* "(\n %nn.nf \0)" 	*/
		printf( "\n");
		goto input;
	    }

/* 	end of set output format kludge					*/


/*	see if we are to restore a previously saved table ? */

	    if (strcmp (restore_func, symbol) == 0) {
		FILE *ifp;
		ifp = fopen (filename, "r");

			for (i = 0; i < NO_NAMES; ++i) {   /* first, clear the table */
				names[i] = 0.0;
				strcpy (name[i], blanks);
			}
			i = 1;
			while (i < NO_NAMES) {
				fscanf (ifp, "%8c", symbol);
				fscanf (ifp, "%18lf", &current_number);
					if (strcmp (symbol, "  eofeof") == 0) {
						fclose (ifp);
						goto input;
					}
				strcpy (name[i], symbol);
				names[i] = current_number;
				++i;
			}
	    }

/*	are we trying to save a table ? 	*/

	if (strcmp (save_func, symbol) == 0) {
FILE *ofp;
		ofp = fopen (filename, "w");
		i=1;
			while (strcmp (name[i], blanks) != 0) {
				fprintf(ofp, "%s%18.5lf", name[i], names[i]);
				++i;
			}
		fprintf(ofp, "%s", "  eofeof");  /* write our very own eof */
		fprintf(ofp, "%18.5lf", current_number);  /* just garbage */
		fclose (ofp);
		goto input;
	}

/**** end of command processing code *****/

	}
	    if (barrier == LEFTPARA) {
		for (i = 0; i < NO_FUNCTS; ++i) {  /* test for function */
			if(strcmp(functions[i], symbol) == 0) {
				barrier = POINT;
				value = function_code[i];
				goto binary_barrier;
			}
		}
	    }
		/* not a function, go look for name in symbol table */
			nam();
			name_link = namm;
			goto binary_barrier;
	}

binary_barrier:
		push_down_names();	/* type of 0 means binary barrier */

			switch (barrier) {

		case 1 :		/* $ decide & terminate */
			oper();			/* expression scan */
		if (out_ctl_flag == 1)
			printf(output_format,current_number);
		save_last();
		goto input;

		case 2 :		/* - decide and loop */
			oper();
			goto loop;

		case 3 :		/* + decide and loop */
			oper();
			goto loop;

		case 4 :		/* * decide and loop */
			oper();
			goto loop;

		case 5 :		/* / decide and loop */
			oper();
			goto loop;

		case 6 :		/* = decide and loop */
			oper();
			goto loop;

		case 7 :		/* ^ decide and loop */
			oper();
			goto loop;

decide_and_loop:
		case 8 :			/* ) decide & loop */
			oper();			/* expression scan */
			name_link = 1;
			get_next_string();
			if (type == -1) {
				oper();		/* decide to compile */
				if (character != '$') {	/* barrier = $ */
                                	goto loop;
				}
			if (out_ctl_flag == 1)
				printf(output_format,current_number);
			save_last();
			goto input;
			}
			if (barrier ==  SUB && last_barrier == RIGHT_PARA ) {
				goto decide_and_loop;
			}
			if (barrier == PLUS ) {
				goto decide_and_loop;
			}

			printf("syntax error in binary barrier\n");
			goto input;

		case 9 :			/* ( imply multiplication */
			barrier = MULTIPLY;
			push_down_operators();
			barrier = LEFTPARA;
			push_down_operators();
			goto loop;

		case 10 :			/* . push & loop  */
			push_down_operators();
			goto loop;

		case 11 :			/* : error  */
			printf("syntax error in binary barrier\n");
			goto input;

		default:
			goto input;
		}


}			/*	end of function begin	*/

double	pops_and_loops()	/* function, pop and loops */
{
			pop_up_names();
			pop_up_operators();
			numbers[name_depth] = last_number;
			return(0);

}			/* end of function pops and loops */

void	error()			/* function, oper error */
{
			printf("%d  \n oper error \n", k);
			character = '$';

}			/* end of function error */

void	wrap_up_calls()		/* function, wrap up calls */
{
			pop_up_names();
			pop_up_operators();
			numbers[name_depth] = last_number;

}			/* end of function, wrap up calls */



	/***************************************************************/
	/*		operator precedence table	               */
	/*					                       */
	/*                  current barrier                       l    */
	/*      $   -   +   *   /   =   ^   )   (   . unm         a    */
	/*  $   1   2   3   4   5   6   7   8   9  10  11    $    s    */
	/*  -  12  13  14  15  16  17  18  19  20  21  22    -    t    */
	/*  +  23  24  25  26  27  28  29  30  31  32  33    +         */
	/*  *  34  35  36  37  38  39  40  41  42  43  44    *    b    */
	/*  /  45  46  47  48  49  50  51  52  53  54  55    /    a    */
	/*  =  56  57  58  59  60  61  62  63  64  65  66    =    r    */
	/*  ^  67  68  69  70  71  72  73  74  75  76  77    ^    r    */
	/*  )  78  79  80  81  82  83  84  85  86  87  88    )    i    */
	/*  (  89  9-  91  92  93  94  95  96  97  98  99    (    e    */
	/*      $   -   +   *   /   =   ^   )   (   . unm         r    */
	/***************************************************************/

	oper()			/* decide to compile */
		/* arithmetic scan and execution */
{

operloop:	last_number = numbers[name_depth - 1];
		current_number = numbers[name_depth];
		k = (barrier - 1) * 11 + operator[operator_depth];

		switch (k) {

		case 1:			/*   print */
			return(0);

		case 2:	case 13: case 24: 	/*   sub  ln = ln - cn */
		case 79:

			last_number = last_number - current_number;
			pops_and_loops();
			goto operloop;

		case 3:	case 14: case 25:	/*   add   ln = ln + cn */
		case 80:

			last_number = last_number + current_number;
			pops_and_loops();
			goto operloop;

		case 4:	case 15: case 26:	/*   mpy   ln = ln * cn */
		case 37: case 48: case 81:

			last_number = last_number * current_number;
			pops_and_loops();
			goto operloop;

		case 5: case 16: case 27:	/*  div   ln = ln / cn  */
		case 38: case 49: case 82:

			if ( current_number == 0.0 ) {
				printf( " divide check ");
				pops_and_loops();
				last_number = 0.0;
				goto operloop;
			}

			last_number = last_number / current_number;
			pops_and_loops();
			goto operloop;

		case 6: case 83:		/*  equal   ln = cn  */

			last_number = current_number;
			name_link = ty[name_depth -1 ];
			if (name_link  == 0) {
				pops_and_loops();
				goto operloop;
			}
			else {
				names[name_link] = last_number;
				ty[name_depth -1] = 0.0;
				pops_and_loops();
				goto operloop;
			}

		case 7:	case 18: case 29:  /*  involution  ln = ln ^ cn */
		case 40: case 51: case 84:

			last_number = pow(last_number,current_number);
			pops_and_loops();
			goto operloop;

		case 8:	case 9: case 19: case 30:	/* ) & ( error  */
		case 41: case 52: case 57: case 58: case 59: case 60:
		case 62: case 63: case 66: case 85: case 96:

			error();
			return(0);

		case 10: case 87:		/*  . function call */
			j = last_number;
			switch (j) {

			case 1:			/* sin */
				last_number = sin(current_number);
				wrap_up_calls();
				return(0);
			case 2:			/* cos */
				last_number = cos(current_number);
				wrap_up_calls();
				return(0);
			case 3:			/* sqrt */
				last_number = sqrt(current_number);
				wrap_up_calls();
				return(0);
			case 4:			/* atan */
				last_number = atan(current_number);
				wrap_up_calls();
				return(0);
			case 5:			/* log */
				last_number = log(current_number);
				wrap_up_calls();
				return(0);
			case 6:			/* log10 */
				last_number = log10(current_number);
				wrap_up_calls();
				return(0);
			case 7:			/* int portion */
				i = current_number;
				last_number = i;
				wrap_up_calls();
				return(0);
			case 8:			/* exp */

				last_number = exp(current_number);
				wrap_up_calls();
				return(0);
			case 9:			/* abs */

				last_number = fabs(current_number);
				wrap_up_calls();
				return(0);
                        default :
				return(0);
              		}

                case 11: case 22: case 33: case 44: 	/* negate  */
                case 55: case 88:
                	last_number = - current_number;
                	pop_up_operators();
                	numbers[name_depth] = last_number;
                	goto operloop;

				/* push */

		case 12: case 17: case 20: case 21: case 23: case 28:
		case 31: case 32: case 34: case 35: case 36:
		case 39: case 42: case 43: case 45: case 46:
		case 47: case 50: case 53: case 54: case 56:
		case 61: case 64: case 65: case 67: case 68:
		case 69: case 70: case 71: case 72: case 73:
		case 74: case 75: case 76: case 77: case 89:
		case 90: case 91: case 92: case 93: case 94:
		case 95: case 97: case 99:

			push_down_operators();
			return(0);

		case 86:		/* end of group */

			pop_up_operators();
			return(0);

		case 98:		/* return */
			return(0);

		default :
			return(0);


		}
}			/* end of function oper */
save_last()		/* save last results in symbol 'lr' */
	{
		strcpy(symbol, fixed_name );
		nam();
		names[namm] = current_number;
		return (0);
	}
pre_load_symbol_table()
{

	if(stat(filename, &stab) == 0 )
	{
		FILE *ifp;
		ifp = fopen (filename, "r");
		if (command_line_arg_flag == 0){
			printf("loading saved variables. . . .\n");
		}
		for (i = 0; i < NO_NAMES; ++i) {   /* first, clear the table */
			names[i] = 0.0;
			strcpy (name[i], blanks);
		}
		i = 1;
		while (i < NO_NAMES) {
			fscanf (ifp, "%8c", symbol);
			fscanf (ifp, "%18lf", &current_number);
			if (strcmp (symbol, "  eofeof") == 0) {
				fclose (ifp);
				goto load_complete;
			}
				strcpy (name[i], symbol);
				names[i] = current_number;
				++i;
			}
		}
		if (command_line_arg_flag == 0){
				printf("no saved variables to load!!\n");
		}
load_complete:

/*	pre load some constants	*/

        strcpy(symbol, "       g");
        nam();
        names[namm] = 32.1578;

        strcpy(symbol, "      pi");
        nam();
        names[namm] = 3.14159265;

        strcpy(symbol, "       e");
        nam();
        names[namm] = 2.71828183;

        strcpy(symbol, "       m");
        nam();
        names[namm] = .43429448;

        strcpy(symbol, "     rad");
        nam();
        names[namm] = 57.2957795;

        strcpy(symbol, "      cm");
        nam();
        names[namm] = 30.48006096;

        strcpy(symbol, "      kg");
        nam();
        names[namm] = .4535924;

		if (command_line_arg_flag == 0){
			print_symbol_table();
		}

        return (0);
}
/*	end of code	*/
