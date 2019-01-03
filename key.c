#include <stdio.h>

char password[9], salt[3], *passwd;


main(argc, argv)

{
	
	printf("Enter Password: ");
														
	while (scanf("%s", password) != EOF) {;
																								
		printf("Enter Salt: ");
		scanf("%s", salt);
		passwd = crypt(password, salt);
		printf("%s %s \n", "Encrypted Password: ", passwd);
		printf("Enter Password: ");
	}
}


