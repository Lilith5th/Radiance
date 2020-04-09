#ifndef lint
static const char	RCSid[] = "$Id: rvmain.c,v 2.17 2016/08/18 00:52:48 greg Exp $";
#endif
/*
 *  rvmain.c - main for rview interactive viewer
 */

 //#include <string>
//#include <iostream>
#include "copyright.h"

#include  <signal.h>
#include  <time.h>

#include  "platform.h"
#include  "ray.h"
#include  "source.h"
#include  "ambient.h"
#include  "rpaint.h"
#include  "random.h"
#include  "paths.h"
#include  "view.h"
#include  "pmapray.h"
#include  "rvmain.h"
#include <process.h>//getpid in this file

#ifdef __cplusplus
extern "C" {
#endif

	// Process a command
	extern "C" void wpf_process_command(const char*);
	// set the abort flag to stop a render in progress
	extern "C" void wpf_set_abort(int);


	typedef struct {
		int do_irrad;           /* compute irradiance? */
		int rand_samp;          /* pure Monte Carlo sampling? */

		double dstrsrc;         /* square source distribution */
		double shadthresh;      /* shadow threshold */
		double shadcert;            /* shadow certainty */

		int directrelay;        /* number of source relays */
		int vspretest;      /* virtual source pretest density */
		int directvis;          /* sources visible? */
		double srcsizerat;      /* maximum ratio source size/dist. */

		//COLOR cextinction;      /* global extinction coefficient */
		//COLOR salbedo;      /* global scattering albedo */
		double seccg;           /* global scattering eccentricity */
		double ssampdist;           /* scatter sampling distance */

		double specthresh;      /* specular sampling threshold */
		double specjitter;      /* specular sampling jitter */

		int backvis;            /* back face visibility */

		int maxdepth;           /* maximum recursion depth */
		double minweight;       /* minimum ray weight */

		////char ambfile;      /* ambient file name */
		//COLOR ambval;       /* ambient value */
		int ambvwt;         /* initial weight for ambient value */
		double ambacc;          /* ambient accuracy */
		int ambres;         /* ambient resolution */
		int ambdiv;         /* ambient divisions */
		int ambssamp;           /* ambient super-samples */
		int ambounce;           /* ambient bounces */
	} RENDER_SETTINGS;			/* render parameters */

	extern char* progname;			/* global argv[0] */
	VIEW  ourview = STDVIEW;		/* viewing parameters */		//TODO LUCANO VIEW
	int  hresolu, vresolu;			/* image resolution */

	int  psample = 8;			/* pixel sample size */
	double	maxdiff = .15;			/* max. sample difference */

	int  greyscale = 1;//def 0			/* map colors to brightness? */
	char* dvcname = "wpf";//dev_default		/* output device name */

	double	exposure = 1;			/* exposure for scene */

	int  newparam = 1;			/* parameter setting changed */

	struct driver* dev =   NULL;		/* driver functions */

	char  rifname[128];			/* rad input file name */

	VIEW  oldview;				/* previous view parameters */

	PNODE  ptrunk;				/* the base of our image */
	RECT  pframe;				/* current frame boundaries */
	int  pdepth;				/* image depth in current frame */

	char* errfile = NULL;			/* error output file */

	int  nproc = 1;				/* number of processes */

	char* sigerr[NSIG];			/* signal error messages */


	static void onsig(int  signo);
	static void sigdie(int  signo, char* msg);
	static void printdefaults(void);

	//above is origi
	//////////////////////////////


	//extern RENDER_SETTINGS  currentRenderSettings = { 0,0,0.0, 0.1, .25, 0, 128, 1, 0.0,  BLKCOLOR, BLKCOLOR, 0.0, 0.0, .3, 1.0,1, 6, 1e-2,// BLKCOLOR , 0,0.3, 32, 256, 64,1 };
	extern RENDER_SETTINGS  currentRenderSettings = { 0,0,0.0, 0.1, 0.25, 0, 128, 1, 0.0,  0.0, 0.0, 0.3, 1.0,1, 6, 1e-2, 0,0.3, 32, 256, 64,1 };

	DllExport extern void setMainView(VIEW view)
	{
		newparam = 0;

		//ourview = view;
		printf("size of view and VIEW\n\n\n");
		printf("\n %f,%f,%f,%f,%f,%f\n", view.hn2,
			view.hoff,
			view.horiz,
			view.vaft,
			view.vdist,
			view.vert);

		printf("and this is the type: %d\n", view.type);
		printf("and this is the typeDef: %d\n", ourview.type);

		printf("and this is the xyz: %f %f %f\n ", view.vp[0], view.vp[1], view.vp[2]);
		printf("and this is the vec: %f %f %f\n ", view.vdir[0], view.vdir[1], view.vdir[2]);
		printf("and this is the vup: %f %f %f\n ", view.vup[0], view.vup[1], view.vup[2]);

		int size = sizeof(view);
		int size2 = sizeof(VIEW);
		printf("and this is the size  in byte: %d\n ", size);
		printf("and this is the size2 in byte: %d\n ", size2);

		newview(&view);
		//newparam = 1;
	}
	DllExport extern int setInpready(int value)
	{
		int temp = value;
		dev->inpready = temp;
		return temp;
	}
	DllExport extern int GetInpready()
	{
		return dev->inpready;
	}
	DllExport extern int GetNnewparam()
	{
		return newparam;
	}
	DllExport extern void setMainExposure(double value)
	{
		exposure = value;
	}

	DllExport extern void setMainRenderSettings(RENDER_SETTINGS value)
	{
		currentRenderSettings = value;

		do_irrad = value.do_irrad;           /* compute irradiance? */
		//rand_samp = value.rand_samp;          /* pure Monte Carlo sampling? */

		//dstrsrc = value.dstrsrc;         /* square source distribution */
		//shadthresh = value.shadthresh;      /* shadow threshold */
		//shadcert = value.shadcert;            /* shadow certainty */

		//directrelay = value.directrelay;        /* number of source relays */
		//vspretest = value.vspretest;      /* virtual source pretest density */
		//directvis = value.directvis;          /* sources visible? */
		//srcsizerat = value.srcsizerat;      /* maximum ratio source size/dist. */

		////cextinction=value.cextinction;      /* global extinction coefficient */
		////salbedo=value.salbedo;      /* global scattering albedo */
		//seccg = value.seccg;           /* global scattering eccentricity */
		//ssampdist = value.ssampdist;           /* scatter sampling distance */

		//specthresh = value.specthresh;      /* specular sampling threshold */
		//specjitter = value.specjitter;      /* specular sampling jitter */

		//backvis = value.backvis;            /* back face visibility */

		//maxdepth = value.maxdepth;           /* maximum recursion depth */
		//minweight = value.minweight;       /* minimum ray weight */

		////char ambfile;      /* ambient file name */
		////ambval=value.ambval;       /* ambient value */
		ambvwt = value.ambvwt;         /* initial weight for ambient value */
		ambacc = value.ambacc;          /* ambient accuracy */
		ambres = value.ambres;         /* ambient resolution */
		ambdiv = value.ambdiv;         /* ambient divisions */
		ambssamp = value.ambssamp;           /* ambient super-samples */
		ambounce = value.ambounce;           /* ambient bounces */

		//newparam = 1;
		//printf("newparam set!\n\n\n");
		//printf("ambounce is %d \n\n\n", value.ambounce);
	}

	DllExport void Set_pdepthZero()
	{
		pdepth = 0;
	}
	DllExport extern void main_nproc(int value)
	{
		nproc = value;
	}


	DllExport  int extern mainDLL(int argc, char* argv[])
	{
		printf("input argC: %d \n", argc);
		printf("input argV[]: %s \n %s \n %s  \n ", argv[0], argv[1], argv[2]);
		printf("Radiance main started \n");

#define	 check(ol,al)		if (argv[i][ol] || \
				badarg(argc-i-1,argv+i+1,al)) \
				goto badopt
#define	 check_bool(olen,var)		switch (argv[i][olen]) { \
				case '\0': var = !var; break; \
				case 'y': case 'Y': case 't': case 'T': \
				case '+': case '1': var = 1; break; \
				case 'n': case 'N': case 'f': case 'F': \
				case '-': case '0': var = 0; break; \
				default: goto badopt; }
		char* octnm = NULL;
		char* err;
		int  rval;
		int  i;
		/* global program name */
		progname = argv[0] = fixargv0(argv[0]);
		/* set our defaults */
		//shadthresh = currentRenderSettings.shadthresh;
		//shadcert = currentRenderSettings.shadcert;
		//directrelay = currentRenderSettings.directrelay;
		//vspretest = currentRenderSettings.vspretest;
		//srcsizerat = currentRenderSettings.srcsizerat;
		//specthresh = currentRenderSettings.specthresh;
		//specjitter = currentRenderSettings.specjitter;
		//maxdepth = currentRenderSettings.maxdepth;
		//minweight = currentRenderSettings.minweight;
		//ambacc = currentRenderSettings.ambacc;
		//ambres = currentRenderSettings.ambres;
		//ambdiv = currentRenderSettings.ambdiv;
		//ambssamp = currentRenderSettings.ambssamp;

		//shadthresh = .1;
		//shadcert = .25;
		//directrelay = 0;
		//vspretest = 128;
		//srcsizerat = 0.;
		//specthresh = .3;
		//specjitter = 1.;
		//maxdepth = 6;
		//minweight = 1e-2;
		//ambacc = 0.3;
		//ambres = 32;
		//ambdiv = 256;
		//ambssamp = 64;

		ambounce = 3;           /* ambient bounces */


		do_irrad = currentRenderSettings.do_irrad;           /* compute irradiance? */
		rand_samp = currentRenderSettings.rand_samp;          /* pure Monte Carlo sampling? */

		dstrsrc = currentRenderSettings.dstrsrc;         /* square source distribution */
		shadthresh = currentRenderSettings.shadthresh;      /* shadow threshold */
		shadcert = currentRenderSettings.shadcert;            /* shadow certainty */

		directrelay = currentRenderSettings.directrelay;        /* number of source relays */
		vspretest = currentRenderSettings.vspretest;      /* virtual source pretest density */
		directvis = currentRenderSettings.directvis;          /* sources visible? */
		srcsizerat = currentRenderSettings.srcsizerat;      /* maximum ratio source size/dist. */

		//cextinction=currentRenderSettings.cextinction;      /* global extinction coefficient */
		//salbedo=currentRenderSettings.salbedo;      /* global scattering albedo */
		seccg = currentRenderSettings.seccg;           /* global scattering eccentricity */
		ssampdist = currentRenderSettings.ssampdist;           /* scatter sampling distance */

		specthresh = currentRenderSettings.specthresh;      /* specular sampling threshold */
		specjitter = currentRenderSettings.specjitter;      /* specular sampling jitter */

		backvis = currentRenderSettings.backvis;            /* back face visibility */

		maxdepth = currentRenderSettings.maxdepth;           /* maximum recursion depth */
		minweight = currentRenderSettings.minweight;       /* minimum ray weight */

		//char ambfile;      /* ambient file name */
		//ambval = currentRenderSettings.ambval;       /* ambient currentRenderSettings */
		ambvwt = currentRenderSettings.ambvwt;         /* initial weight for ambient currentRenderSettings */
		ambacc = currentRenderSettings.ambacc;          /* ambient accuracy */
		ambres = currentRenderSettings.ambres;         /* ambient resolution */
		ambdiv = currentRenderSettings.ambdiv;         /* ambient divisions */
		ambssamp = currentRenderSettings.ambssamp;           /* ambient super-samples */
		ambounce = currentRenderSettings.ambounce;           /* ambient bounces */



		printf("OPTION CITY STARTING...\n");
		/* option city */
		for (i = 1; i < argc; i++) {
			/* expand arguments */
			while ((rval = expandarg(&argc, &argv, i)) > 0)
				;
			if (rval < 0) {
				sprintf(errmsg, "cannot expand '%s'", argv[i]);
				error(SYSTEM, errmsg);
			}
			if (argv[i] == NULL || argv[i][0] != '-')
				break;			/* break from options */
			if (!strcmp(argv[i], "-version")) {
				puts(VersionID);
				quit(0);
			}
			if (!strcmp(argv[i], "-defaults") ||
				!strcmp(argv[i], "-help")) {
				printdefaults();
				quit(0);
			}
			if (!strcmp(argv[i], "-devices")) {
				printdevices();
				quit(0);
			}
			rval = getrenderopt(argc - i, argv + i);
			if (rval >= 0) {
				i += rval;
				continue;
			}
			rval = getviewopt(&ourview, argc - i, argv + i);
			if (rval >= 0) {
				i += rval;
				continue;
			}
			switch (argv[i][1]) {
			case 'n':				/* # processes */
				check(2, "i");
				nproc = atoi(argv[++i]);
				if (nproc <= 0)
					error(USER, "bad number of processes");
				break;
			case 'v':				/* view file */
				if (argv[i][2] != 'f')
					goto badopt;
				check(3, "s");
				rval = viewfile(argv[++i], &ourview, NULL);
				if (rval < 0) {
					sprintf(errmsg,
						"cannot open view file \"%s\"",
						argv[i]);
					error(SYSTEM, errmsg);
				}
				else if (rval == 0) {
					sprintf(errmsg,
						"bad view file \"%s\"",
						argv[i]);
					error(USER, errmsg);
				}
				break;
			case 'b':				/* grayscale */
				check_bool(2, greyscale);
				break;
			case 'p':				/* pixel */
				switch (argv[i][2]) {
				case 's':				/* sample */
					check(3, "i");
					psample = atoi(argv[++i]);
					break;
				case 't':				/* threshold */
					check(3, "f");
					maxdiff = atof(argv[++i]);
					break;
				case 'e':				/* exposure */
					check(3, "f");
					exposure = atof(argv[++i]);
					if (argv[i][0] == '+' || argv[i][0] == '-')
						exposure = pow(2.0, exposure);
					break;
				default:
					goto badopt;
				}
				break;
			case 'w':				/* warnings */
				rval = erract[WARNING].pf != NULL;
				check_bool(2, rval);
				if (rval) erract[WARNING].pf = wputs;
				else erract[WARNING].pf = NULL;
				break;
			case 'e':				/* error file */
				check(2, "s");
				errfile = argv[++i];
				break;
			case 'o':				/* output device */
				check(2, "s");
				dvcname = argv[++i];
				break;
			case 'R':				/* render input file */
				check(2, "s");
				strcpy(rifname, argv[++i]);
				break;
			default:
				goto badopt;
			}
		}

		err = setview(&ourview);	/* set viewing parameters */
		if (err != NULL)
			error(USER, err);
		/* set up signal handling */
		sigdie(SIGINT, "Interrupt");
		sigdie(SIGTERM, "Terminate");
