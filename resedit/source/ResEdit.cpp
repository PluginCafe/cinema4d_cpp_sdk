/*********************************************************************\
	File name        : ResEdit.cpp
	Description      :
	Created at       : 11.08.01, @ 09:29:23
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/

#include "stdafx.h"
#include "globals.h"
#include "MakeDistriDialog.h"


#if (defined _DEBUG) && (defined USE_CPP_NEW_DELETE)
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

Bool RegisterResEdit(void);
Bool RegisterWizardCommand();
Bool RegisterDiffZipCommand();
void FreeMenuItems();
void WriteDistriPrefs(BaseFile* pFile);
void ReadDistriPrefs(BaseFile* pFile);
void WriteDistriPrefs();
void ReadDistriPrefs();
void FreeDiffZipCommand();

void SaveResEditPrefs()
{
	if (!g_pResEditPrefs)
		return;

	Filename fnPrefs = GeGetC4DPath(C4D_PATH_PREFS) + String("ResEdit.prf");
	AutoAlloc <BaseFile> pf;
	if (pf)
	{
		if (ForceOpenFileWrite(pf, fnPrefs))
		{
			pf->WriteString(g_pResEditPrefs->strOrigPath);
			pf->WriteString(g_pResEditPrefs->strNewPath);
			pf->WriteString(g_pResEditPrefs->strExtractPath);
			pf->WriteString(g_pResEditPrefs->strWizardPath);
			pf->WriteFilename(g_pResEditPrefs->fnNewZip);
			pf->WriteFilename(g_pResEditPrefs->fnDestZip);
			g_pResEditPrefs->arOldFiles.Write(pf, 0);
			pf->Close();
		}
	}
	WriteDistriPrefs();
}

static CDynamicFilenameSet *g_pDiffzipFiles = nullptr;
static Filename *g_pfnDiffzipNewSrc;
static Filename *g_pfnDiffzipDest;

/*********************************************************************\
	Function name    : C4D_PlStart
	Description      : start function of the plugin
	Created at       : 11.08.01, @ 09:46:11
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
Bool PluginStart()
{
#if (defined _DEBUG) && (defined USE_CPP_NEW_DELETE)
	_//CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
	if (!resource.Init()) return false; // don't start plugin without resource

	if (!RegisterResEdit()) return false;
	if (!RegisterWizardCommand()) return false;
	if (!RegisterDiffZipCommand()) return false;

	// load the control images
	g_pControlImages = BaseBitmap::Alloc();
	if (!g_pControlImages)
		return false;
	if (g_pControlImages->Init(GeGetPluginPath() + String("res") + String("buttons.tif"))!=IMAGERESULT_OK)
		return false;

	g_pstrFillSave = NewObjClear(String);
	g_pLastOpenFile = NewObjClear(Filename);
	g_pResEditPrefs = NewObjClear(ResEditPrefs);

	Filename fnPrefs = GeGetC4DPath(C4D_PATH_PREFS) + String("ResEdit.prf");
	AutoAlloc <BaseFile> pf;
	if (!pf || !g_pstrFillSave || !g_pLastOpenFile || !g_pResEditPrefs)
		return false;
	if (pf->Open(fnPrefs, FILEOPEN_READ, FILEDIALOG_NONE))
	{
		pf->ReadString(&g_pResEditPrefs->strOrigPath);
		pf->ReadString(&g_pResEditPrefs->strNewPath);
		pf->ReadString(&g_pResEditPrefs->strExtractPath);
		pf->ReadString(&g_pResEditPrefs->strWizardPath);
		pf->ReadFilename(&g_pResEditPrefs->fnNewZip);
		pf->ReadFilename(&g_pResEditPrefs->fnDestZip);
		g_pResEditPrefs->arOldFiles.Read(pf);
		pf->Close();
	}
	ReadDistriPrefs();

	*g_pstrFillSave = "";
	g_chFill = ' ';
	g_lFillChars = 2;
	char ch[2];
	ch[0] = g_chFill;
	ch[1] = '\0';
	Int32 a;
	for (a = 0; a < g_lFillChars; a++)
		*g_pstrFillSave += ch;

	*g_pLastOpenFile = GeGetStartupPath();

	GePrint("Loaded ResEdit version 5.00\n");

	return true;
}

volatile const char* pChName = "Resource editor version 5.02 by Thomas Kunert";

void EndActivity()
{
	FreeMenuItems();
	FreeDiffZipCommand();
}

/*********************************************************************\
	Funktionsname    : C4D_PlEnd
	Beschreibung     : bye
	Rückgabewert     : void
	Erstellt am      : 11.08.01, @ 09:46:34
	Argument         : void
\*********************************************************************/
void PluginEnd(void)
{
	DeleteObj(g_pDiffzipFiles);
	DeleteObj(g_pfnDiffzipNewSrc);
	DeleteObj(g_pfnDiffzipDest);

	BaseBitmap::Free(g_pControlImages);
	BaseBitmap::Free(g_pStringCompareIcons);

	SaveResEditPrefs();

	if (g_pResEditPrefs)
	{
		g_pResEditPrefs->arOldFiles.Free();
		DeleteObj(g_pResEditPrefs);
	}

	DeleteObj(g_pstrFillSave);
	DeleteObj(g_pLastOpenFile);

#if (defined _DEBUG) && (defined USE_CPP_NEW_DELETE)
	//_CrtDumpMemoryLeaks();
#endif
}

