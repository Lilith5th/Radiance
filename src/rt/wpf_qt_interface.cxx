#include <string>
#include <iostream>
//#include "rpaint.h"//dodano
//#include "rvmain_dll.cxx"
//#include "rvmainDLL.c"
////#include "rvuwidget.h"
////#include "mainwindow.h"


#ifdef EXPORTS
#define DllExport __declspec(dllexport) 
#else
#define DllExport __declspec(dllimport) 
#endif

#ifdef __cplusplus
extern "C" {
#endif
	const char* myLastPrompt = "";
	static int XSize = 1024;
	static int YSize = 768;

	class program {

	public:
		int mytest(void)
		{
			return 0;
		}

	public:
		int main(int argc, char* argv[]) {
			int i, sum = 0;

			if (argc != 3) {
				printf("Two numbers are needed.\n");
				exit(1);
	}
			printf("The sum is :%d \n ", atoi(argv[1]) + atoi(argv[2]));
			return 0; // WE MUST DO THIS NOW! 
}
	};

	// actual main for testing
	int main(int argc, char** argv)
	{
		program p;
		return p.main(argc, argv);
	}

	thread_local int curThread;

	// callback function prototype
	typedef char* (*CSPromptCALLBACK)(); //(const char* m);
	typedef char* (*CBTxtDia) (int curThread, char* inp, const char* prompt); //(const char* m);
	typedef void (*CBFlush)(); //(const char* m);
	typedef void (*CBDebug)(int curThread, char* m); //(const char* m);
	typedef void (*CBComout)(int curThread, char* m); //(const char* m);
	typedef void (*CBUpdateImage)(int curThread, int* r, int* g, int* b, int* xmin, int* ymin, int* xmax, int* ymax);
	typedef int(*CBViewRes)(int curThread, int* i); //(const char* m);

	CBComout pf_ComOut_Callback = nullptr;
	CBDebug pf_Debug_Callback = nullptr;
	CBUpdateImage pf_RenderCallback = nullptr;
	CBFlush pf_FlushCallback = nullptr;
	CBTxtDia pf_TXT_DiaCallback = nullptr;
	CSPromptCALLBACK pf_PromptCallback = nullptr;
	CBViewRes pf_GetViewResolution = nullptr;

	//setting callback functions in c# 
	DllExport void SetDebugCallback(CBDebug pfn)
	{
		if (pfn)
			pf_Debug_Callback = pfn;
	}

	DllExport void SetComoutCallback(CBComout pfn)
	{
		if (pfn)
			pf_ComOut_Callback = pfn;
	}
	DllExport void SetTXTDiaCallback(CBTxtDia pfn)
	{
		if (pfn)
			pf_TXT_DiaCallback = pfn;
	}
	DllExport void SetPromptCallback(CSPromptCALLBACK pfn)
	{
		if (pfn)
			pf_PromptCallback = pfn;
	}
	DllExport void SetRenderCallback(CBUpdateImage pfn)
	{
		printf("render callbacking... to winform");
		if (pfn)
			pf_RenderCallback = pfn;
	}
	DllExport void SetFlushCallback(CBFlush pfn)
	{
		if (pfn)
			pf_FlushCallback = pfn;
	}
	DllExport void SetResCallback(CBViewRes pfn)
	{
		if (pfn)
			pf_GetViewResolution = pfn;
	}



	/*DllExport void CS_SetRes(int X, int Y)
	{
		XSize = X;
		YSize = Y;
	}*/


	//SEND IMG DATA TO C# 
	//calling callback functions from c(++)
	int CS_Paint(int r, int g, int b, int xmin, int ymin, int xmax, int ymax)
	{
		if (pf_RenderCallback)
			pf_RenderCallback(curThread, &r, &g, &b, &xmin, &ymin, &xmax, &ymax);
		return 0;
	}

	void CS_FlushDisplay()
	{
		if (pf_FlushCallback)
			pf_FlushCallback();
	}

	DllExport void CS_comout(char* m)
	{
		if (pf_ComOut_Callback)
			pf_ComOut_Callback(curThread, m);
		myLastPrompt = m;
	}

	DllExport void CS_debug(char* m)
	{
		if (pf_Debug_Callback)
			pf_Debug_Callback(curThread, m);
	}


	DllExport void CS_GetPosition(int* x, int* y)
	{
		//MainWindowInstance->pick(x, y);
		//addcallback here
		*y = YSize - *y - 1;
	}

	DllExport void CS_SetProgress(int p)
	{
		//return;
		//MainWindowInstance->setProgress(p);
	}

	extern "C" void SetNew(int* x, int* y)
	{
		//MainWindowInstance->pick(x, y);
		//*y = YSize - *y -1;
	}

	//////////////////////////////////
	//////////////////////////////////
	//////////////////////////////////
	//////////////////////////////////NOT USING

	DllExport int wpf_rvu_init(char* name, char* id, int* xsize, int* ysize)//todo remove
	{
		printf("CALLING WPF RVU INT... setting resolution\n");
		//static int argc = 1;
		//static char* argv[] = { "rvu" };
		*xsize = XSize;//TODO LUCANO MANDATORY!!!
		*ysize = YSize;//TODO LUCANO MANDATORY!!!
		//mainDLL(argc, argv);
		return 0;
	}


	//////////////////////////////////NOT USING
	extern "C" int wpf_rvu_run()
	{
		printf("CALLING WPF RVU RUN... ovo je beskorisno \n");

		//winform is already running
		/*The exec() functions return only if an error has occurred.The
			return value is - 1, and errno is set to indicate the error.*/

			//while (true) {
			//}
		return 0;
	}


	DllExport void CS_SetRes(int X, int Y)
	{
		printf("CALLING CS_SetRes... \n");
		XSize = X;
		YSize = Y;
		//printf("resizing resolution %d:\n", XSize);
		//MainWindowInstance->resizeImage(X, Y);
		//MainWindowInstance->show();
	}

	// Return 1 if inp was changed
	// Return 0 if no change was made to inp
	extern "C"
		int CS_GetTextDia(char* inp, const char* prompt)
	{
		CS_debug("CS_GetTextDia");
		//char curText[512]="";
		//if (pf_TXT_DiaCallback)
		//curText =	pf_TXT_DiaCallback();
		bool ok = 1;
		if (!strcmp(myLastPrompt, ""))
		{
			if (myLastPrompt != inp)
			{
				strcpy(inp, myLastPrompt);
				return 1;
			}
		}
		return 0;
	}

#ifdef __cplusplus
}
#endif