#if !defined(_WIN32) && !defined(_WIN64)
		sigdie(SIGHUP, "Hangup");
		sigdie(SIGPIPE, "Broken pipe");
		sigdie(SIGALRM, "Alarm clock");
#endif
		/* open error file */
		if (errfile != NULL) {
			if (freopen(errfile, "a", stderr) == NULL)
				quit(2);
			fprintf(stderr, "**************\n*** PID %5d: ",
				getpid());
			printargs(argc, argv, stderr);
			putc('\n', stderr);
			fflush(stderr);
		}

#ifdef	NICE
		nice(NICE);			/* lower priority */
#endif
					/* get octree */
		if (i == argc)
			octnm = NULL;
		else if (i == argc - 1)
			octnm = argv[i];
		else
			goto badopt;
		if (octnm == NULL)
			error(USER, "missing octree argument");

		/* set up output & start process(es) */
		SET_FILE_BINARY(stdout);

		printf("rayinit started...\n");
		ray_init(octnm);		/* also calls ray_init_pmap() */
		printf("rayinit over...\n");
		/* temporary shortcut, until winrview is refactored into a "device" */
#ifndef WIN_RVIEW
		rview();			/* run interactive viewer */
		printf("why are we here?!!!");

		//devclose();			/* close output device */
#endif

	/* PMAP: free photon maps */
		//ray_done_pmap();
