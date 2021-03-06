/*
TWRITE.C v0.1
Altair 8800micro utility to copy files from the CP/M environment to the
Propeller Terminal SDCard.

Garry Jordan, 03/2013
*/

#include <stdio.h>

/* open() modes */
#define READ  0
#define WRITE 1
#define RDWR  2

/* seek() codes */
#define SEEKORG 0
#define SEEKCUR 1
#define SEEKEND 2

main(argc, argv)
int argc;
char **argv;
{
	int i, tmp, fnidx, nfiles, fd;
	int nsecs;
	char *lastarg, *cp, *buff;

/*
	if (argc < 2 || argc > 3)
	{
		usage(FALSE);
		exit(ERROR);
	}
*/

	i = 1;

	while (i < argc)
	{
		if ('-' == *argv[i])
		{
			switch(argv[i][1])
			{
				case 'H':
				case 'h':
				case '?':
					usage();
					exit(OK);
				default:
					printf("Unrecognized option: -%s\n",
					       argv[i]);
					usage();
					exit(ERROR);
			}

			argc--;
			for (tmp = i; tmp < argc; tmp++)
				argv[tmp] = argv[tmp + 1];
		}
		else
			i++;
	}

	lastarg = argv[argc - 1];	/* FIXME: may be able to remove this */

	if (wildexp(&argc, &argv, FALSE))
	{
		fprintf(stderr, "Invalid filespec.\n");
		exit(ERROR);
	}

	/* If user area or drive present, get offset to filename */
	for (fnidx = 0, cp = argv[1]; *cp; cp++)
	{
		if ('/' == *cp)
			fnidx = cp - argv[1] + 1;
		if (':' == *cp)
			fnidx = cp - argv[1] + 1;
	}

	/* Process the file list. */
	for(nfiles = 0, i = 1; i < argc; i++)
	{
		if (ERROR == (fd = open(argv[i], READ)))
		{
			fprintf(stderr, errmsg(errno()));
			exit(errno());
		}

		printf("%s, (%d sectors)\n", argv[i], (nsecs = cfsize(fd)));
		check_text(fd, nsecs);
		nfiles++;
		close(fd);
	}

        if (nfiles == 1)
            printf("1 file\n");
        else
            printf("%d files\n", nfiles);
	exit(OK);
}

void check_text(fd, nsecs)
{
	char buff[SECSIZ];
	int i, ctrlz, czidx;

	if (ERROR == seek(fd, -1, SEEKEND))
	{
		fprintf(stderr, "%s", errmsg(errno()));
		return;
	}

	printf("\tPositioned at sector %d of %d\n", tell(fd), nsecs);

	if (read(fd, buff, 1) < 0)
	{
		fprintf(stderr, "%s", errmsg(errno()));
		return;
	}

	for(ctrlz = i = 0; i < SECSIZ; i++)	
	{
		if (0x1a == buff[i])
		{
			if (!ctrlz && 0x1a == buff[i])
				czidx = i;
			ctrlz++;
		}
	}

	if (!ctrlz)
		printf("\tFile is binary.\n");
	else
	{
		printf("\tFile may be text.\n");
		printf("\tCtrl-Z idx: %d, count: %d\n", czidx, ctrlz);
	}

	seek(fd, 0, SEEKORG);
}

void usage()
{
	printf("Don't know nuthin...\n");
}
