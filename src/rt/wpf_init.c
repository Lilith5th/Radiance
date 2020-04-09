#ifndef lint
static const char	RCSid[] = "$Id: wpf_init.c,v 1.1 2011/10/22 22:38:10 greg Exp $";
#endif
/*
 *  wpf_init.c - driver for wpf
 */


#include "math.h"
#include "copyright.h"
#include  "standard.h"
#include  "platform.h"
#include  "color.h"
#include  "driver.h"
#include  "rpaint.h"
#include "color.h"

#ifdef __cplusplus
extern "C" {
#endif
	static char lastPrompt[1024];
	static const char* currentCommand = 0;
	static int abort_render = 0;
	static int progress = 0;
	static int last_total_progress = 0;
#define GAMMA		2.2		/* default exponent correction */

	static dr_closef_t wpf_close;
	static dr_clearf_t wpf_clear;
	static dr_paintrf_t wpf_paintr;
	static dr_getcurf_t wpf_getcur;
	static dr_comoutf_t wpf_comout;
	static dr_cominf_t wpf_comin;
	static dr_flushf_t wpf_flush;

	//struct function pointer 
	static struct driver  wpf_driver = {
	  wpf_close, wpf_clear, wpf_paintr, wpf_getcur,
	  wpf_comout, wpf_comin, wpf_flush
	};

	/* functions from wpf_rvu_main.cxx */
	extern int wpf_rvu_init(char*, char*, int*, int*);
	extern void	 CS_SetRes(int, int);
	int CS_Paint(int r, int g, int b, int xmin, //NOTE LUCANO IZVORNO NIJE BIA INT
		int ymin, int xmax, int ymax);
	extern void CS_FlushDisplay();
	extern int wpf_rvu_run();
	extern void CS_comout(const char*);
	extern void CS_debug(const char*);
	extern void CS_SetProgress(int);
	extern int CS_GetTextDia(char*, const char*);
	extern void CS_GetPosition(int* x, int* y);//NOTE LUCANO doda


	extern struct driver*
		wpf_init(		/* initialize driver */
			char* name,
			char* id
		)
	{
		printf("initializing winform driver\n");
		lastPrompt[0] = 0;
		make_gmap(GAMMA);
		wpf_rvu_init(name, id, &wpf_driver.xsiz, &wpf_driver.ysiz);//pokreni gui part
		return(&wpf_driver);
	}

	static void
		wpf_close(void)			/* close our display */
	{
		fprintf(stderr, "wpf close\n");
	}

	static  void
		wpf_clear(			/* clear our display */
			int  xres,
			int  yres
		)
	{
		//CS_comout("cleaning display");
		CS_SetRes(xres, yres);
	}

	extern void wpf_setRes(int X, int Y)//EX MACHINA
	{
		dev->xsiz = X;
		dev->ysiz = Y;
		wpf_clear(X, Y);
	}

	static void
		wpf_paintr(		/* fill a rectangle */
			COLOR  col, int  xmin, int  ymin,
			int  xmax, int  ymax
		)
	{
		uby8 rgb[3];
		map_color(rgb, col);
		printf("%c",(char*)rgb[RED]);
		CS_Paint(rgb[RED], rgb[GRN], rgb[BLU], xmin, ymin, xmax, ymax);
	}

#define SCALE 1.2
	static void
		wpf_flush(void)			/* flush output */
	{
		int p;
		if (last_total_progress)
		{
			progress++;
			p = pdepth * 10 + (int)floor(10 *
				atan(SCALE * progress / last_total_progress) * 2.0 / 3.141593);
			CS_SetProgress(p);
		}
		CS_FlushDisplay();
	}




	static int comincalls = 0;
	static void
		wpf_comin(		/* read in command line wpf gui */
			char* inp,
			char* prompt
		)
	{
		if (prompt && strlen(prompt))
		{
			wpf_comout(prompt);
			CS_debug("comin 01");
		}

		if (comincalls < 2)
		{
			comincalls++;
			CS_debug("comin 02");
		}
		/* if prompt is not set, then use the lastprompt which
		   comes from the last call to comout */
		if (!prompt)
		{
			prompt = lastPrompt;
			CS_debug("comin 03");
		}

		/* first time call to wpf_comin start wpf event loop */
		if (comincalls == 1)
		{
			//return;
			//no need for this. we are 
			//already running the event 
			//loop back in winforms
			//wpf_rvu_run(); exit(0);
			CS_debug("comin 04");
		}

		///* this code will be called from inside the running
		//   wpf_rvu_run loop in two cases:
		//   1. when a user enters a new command to the text widget. in
		//   this case the currentcommand is not null.
		//   2. when a command calls comin directly, that is the else
		//   case. */

		CS_debug("current com:");
		CS_debug(currentCommand);

		if (currentCommand)//ako postoji trenutna naredba, izbaci je na prompt. I izadji iz ove metode.
		{
			CS_debug("comin 05");
			/* a user has entered a new command */
			wpf_comout((char*)currentCommand);
			/* copy the user command into inp, set
			   currentcommand to null so as not to use it again,
			   and then return */
			strcpy(inp, currentCommand);
			currentCommand = 0;
			CS_debug("comin 05b");
			return;//takes us out of this method
		}
		else
		{
			CS_debug("comin 06");
			//return;
			/* in this case comin has been called from inside a
			   command call like view. ok tricky bit here.

			   if the prompt is empty,
			   it is just trying to get the user to hit enter so that
			   the most recent comout does not scrool away. since the
			   wpf interface has a scroll bar we don't have to do that,
			   and do not want to prompt the user.

			   if the prompt is set to "done: ",
			   then we don't want to prompt the user. in other cases
			   we assume that we are inside a command that is asking
			   for more input and we use a dialog box to get it. */

			   //ako je nesto u "promptu" i nije "done: "
			//wpf_comout("we are checkin if prompt longer than 0 and that its not done:");
			//prompt je zadnja linija outputa
			CS_debug(prompt);
			if (strlen(prompt) > 0 && strcmp(prompt, "done: ") != 0)//0 is equal
			{
				CS_debug("comin 07");
				/* if the user entered new text, then echo that
				   to the output window. */
				if (CS_GetTextDia(inp, prompt))//if textbox is not empty, and current text value is different from inp 
				{
					CS_debug("comin 072");
					CS_comout(inp);
				}
				lastPrompt[0] = 0;
			}
			if (strcmp(prompt, "done: ") == 0)
			{
				CS_debug("comin 08");
			}
		}
	}

	static void//send to screen
		wpf_comout(		/* write out string to stdout */
			char* outp)
	{
		if (!outp) {
			return;
		}
		CS_debug("wpfComout START");
		if (strlen(outp) > 0)
		{//we check if its a command or some random notice
			if (outp[strlen(outp) - 2] == ':') {
				strcpy(lastPrompt, outp);
			}
			CS_comout(outp);
		}
		CS_debug("wpfComout END");
	}

	//extern void CS_GetPosition(int* x, int* y);

	static int
		wpf_getcur(		/* get cursor position */
			int* xp,
			int* yp
		)
	{
		CS_GetPosition(xp, yp);
		return 0;
	}

	void wpf_set_abort(int value)	//todo LUCANO added extern
	{
		dev->inpready = value;
	}

	/* process one command from the GUI */
	void wpf_process_command(const char* com)
	{
		char  buf[512];
		buf[0] = 0;
		/* set the currentCommand to the command */
		currentCommand = com;
		/* Call command with no prompt, this will in
		   turn call qt_comin which will use the currentCommand
		   pointer as the command to process */

		CS_debug("wpf process command");
		command("");
		CS_debug("wpf process command2");
		/* after processing a command try to do a render */
		for (; ; )
		{
			if (hresolu <= 1 << pdepth && vresolu <= 1 << pdepth)
			{
				//CS_SetProgress(100);
				CS_comout("done");
				return;
			}
			errno = 0;
			if (hresolu <= psample << pdepth && vresolu <= psample << pdepth)
			{
				sprintf(buf, "%d sampling...\n", 1 << pdepth);
				CS_comout(buf);
				last_total_progress = progress ? progress : 1;
				progress = 0;
				rsample();
			}
			else
			{
				sprintf(buf, "%d refining...\n", 1 << pdepth);
				CS_comout(buf);
				last_total_progress = progress ? progress : 1;
				progress = 0;
				refine(&ptrunk, pdepth + 1);
			}
			if (dev->inpready)
			{
				CS_comout("abort");
				dev->inpready = 0;
				pdepth = 10;
				return;
			}
			/* finished this depth */
			pdepth++;
		}
	}



#ifdef __cplusplus
}
#endif