#ifdef WIN_RVIEW
		//return 1;
#endif
		quit(0);

	badopt:
		sprintf(errmsg, "command line error at '%s'", argv[i]);
		error(USER, errmsg);
		return 1; /* pro forma return */

#undef	check
#undef	check_bool
	}

	void
		wputs(				/* warning output function */
			char* s
		)
	{
		int  lasterrno = errno;
		eputs(s);
		errno = lasterrno;
	}


	void
		eputs(				/* put string to stderr */
			char* s
		)
	{
		static int  midline = 0;

		if (!*s)
			return;
		if (!midline++) {
			fputs(progname, stderr);
			fputs(": ", stderr);
		}
		fputs(s, stderr);
		if (s[strlen(s) - 1] == '\n') {
			fflush(stderr);
			midline = 0;
		}
	}

	static void
		onsig(				/* fatal signal */
			int  signo
		)
	{
		static int  gotsig = 0;

		if (gotsig++)			/* two signals and we're gone! */
			_exit(signo);

#if !defined(_WIN32) && !defined(_WIN64)
		alarm(15);			/* allow 15 seconds to clean up */
		signal(SIGALRM, SIG_DFL);	/* make certain we do die */
#endif
		eputs("signal - ");
		eputs(sigerr[signo]);
		eputs("\n");
		printf("not another close");
		devclose();
		quit(3);
	}


	/*static*/ void
		sigdie(			/* set fatal signal */
			int  signo,
			char* msg
		)
	{
		if (signal(signo, onsig) == SIG_IGN)
			signal(signo, SIG_IGN);
		sigerr[signo] = msg;
	}


	/*static*/ void
		printdefaults(void)			/* print default values to stdout */
	{
		printf("-n %-2d\t\t\t\t# number of rendering processes\n", nproc);
		printf(greyscale ? "-b+\t\t\t\t# greyscale on\n" :
			"-b-\t\t\t\t# greyscale off\n");
		printf("-vt%c\t\t\t\t# view type %s\n", ourview.type,
			ourview.type == VT_PER ? "perspective" :
			ourview.type == VT_PAR ? "parallel" :
			ourview.type == VT_HEM ? "hemispherical" :
			ourview.type == VT_ANG ? "angular" :
			ourview.type == VT_CYL ? "cylindrical" :
			ourview.type == VT_PLS ? "planisphere" :
			"unknown");
		printf("-vp %f %f %f\t# view point\n",
			ourview.vp[0], ourview.vp[1], ourview.vp[2]);
		printf("-vd %f %f %f\t# view direction\n",
			ourview.vdir[0], ourview.vdir[1], ourview.vdir[2]);
		printf("-vu %f %f %f\t# view up\n",
			ourview.vup[0], ourview.vup[1], ourview.vup[2]);
		printf("-vh %f\t\t\t# view horizontal size\n", ourview.horiz);
		printf("-vv %f\t\t\t# view vertical size\n", ourview.vert);
		printf("-vo %f\t\t\t# view fore clipping plane\n", ourview.vfore);
		printf("-va %f\t\t\t# view aft clipping plane\n", ourview.vaft);
		printf("-vs %f\t\t\t# view shift\n", ourview.hoff);
		printf("-vl %f\t\t\t# view lift\n", ourview.voff);
		printf("-pe %f\t\t\t# pixel exposure\n", exposure);
		printf("-ps %-9d\t\t\t# pixel sample\n", psample);
		printf("-pt %f\t\t\t# pixel threshold\n", maxdiff);
		printf("-o %s\t\t\t\t# output device\n", dvcname);
		printf(erract[WARNING].pf != NULL ?
			"-w+\t\t\t\t# warning messages on\n" :
			"-w-\t\t\t\t# warning messages off\n");
		print_rdefaults();

	}



#ifdef __cplusplus
}
#endif