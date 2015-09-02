/////////////////////////////////////////////////////////////
// CINEMA 4D SDK                                           //
/////////////////////////////////////////////////////////////
// (c) 1989-2013 MAXON Computer GmbH, all rights reserved  //
/////////////////////////////////////////////////////////////

/*

This is a sample implementation of command line rendering.
Except for the name of the options the code is identical to C4D's internal code.

To use C4D in a batch script write e.g. the following (you need to adapt the paths of the executable and files).
C4D returns the RENDERRESULT enumeration value as error code.

== == BATCH SCRIPT on Windows == ==
start /b /wait "parentconsole" "CINEMA 4D 64 Bit.exe" -nogui -sdk_render cube.c4d -sdk_oimage test.tif

IF ERRORLEVEL 1 ECHO Error Detected
IF ERRORLEVEL 1000 IF NOT ERRORLEVEL 1001 ECHO Error Message: Project not found
IF ERRORLEVEL 1001 IF NOT ERRORLEVEL 1002 ECHO Error Message: Error loading project
IF ERRORLEVEL 1002 IF NOT ERRORLEVEL 1003 ECHO Error Message: No output file specified
IF ERRORLEVEL 1 IF NOT ERRORLEVEL 2 ECHO Error Message: Out of memory
IF ERRORLEVEL 2 IF NOT ERRORLEVEL 3 ECHO Error Message: Asset missing
IF ERRORLEVEL 5 IF NOT ERRORLEVEL 6 ECHO Error Message: Saving Failed
IF ERRORLEVEL 6 IF NOT ERRORLEVEL 7 ECHO Error Message: User Break
IF ERRORLEVEL 7 IF NOT ERRORLEVEL 8 ECHO Error Message: GI Cache missing

== == BATCH SCRIPT on OS X == ==

#!/usr/bin/env/sh

CINEMA\ 4D.app/Contents/MacOS/CINEMA\ 4D -nogui -sdk_render cube.c4d -sdk_oimage test.tif

if ["$?" = "0"]; then
	echo "rendering succeeded"
else
	echo "rendering failed"
fi

*/

#include "c4d.h"
#include <string.h>
#include <ctype.h>

#if defined(MAXON_TARGET_OSX) || defined(MAXON_TARGET_LINUX)
	#include <unistd.h>	// getcwd
#elif defined MAXON_TARGET_WINDOWS
	#include <direct.h>
	#define getcwd _getcwd
#endif

#include "main.h"

Float lastProgressValue = -1.0;
RENDERPROGRESSTYPE lastProgressType = RENDERPROGRESSTYPE_AFTERRENDERING;

static void RenderProgressHook(Float p, RENDERPROGRESSTYPE progress_type, void* context)
{
	if (Abs(p - lastProgressValue) > 0.01 || progress_type != lastProgressType)
	{
		if (progress_type != lastProgressType)
		{
			switch (progress_type)
			{
				case RENDERPROGRESSTYPE_AFTERRENDERING:
					lastProgressValue = 1.0;	// no further output necessary
					GeConsoleOut("Rendering Phase: Finalize");
					break;

				case RENDERPROGRESSTYPE_BEFORERENDERING:
					lastProgressValue = -1.0;	// output new percent value
					GeConsoleOut("Rendering Phase: Setup");
					break;

				case RENDERPROGRESSTYPE_DURINGRENDERING:
					lastProgressValue = -1.0;	// output new percent value
					GeConsoleOut("Rendering Phase: Main Render");
					break;

				case RENDERPROGRESSTYPE_GLOBALILLUMINATION:
					lastProgressValue = -1.0;	// output new percent value
					GeConsoleOut("Rendering Phase: Global Illumination");
					break;
			}

		}

		if (Abs(p - lastProgressValue) > 0.01)
		{
			GeConsoleOut(String("Progress: ") + String::IntToString(Int32(p * 100.0)) + String("%"));
		}

		lastProgressValue = p;
		lastProgressType	= progress_type;
	}
}

