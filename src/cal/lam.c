#ifndef lint
static const char	RCSid[] = "$Id: lam.c,v 1.25 2019/08/13 16:31:35 greg Exp $";
#endif
/*
 *  lam.c - simple program to laminate files.
 *
 *	7/14/88		Greg Ward
 */

#include <ctype.h>

#include "rtio.h"
#include "platform.h"
#include "paths.h"

#define MAXFILE		512		/* maximum number of files */

#define MAXLINE		65536		/* maximum input line */

FILE	*input[MAXFILE];
int	bytsiz[MAXFILE];
char	*tabc[MAXFILE];
int	nfiles = 0;

char	buf[MAXLINE];

int
main(int argc, char *argv[])
{
	long	incnt = 0;
	int	unbuff = 0;
	int	binout = 0;
	char	*curtab = "\t";
	int	curbytes = 0;
	int	puteol;
	int	i;

	for (i = 1; i < argc; i++) {
		if (argv[i][0] == '-') {
			switch (argv[i][1]) {
			case 't':
				curtab = argv[i]+2;
				if (!*curtab) curtab = "\n";
				break;
			case 'u':
				unbuff = !unbuff;
				break;
			case 'i':
				switch (argv[i][2]) {
				case 'n':
					incnt = atol(argv[++i]);
					break;
				case 'f':
				case 'F':
					curbytes = sizeof(float);
					break;
				case 'd':
				case 'D':
					curbytes = sizeof(double);
					break;
				case 'i':
				case 'I':
					curbytes = sizeof(int);
					break;
				case 'w':
				case 'W':
					curbytes = 2;
					break;
				case 'b':
					curbytes = 1;
					break;
				case 'a':
					curbytes = argv[i][3] ? -1 : 0;
					break;
				default:
					goto badopt;
				}
				if (isdigit(argv[i][3]))
					curbytes *= atoi(argv[i]+3);
				curbytes += (curbytes == -1);
				if (curbytes > MAXLINE) {
					fputs(argv[0], stderr);
					fputs(": input size too big\n", stderr);
					return(1);
				}
				if (curbytes > 0) {
					curtab = "";
					++binout;
				}
				break;
			case '\0':
				tabc[nfiles] = curtab;
				input[nfiles] = stdin;
				if (curbytes > 0)
					SET_FILE_BINARY(input[nfiles]);
				bytsiz[nfiles++] = curbytes;
				break;
			badopt:;
			default:
				fputs(argv[0], stderr);
				fputs(": bad option\n", stderr);
				return(1);
			}
		} else if (argv[i][0] == '!') {
			tabc[nfiles] = curtab;
			if ((input[nfiles] = popen(argv[i]+1, "r")) == NULL) {
				fputs(argv[i], stderr);
				fputs(": cannot start command\n", stderr);
				return(1);
			}
			if (curbytes > 0)
				SET_FILE_BINARY(input[nfiles]);
			bytsiz[nfiles++] = curbytes;
		} else {
			tabc[nfiles] = curtab;
			if ((input[nfiles] = fopen(argv[i], "r")) == NULL) {
				fputs(argv[i], stderr);
				fputs(": cannot open file\n", stderr);
				return(1);
			}
			if (curbytes > 0)
				SET_FILE_BINARY(input[nfiles]);
			bytsiz[nfiles++] = curbytes;
		}
		if (nfiles >= MAXFILE) {
			fputs(argv[0], stderr);
			fputs(": too many input streams\n", stderr);
			return(1);
		}
	}
	if (!nfiles) {
		fputs(argv[0], stderr);
		fputs(": no input streams\n", stderr);
		return(1);
	}
	if (binout)				/* binary output? */
		SET_FILE_BINARY(stdout);
#ifdef getc_unlocked				/* avoid lock/unlock overhead */
	for (i = nfiles; i--; )
		flockfile(input[i]);
	flockfile(stdout);
#endif
	puteol = 0;				/* any ASCII output at all? */
	for (i = nfiles; i--; )
		puteol += (bytsiz[i] <= 0);
	do {					/* main loop */
		for (i = 0; i < nfiles; i++) {
			if (bytsiz[i] > 0) {		/* binary input */
				if (getbinary(buf, bytsiz[i], 1, input[i]) < 1)
					break;
				if (putbinary(buf, bytsiz[i], 1, stdout) != 1)
					break;
			} else if (bytsiz[i] < 0) {	/* multi-line input */
				int	n = -bytsiz[i];
				while (n--) {
					if (fgets(buf, MAXLINE, input[i]) == NULL)
						break;
					if ((i > 0) | (n < -bytsiz[i]-1))
						fputs(tabc[i], stdout);
					buf[strlen(buf)-1] = '\0';
					if (fputs(buf, stdout) == EOF)
						break;
				}
				if (n >= 0)		/* fell short? */
					break;
			} else {			/* single-line input */
				if (fgets(buf, MAXLINE, input[i]) == NULL)
					break;
				if (i)
					fputs(tabc[i], stdout);
				buf[strlen(buf)-1] = '\0';
				if (fputs(buf, stdout) == EOF)
					break;
			}
		}
		if (i < nfiles)
			break;
		if (puteol)
			putchar('\n');
		if (unbuff)
			fflush(stdout);
	} while (--incnt);
							/* check ending */
	if (fflush(stdout) == EOF) {
		fputs(argv[0], stderr);
		fputs(": write error on standard output\n", stderr);
		return(1);
	}
	if (incnt > 0) {
		fputs(argv[0], stderr);
		fputs(": warning: premature EOD\n", stderr);
	}
	return(0);
}
