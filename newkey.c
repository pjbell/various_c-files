#include <stdio.h>

char password[9], *passwd;


main(argc, argv)
int argc;
char *argv[];

{
	
		passwd = crypt(argv[1], argv[2]);
		printf("%s\n", passwd);
	}