Bool PluginMessage(Int32 id, void *data)
{
	switch (id)
	{
		case C4DMSG_PRIORITY:
			SetPluginPriority(data, 800);
			return true;

		case C4DPL_COMMANDLINEARGS:
			{
				C4DPL_CommandLineArgs *args = (C4DPL_CommandLineArgs*)data;
				Int32 i;

				for (i=0;i<args->argc;i++)
				{
					if (!args->argv[i]) continue;

					if (!strcmp(args->argv[i],"--help") || !strcmp(args->argv[i],"-help"))
					{
						// do not clear the entry so that other plugins can make their output!!!
						GePrint("\x01-makedistri <distributionname> <build_id> ... starts the makedistri script for the given distribution and buildid");
						GePrint("\x01-diffzips <previous zip files> -diffsrc <new zip file> -diffdest <destination zif diff ... generates a difference zip out of X basezips and one new zip");
						GePrint("\x01  example: -diffzips \"11.513_RC20354.zip\" \"11.514_RC20476.zip\" \"11.516_RC21195.zip\" -diffsrc \"11.517_RC21478.zip\" -diffdest \"11.517_RC21478_diff_test.zip\"");
					}
					else if (!strcmp(args->argv[i],"-makedistri"))
					{
						args->argv[i] = nullptr;

						if (i+1<args->argc && args->argv[i+1] && args->argv[i+1][0]!='-')
						{
							i++;
							const String prffile = args->argv[i];
							args->argv[i] = nullptr;

							if (i+1<args->argc && args->argv[i+1] && args->argv[i+1][0]!='-')
							{
								i++;
								const String buildid = args->argv[i];
								args->argv[i] = nullptr;
								Int32 tt = GeGetTimer();
								MakeDistriCommandLine(prffile,buildid);
								GePrint("Time: " + String::FloatToString((GeGetTimer() - tt) / 1000.0) + " sec.");
							}
							else
								GePrint("\x01-makedistri: missing argument (see --help for help)");
						}
						else
							GePrint("\x01-makedistri: missing argument (see --help for help)");
					}
					else if (!strcmp(args->argv[i],"-diffzips"))
					{
						args->argv[i] = nullptr;

						if (!g_pDiffzipFiles) g_pDiffzipFiles = NewObjClear(CDynamicFilenameSet);
						if (!g_pDiffzipFiles) break;

						for (i++;i<args->argc;i++)
						{
							if (!args->argv[i]) { i--; break; }
							if (args->argv[i][0]=='-') { i--; break; }
							g_pDiffzipFiles->AddFilename(args->argv[i]);
							args->argv[i] = nullptr;
						}
					}
					else if (!strcmp(args->argv[i],"-diffsrc"))
					{
						args->argv[i] = nullptr;

						if (!g_pfnDiffzipNewSrc) g_pfnDiffzipNewSrc = NewObjClear(Filename);
						if (g_pfnDiffzipNewSrc)
						{
							if (i+1<args->argc && args->argv[i+1] && args->argv[i+1][0]!='-')
							{
								i++;
								*g_pfnDiffzipNewSrc = args->argv[i];
								args->argv[i] = nullptr;
							}
						}
					}
					else if (!strcmp(args->argv[i],"-diffdest"))
					{
						args->argv[i] = nullptr;

						GePrint("@Starting ZipDiff");

						if (!g_pfnDiffzipDest) g_pfnDiffzipDest = NewObjClear(Filename);
						if (g_pfnDiffzipDest)
						{
							if (i+1<args->argc && args->argv[i+1] && args->argv[i+1][0]!='-')
							{
								i++;
								*g_pfnDiffzipDest = args->argv[i];
								args->argv[i] = nullptr;

								if (g_pDiffzipFiles && g_pfnDiffzipNewSrc && g_pfnDiffzipDest && g_pDiffzipFiles->GetElementCount()>=1)
								{
									Int32 i;
									Bool DiffZipFiles(const CDynamicFilenameSet &arFiles, const Filename &fnNew, const Filename &fnDestZip, const char* pchPassword);
									GePrint("Base Zips:");
									for (i=0;i<g_pDiffzipFiles->GetElementCount();i++)
									{
										GePrint("  "+(*g_pDiffzipFiles)[i]->GetString());
									}
									GePrint("New Zip: "+(*g_pfnDiffzipNewSrc).GetString());
									GePrint("Difference Zip: "+(*g_pfnDiffzipDest).GetString());
									Bool res = DiffZipFiles(*g_pDiffzipFiles, *g_pfnDiffzipNewSrc, *g_pfnDiffzipDest, nullptr);
									if (res)
										GePrint("ZifDiff successfully finished");
									else
										GePrint("Error: ZifDiff had an error");
								}
								GePrint("@Script End");
								break;
							}
							else
								GePrint("Error: ZifDiff no Input files");
						}
						GePrint("@Script End");
					}
				}
			}
			break;

		case C4DPL_ENDACTIVITY:
			EndActivity();
			break;
	}

	return 0;
}
