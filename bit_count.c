
/* bitcount: frequency of bits in each byte of a file.
 *
 * Usage: bitcount <filename>
 *
* 17 Jan 1998, SCRYER
*/

#include <stdio.h>

 main(int argc, char **argv)
{
	 unsigned long freq[8], n;
	 FILE *infile;
	 int c, i;

	 if (argc != 2)
	 {
		 fprintf(stderr, "Usage: bitcount <filename>\n");
		 return 1;
	 }
	 if ((infile = fopen(argv[1], "rb")) == NULL)
	 {
		 fprintf(stderr, "Can't read input file %s.\n", argv[1]);
		 return 1;
	 }
	 for (i = 0; i < 8; i++)
		 freq[i] = 0;
	 n = 0;
	 while ((c = getc(infile)) != EOF)
	 {
		 n++;                            /* Count total bytes */
		 for (i = 0; i < 8; i++)
			 if (c & (1 << i)) freq[i]++;
	 }
	 fclose(infile);
	 printf("%d bytes.  From least significant to most:\n", n);
	 for (i = 0; i < 8; i++)
		 printf("%d: %8d %5.2f%%\n", i, freq[i], 100.*freq[i]/n);
	 return 0;
}