void CommandLineRendering(C4DPL_CommandLineArgs* args)
{
	BaseDocument* doc = nullptr;
	String				filename;
	String				oimage;
	String				omultipass;
	Int32					i;
	Int32					frameto = NOTOK;
	Int32					framefrom = NOTOK;
	Int32					framestep = 1;
	Int32					resolutionx = 0, resolutiony = 0;
	Int32					format = 0;
	String				tmp;

	for (i = 0; i < args->argc; i++)
	{
		if (!args->argv[i])
			continue;

		if (!strcmp(args->argv[i], "--help") || !strcmp(args->argv[i], "-help"))
		{
			// do not clear the entry so that other plugins can make their output!!!
			GeConsoleOut("Render Options:");
			GeConsoleOut("-sdk_render filename          ... specify a file to render");
			GeConsoleOut("-sdk_frame from [to [step]]   ... specify start frame, end frame and frame step. 'to' and 'step' are optional");
			GeConsoleOut("-sdk_oimage imagename         ... override the image output path for rendering");
			GeConsoleOut("-sdk_omultipass imagename     ... override the multipass output path for rendering");
			GeConsoleOut("-sdk_oformat imageformat      ... override the image output format to TIFF/TGA/BMP/IFF/JPG/PICT/PSD/PSB/RLA/RPF/B3D");
			GeConsoleOut("-sdk_oresolution width height ... override output image size");
			GeConsoleOut("-sdk_threads threadcnt        ... specify number of threads (0 for auto-detection)");
			GeConsoleOut("");
		}
		else if (!strcmp(args->argv[i], "-sdk_render"))
		{
			args->argv[i] = nullptr;
			i++;
			if (i < args->argc && args->argv[i])
			{
				if (args->argv[i][0] == '-')
				{
					i--;
				}
				else
				{
#ifdef MAXON_TARGET_OSX
					tmp.SetCString(args->argv[i], -1, STRINGENCODING_UTF8);	// MacOS X is using UTF8 as encoding
#else
					tmp.SetCString(args->argv[i], -1);
#endif
					filename += tmp;

					args->argv[i] = nullptr;
				}
			}
		}
		else if (!strcmp(args->argv[i], "-sdk_frame"))
		{
			args->argv[i] = nullptr;

			// from
			if (i + 1 < args->argc && args->argv[i + 1] && args->argv[i + 1][0] != '-')
			{
				i++;
				framefrom = frameto = String(args->argv[i]).ParseToInt32();
				args->argv[i] = nullptr;

				// to
				if (i + 1 < args->argc && args->argv[i + 1] && args->argv[i + 1][0] != '-')
				{
					i++;
					frameto = String(args->argv[i]).ParseToInt32();
					args->argv[i] = nullptr;

					// step
					if (i + 1 < args->argc && args->argv[i + 1] && args->argv[i + 1][0] != '-')
					{
						i++;
						framestep = LMax(String(args->argv[i]).ParseToInt32(), 1);
						args->argv[i] = nullptr;
					}
				}
			}
		}
		else if (!strcmp(args->argv[i], "-sdk_oimage"))
		{
			args->argv[i] = nullptr;

			if (i + 1 < args->argc && args->argv[i + 1] && args->argv[i + 1][0] != '-')
			{
				i++;
				oimage = String(args->argv[i]);
				args->argv[i] = nullptr;
			}
		}
		else if (!strcmp(args->argv[i], "-sdk_omultipass"))
		{
			args->argv[i] = nullptr;

			if (i + 1 < args->argc && args->argv[i + 1] && args->argv[i + 1][0] != '-')
			{
				i++;
				omultipass = String(args->argv[i]);
				args->argv[i] = nullptr;
			}
		}
		else if (!strcmp(args->argv[i], "-sdk_threads"))
		{
			args->argv[i] = nullptr;

			if (i + 1 < args->argc && args->argv[i + 1] && args->argv[i + 1][0] != '-')
			{
				i++;
				Int32 threadcnt = String(args->argv[i]).ParseToInt32();
				GetWorldContainerInstance()->SetInt32(WPREF_CPUCOUNT, threadcnt);
				GetWorldContainerInstance()->SetBool(WPREF_CPUCUSTOM, true);
				GeConsoleOut(String("msg: set thread number to ") + (threadcnt ? String::IntToString(threadcnt) : String("AUTO")));
				args->argv[i] = nullptr;
			}
		}
		else if (!strcmp(args->argv[i], "-sdk_oresolution"))
		{
			args->argv[i] = nullptr;

			if (i + 1 < args->argc && args->argv[i + 1] && args->argv[i + 1][0] != '-')
			{
				i++;
				resolutionx = String(args->argv[i]).ParseToInt32();
				args->argv[i] = nullptr;
				if (i + 1 < args->argc && args->argv[i + 1] && args->argv[i + 1][0] != '-')
				{
					i++;
					resolutiony = String(args->argv[i]).ParseToInt32();
					args->argv[i] = nullptr;
					GeConsoleOut(String("msg: override imagesize to ") + String::IntToString(resolutionx) + " " + String::IntToString(resolutiony));
				}
			}
		}
		else if (!strcmp(args->argv[i], "-sdk_oformat"))
		{
			args->argv[i] = nullptr;

			if (i + 1 < args->argc && args->argv[i + 1] && args->argv[i + 1][0] != '-')
			{
				AutoGeFree<Char> str(String(args->argv[i]).ToUpper().GetCStringCopy());
				if (!str)
					break;
				i++;
				if (!strcmp(str, "TIFF"))
					format = FILTER_TIF;
				else if (!strcmp(str, "TGA"))
					format = FILTER_TGA;
				else if (!strcmp(str, "BMP"))
					format = FILTER_BMP;
				else if (!strcmp(str, "IFF"))
					format = FILTER_IFF;
				else if (!strcmp(str, "JPG"))
					format = FILTER_JPG;
				else if (!strcmp(str, "PICT"))
					format = FILTER_PICT;
				else if (!strcmp(str, "PSD"))
					format = FILTER_PSD;
				else if (!strcmp(str, "PSB"))
					format = FILTER_PSB;
				else if (!strcmp(str, "RLA"))
					format = FILTER_RLA;
				else if (!strcmp(str, "RPF"))
					format = FILTER_RPF;
				else if (!strcmp(str, "B3D"))
					format = FILTER_B3D;
				else if (!strcmp(str, "HDR"))
					format = FILTER_HDR;
				else if (!strcmp(str, "EXR"))
					format = FILTER_EXR;
				else if (!strcmp(str, "PNG"))
					format = FILTER_PNG;

				else
					GeConsoleOut(String("error: oformat not supported ") + args->argv[i]);

				if (format)
					GeConsoleOut(String("msg: override output format to ") + args->argv[i]);

				args->argv[i] = nullptr;
			}
		}
	}

	{
		Filename fn(filename);

		if (!fn.GetDirectory().Content())
		{
			static Char startup[1024];
			getcwd(startup, 1024);
			Int32 len = (Int32)strlen(startup);
			if (len && startup[len - 1] != '/')
			{
				startup[len] = 0;
			}

#ifdef MAXON_TARGET_OSX
			tmp.SetCString(args->argv[i], -1, STRINGENCODING_UTF8);	// MacOS X is using UTF8 as encoding
			fn = tmp + fn.GetFile();
#else
			fn = Filename(startup) + fn.GetFile();
#endif
		}

		do
		{
			GeConsoleOut("Loading Project: " + fn.GetString());

			if (!GeFExist(fn))
			{
				SetExitCode(RENDERRESULT_PROJECTNOTFOUND);
				GeConsoleOut("Project not found");
				break;
			}

			doc = LoadDocument(fn, SCENEFILTER_OBJECTS | SCENEFILTER_MATERIALS | SCENEFILTER_PROGRESSALLOWED | SCENEFILTER_DIALOGSALLOWED | SCENEFILTER_NONEWMARKERS, nullptr);	// FIX[57267] - !! markers should be kept in all places where they do not need to be changed !!
			if (!doc)
			{
				SetExitCode(RENDERRESULT_ERRORLOADINGPROJECT);
				GeConsoleOut("Error loading project");
				break;
			}

			RenderData* rdata = doc->GetActiveRenderData();
			if (!rdata)
			{
				SetExitCode(RENDERRESULT_OUTOFMEMORY);
				GeConsoleOut("No render data present");
				break;
			}

			BaseContainer data = rdata->GetData();

			if (framefrom != NOTOK)
			{
				data.SetInt32(RDATA_FRAMESEQUENCE, 0);
				data.SetTime(RDATA_FRAMEFROM, BaseTime(framefrom, doc->GetFps()));
				data.SetTime(RDATA_FRAMETO, BaseTime(frameto, doc->GetFps()));
				data.SetInt32(RDATA_FRAMESTEP, framestep);
			}

			Int32 starttime = GeGetTimer();

			if (oimage.Content())
			{
				data.SetBool(RDATA_GLOBALSAVE, true);
				data.SetBool(RDATA_SAVEIMAGE, true);
				data.SetFilename(RDATA_PATH, oimage);
				if (!omultipass.Content())
					data.SetBool(RDATA_MULTIPASS_SAVEIMAGE, false);
			}
			if (omultipass.Content())
			{
				data.SetBool(RDATA_GLOBALSAVE, true);
				data.SetBool(RDATA_MULTIPASS_SAVEIMAGE, true);
				data.SetBool(RDATA_MULTIPASS_ENABLE, true);
				data.SetFilename(RDATA_MULTIPASS_FILENAME, omultipass);
				if (!oimage.Content())
					data.SetBool(RDATA_SAVEIMAGE, false);
			}
			if (format != 0)
			{
				// override fileformat
				data.SetInt32(RDATA_FORMAT, format);
			}
			if (resolutionx && resolutiony)
			{
				data.SetInt32(RDATA_XRES, resolutionx);
				data.SetInt32(RDATA_YRES, resolutiony);
			}

			RENDERRESULT res = RENDERRESULT_OK;

			if ((data.GetBool(RDATA_SAVEIMAGE) && data.GetFilename(RDATA_PATH).Content()) ||
					(data.GetBool(RDATA_MULTIPASS_SAVEIMAGE) && data.GetBool(RDATA_MULTIPASS_ENABLE) &&
					 data.GetFilename(RDATA_MULTIPASS_FILENAME).Content()))
			{
				MultipassBitmap* bmp2 = MultipassBitmap::Alloc(data.GetInt32(RDATA_XRES), data.GetInt32(RDATA_YRES), COLORMODE_RGB);
				if (!bmp2)
				{
					GeConsoleOut("Image allocation out of memory (2)");
					SetExitCode(RENDERRESULT_OUTOFMEMORY);
					break;
				}

				starttime = GeGetTimer();
				res = RenderDocument(doc, data, RenderProgressHook, nullptr, bmp2, RENDERFLAGS_EXTERNAL | RENDERFLAGS_NODOCUMENTCLONE | RENDERFLAGS_SHOWERRORS, nullptr);

				MultipassBitmap::Free(bmp2);
			}
			else
			{
				SetExitCode(RENDERRESULT_NOOUTPUTSPECIFIED);
				GeConsoleOut("Error: no output image specified");
				break;
			}

			switch (res)
			{
				case RENDERRESULT_OK:
					GeConsoleOut("Rendering successful: " + String::FloatToString((GeGetTimer() - starttime) / 1000.0) + " sec.");
					break;
				case RENDERRESULT_OUTOFMEMORY:
					GeConsoleOut("Rendering failed: Out of memory");
					break;
				case RENDERRESULT_ASSETMISSING:
					GeConsoleOut("Rendering failed: Asset missing");
					break;
				case RENDERRESULT_SAVINGFAILED:
					GeConsoleOut("Rendering failed: Saving failed");
					break;
				case RENDERRESULT_USERBREAK:
					GeConsoleOut("Rendering failed: User break");
					break;
				case RENDERRESULT_GICACHEMISSING:
					GeConsoleOut("Rendering failed: GI cache missing");
					break;
				case RENDERRESULT_NOMACHINE:
					GeConsoleOut("Rendering failed: No machine available");
					break;
				default:
					GeConsoleOut("Rendering failed: Unknown error");
			}

			if (res != RENDERRESULT_OK)
				SetExitCode(res);
		}	while (false);
	}
	if (doc)
		BaseDocument::Free(doc);
}
