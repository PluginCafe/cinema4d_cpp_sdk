#include "stdafx.h"
#include "globals.h"
#include "MakeDistriDialog.h"
#include "crc.h"
#include <stdio.h>

extern PresetArray g_Presets;
#define FILE_HEADER											"ResEdit Distribution Preset"
#define ORIG_PATH												"Original Path="
#define DEST_PATH												"Destination Path="
#define CREATE_ZIP											"Create Zip File="
#define CREATE_ZIP_COMPRESSION					"Zip Compression Level="
#define CHECK_VERSION										"Check Version="
#define PARSE_SYMBOLS										"Parse Symbols="
#define WRITE_BUILD											"Write Build Info="
#define BATCH														"Batch="
#define REMOVE_SCC											"Remove SCC="
#define PASSWORD												"Password="
#define FILTER_STRING										"Filter String="
#define FILTER_CONDITION								"Condition="
#define FILTER_ACTION										"Action="
#define FILTER_RENAME										"Rename="
#define FILTER_FLAG											"Flag="
#define W_FILTER_COND_FN_IS							String("is")
#define W_FILTER_COND_FN_EXT						String("extension")
#define W_FILTER_COND_PATH_CONTAINS			String("path contains")
#define W_FILTER_COND_PATH_IS						String("path is")
#define W_FILTER_COND_DIRECTORY_IS			String("directory is")
#define W_FILTER_ACTION_INCLUDE					String("include")
#define W_FILTER_ACTION_EXCLUDE					String("exclude")
#define W_FILTER_ACTION_RENAME					String("rename")
#define W_FILTER_ACTION_COMPILE_INCL		String("compile and include")
#define W_FILTER_ACTION_MOVE_TO					String("move to")
#define W_FILTER_ACTION_FORCE_COPY			String("force copy")
#define W_FILTER_ACTION_COMPILE_INCL_KILL String("compile, include and kill")
#define W_FILTER_FLAG_SET_XBIT					String("set X-Bit")

void UpdateCRC(Crc32 &crc, const String &str)
{
	Int32 lLen = str.GetCStringLen();
	Char* pchString = NewMem(Char,lLen + 2);
	if (!pchString)
		return;
	str.GetCString(pchString, lLen + 1);
	crc.Update(pchString, lLen);
	DeleteMem(pchString);
}

UInt32 GetPresetCrc(PresetElement* pPreset)
{
	Crc32 crc;
	Int32 l, lCount;

	UpdateCRC(crc, pPreset->strOrigin);
	UpdateCRC(crc, pPreset->strDestination);
	UpdateCRC(crc, pPreset->strName);
	UpdateCRC(crc, pPreset->strPassword);
	crc.Update(&pPreset->bCreateZipFile, sizeof(pPreset->bCreateZipFile));
	crc.Update(&pPreset->lCreateZipCompressionLevel, sizeof(pPreset->lCreateZipCompressionLevel));
	crc.Update(&pPreset->bCheckVersion, sizeof(pPreset->bCheckVersion));
	crc.Update(&pPreset->bParseSymbols, sizeof(pPreset->bParseSymbols));
	crc.Update(&pPreset->bWriteBuildInfo, sizeof(pPreset->bWriteBuildInfo));
	crc.Update(&pPreset->bBatch, sizeof(pPreset->bBatch));
	crc.Update(&pPreset->bRemoveSCC, sizeof(pPreset->bRemoveSCC));
	lCount = pPreset->arFilters.GetElementCount();
	for (l = 0; l < lCount; l++)
	{
		FilterElement* pFilter = pPreset->arFilters[l];
		if (!pFilter)
			continue;
		crc.Update(&pFilter->lCondition, sizeof(pFilter->lCondition));
		crc.Update(&pFilter->lAction, sizeof(pFilter->lAction));
		UpdateCRC(crc, pFilter->str);
		UpdateCRC(crc, pFilter->strRename);
		crc.Update(&pFilter->bSetXBit, sizeof(pFilter->bSetXBit));
	}

	return crc.GetCrc();
}

String EncryptPassword(const String& str)
{
	Int32 lLen = str.GetCStringLen(STRINGENCODING_8BIT);
	if (lLen <= 0)
		return str;

	Int32 lCryptLen = (lLen + 256) / 256;
	lCryptLen *= 256;
	Int32 i;
	Random rnd;
	rnd.Init(97355832);
	Int32 plKey[30];
	char ch[3];
	String strCrypt;

	GeCipher256 c;

	char* pchPassword = NewMemClear(Char,lCryptLen);
	if (!pchPassword)
		return str;
	str.GetCString(pchPassword, lLen + 1, STRINGENCODING_8BIT);
	pchPassword[lLen] = 0;
	for (i = 0; i < lCryptLen; i++)
	{
		rnd.Get01();
		pchPassword[i] ^= (rnd.GetSeed() & 0xff);
	}

	rnd.Init(956612571);
	for (i = 0; i < 30; i++)
	{
		rnd.Get01();
		plKey[i] = rnd.GetSeed();
		lIntel(&(plKey[i]));
	}

	if (!c.Open((UChar*)plKey, sizeof(plKey), true))
	{
		DeleteMem(pchPassword);
		return str;
	}

	for (i = 0; i < lCryptLen; i += 256)
		c.Encrypt((UChar*)&pchPassword[i], 256);

	for (i = 0; i < lCryptLen; i++)
	{
		sprintf_safe(ch, sizeof(ch), "%02x", ((int)pchPassword[i]) & 0xff);
		DebugAssert(ch[2] == 0);
		strCrypt += ch;
	}

	DeleteMem(pchPassword);
	return strCrypt;
}

String DecryptPassword(const String &str)
{
	Int32 lLen = str.GetCStringLen(STRINGENCODING_8BIT);
	if (lLen <= 0)
		return str;

	if (lLen % 512)
	{
		DebugAssert(false);
		return "";
	}

	GeCipher256 c;
	Random rnd;
	Int32 plKey[30];

	char *pchCryptPassword = nullptr, *pchPassword = nullptr;

	pchCryptPassword = NewMemClear(Char,lLen + 2);
	pchPassword = (lLen > 1) ? NewMemClear(Char,lLen / 2) : nullptr;
	if (!pchCryptPassword || !pchPassword)
	{
		DeleteMem(pchCryptPassword);
		DeleteMem(pchPassword);
		return "";
	}

	Int32 i;
	int n;
	str.GetCString(pchCryptPassword, lLen + 1, STRINGENCODING_8BIT);
	lLen /= 2;
	for (i = 0; i < lLen; i++)
	{
		sscanf(&pchCryptPassword[2 * i], "%02x", &n);
		DebugAssert(n >= 0 && n < 256);
		pchPassword[i] = (char)n;
	}

	rnd.Init(956612571);
	for (i = 0; i < 30; i++)
	{
		rnd.Get01();
		plKey[i] = rnd.GetSeed();
		lIntel(&(plKey[i]));
	}

	if (!c.Open((UChar*)plKey, sizeof(plKey), true))
	{
		DeleteMem(pchCryptPassword);
		DeleteMem(pchPassword);
		return "";
	}
	for (i = 0; i < lLen; i += 256)
		c.Decrypt((UChar*)&pchPassword[i], 256);

	rnd.Init(97355832);
	for (i = 0; i < lLen; i++)
	{
		rnd.Get01();
		pchPassword[i] ^= (rnd.GetSeed() & 0xff);
	}

	String strDecrypt = pchPassword;

	DeleteMem(pchCryptPassword);
	DeleteMem(pchPassword);

	return strDecrypt;
}

void ReadString(char *&pszString, String &str)
{
	str = String();
	char* p = pszString;
	Bool bInString = false;
	while (*p != '\0')
	{
		if (*p == '\"')
			bInString = !bInString;
		else if (bInString)
			str += String(1, *p);
		else if (!bInString)
		{
			if (*p != ' ')
				break;
		}
		p++;
	}
	pszString = p;
}

void ReadBool(char *&pszString, Bool &b)
{
	char* p = pszString;
	b = false;
	while (*p != '\0')
	{
		if (*p == '1')
			b = true;
		else if (*p == '0')
			b = false;
		else
		{
			if (*p != ' ')
				break;
		}
		p++;
	}
	pszString = p;
}

void ReadInt32(char *&pszString, Int32 &b)
{
	char* p = pszString;
	b = 0;
	while (*p != '\0')
	{
		if (*p >='0' && *p<='9')
			b = b*10 + (*p-'0');
		else
		{
			if (*p != ' ')
				break;
		}
		p++;
	}
	pszString = p;
}

void WriteLine(CDynamicArray<char>& arLine, BaseFile* pFile)
{
	pFile->WriteBytes(arLine.GetArray(), arLine.GetElementCount());
	pFile->WriteBytes((void*)"\r\n", 2);
	arLine.ResetCounter();
}

Bool ReadLine(BaseFile* pFile, CDynamicArray<char>& arLine)
{
	if (pFile->GetPosition() >= pFile->GetLength())
		return false;

	char ch;
	Bool bEmpty, bEOF = false;

	// read line
	do
	{
		arLine.ResetCounter();
		while (!bEOF)
		{
			if (!pFile->TryReadBytes(&ch, 1))
			{
				if (!arLine.Append('\0')) return false;
				bEOF = true;
			}
			else if (ch == '\r' || ch == '\n')
			{
				if (!arLine.Append('\0')) return false;
				break;
			}
			else
			{
				if (!arLine.Append(ch)) return false;
			}
		}
		bEmpty = (arLine.GetElementCount() == 0);
		if (!bEmpty && arLine[0] == '\0')
			bEmpty = true;
	} while (bEmpty && !bEOF);

	return true;
}

Bool AppendString(CDynamicArray<char>& arLine, const char* pchLine)
{
	while (*pchLine != '\0')
	{
		if (*pchLine == '\"')
		{
			if (!arLine.Append('\"') || !arLine.Append('\"'))
				return false;
		}
		else
		{
			if (!arLine.Append(*pchLine))
				return false;
		}
		pchLine++;
	}
	return true;
}

Bool AppendString(CDynamicArray<char>& arLine, CDynamicArray<char>& arTempString, const String &str)
{
	char *pchTemp;

	Int32 lLen = str.GetCStringLen(STRINGENCODING_7BITHEX);
	if (!arLine.Append('\"')) return false;
	if (!arTempString.SetMinSizeNoCopy(3 + lLen)) return false;
	pchTemp = arTempString.GetArray();
	str.GetCString(pchTemp, lLen + 1, STRINGENCODING_7BITHEX);
	if (!AppendString(arLine, pchTemp)) return false;
	if (!arLine.Append('\"')) return false;

	return true;
}

void WriteElement(PresetElement* pElement)
{
	CDynamicArray <char> arLine, arTempString;
	AutoAlloc <BaseFile> pFile;
	Filename fn = GeGetC4DPath(C4D_PATH_PREFS) + String("ResEdit");
	if (!pFile)
		return;

	String strNewName;
	Int32 n;
	for (n = 0; n < pElement->strName.GetLength(); n++)
	{
		UInt16 w = (UInt16)pElement->strName[n];
#if defined _WINDOWS
		if (('0' <= w && w <= '9') || ('A' <= w && w <= 'z') || w == ' ' || w == '_' || w == (UInt16)'ï¿½' || w == (UInt16)'ï¿½' || w == (UInt16)'ï¿½' ||
				w == (UInt16)'ï¿½' || w == (UInt16)'ï¿½' || w == (UInt16)'ï¿½' || w == (UInt16)'ï¿½')
#elif defined __MAC
		if (('0' <= w && w <= '9') || ('A' <= w && w <= 'z') || w == ' ' || w == '_' || w == (UInt16)'Š' || w == (UInt16)'š' || w == (UInt16)'Ÿ' ||
				w == (UInt16)'€' || w == (UInt16)'…' || w == (UInt16)'†' || w == (UInt16)'§')
#elif defined __LINUX
		if (('0' <= w && w <= '9') || ('A' <= w && w <= 'z') || w == ' ' || w == '_' || w == (UInt16)'Ã¼' || w == (UInt16)'Ã¶' || w == (UInt16)'Ã¤' ||
				w == (UInt16)'Ãœ' || w == (UInt16)'Ã–' || w == (UInt16)'Ã„' || w == (UInt16)'ÃŸ')
#else
#error Wrong operating system
#endif
			strNewName += String(1, w);
		else
			strNewName += String(1, '_');
	}

	if (!AssertPathExists(fn))
	{
		GePrint(String("Could not create path ") + fn.GetString());
		return;
	}

	if (GetPresetCrc(pElement) == pElement->ulCRC && (GeFExist(fn + (strNewName + ".prf")) || GeFExist(GeGetC4DPath(C4D_PATH_RESOURCE) + String("distribution") + (strNewName + ".prf"))))
		return;

	fn += (strNewName + ".prf");
	if (!ForceOpenFileWrite(pFile, fn))
	{
		GePrint(String("Could not write file ") + fn.GetString());
		return;
	}

	AppendString(arLine, FILE_HEADER);
	WriteLine(arLine, pFile);

	AppendString(arLine, ORIG_PATH);
	AppendString(arLine, arTempString, pElement->strOrigin);
	WriteLine(arLine, pFile);

	AppendString(arLine, DEST_PATH);
	AppendString(arLine, arTempString, pElement->strDestination);
	WriteLine(arLine, pFile);

	AppendString(arLine, CREATE_ZIP);
	AppendString(arLine, pElement->bCreateZipFile ? "1" : "0");
	WriteLine(arLine, pFile);

	Char *pszZipLevel = String::IntToString(pElement->lCreateZipCompressionLevel).GetCStringCopy();
	if (pszZipLevel)
	{
		AppendString(arLine, CREATE_ZIP_COMPRESSION);
		AppendString(arLine, pszZipLevel);
		WriteLine(arLine, pFile);
		DeleteMem(pszZipLevel);
	}

	AppendString(arLine, CHECK_VERSION);
	AppendString(arLine, pElement->bCheckVersion ? "1" : "0");
	WriteLine(arLine, pFile);

	AppendString(arLine, PARSE_SYMBOLS);
	AppendString(arLine, pElement->bParseSymbols ? "1" : "0");
	WriteLine(arLine, pFile);

	AppendString(arLine, WRITE_BUILD);
	AppendString(arLine, pElement->bWriteBuildInfo ? "1" : "0");
	WriteLine(arLine, pFile);

	AppendString(arLine, BATCH);
	AppendString(arLine, pElement->bBatch ? "1" : "0");
	WriteLine(arLine, pFile);

	AppendString(arLine, REMOVE_SCC);
	AppendString(arLine, pElement->bRemoveSCC ? "1" : "0");
	WriteLine(arLine, pFile);

	AppendString(arLine, PASSWORD);
	AppendString(arLine, arTempString, EncryptPassword(pElement->strPassword));
	WriteLine(arLine, pFile);

	Int32 lFilterCount = pElement->arFilters.GetElementCount();
	FilterElement** ppFilterElements = pElement->arFilters.GetArray()->GetArray();

	for (n = 0; n < lFilterCount; n++)
	{
		AppendString(arLine, FILTER_STRING);
		AppendString(arLine, arTempString, ppFilterElements[n]->str);
		AppendString(arLine, " " FILTER_CONDITION);
		switch (ppFilterElements[n]->lCondition)
		{
		case FILTER_COND_FN_IS: AppendString(arLine, arTempString, W_FILTER_COND_FN_IS); break;
		case FILTER_COND_FN_EXT: AppendString(arLine, arTempString, W_FILTER_COND_FN_EXT); break;
		case FILTER_COND_PATH_CONTAINS: AppendString(arLine, arTempString, W_FILTER_COND_PATH_CONTAINS); break;
		case FILTER_COND_PATH_IS: AppendString(arLine, arTempString, W_FILTER_COND_PATH_IS); break;
		case FILTER_COND_DIRECTORY_IS: AppendString(arLine, arTempString, W_FILTER_COND_DIRECTORY_IS); break;
		default:
			DebugAssert(false);
		}
		AppendString(arLine, " " FILTER_ACTION);
		switch (ppFilterElements[n]->lAction)
		{
		case FILTER_ACTION_INCLUDE: AppendString(arLine, arTempString, W_FILTER_ACTION_INCLUDE); break;
		case FILTER_ACTION_EXCLUDE: AppendString(arLine, arTempString, W_FILTER_ACTION_EXCLUDE); break;
		case FILTER_ACTION_RENAME: AppendString(arLine, arTempString, W_FILTER_ACTION_RENAME); AppendString(arLine, " " FILTER_RENAME); AppendString(arLine, arTempString, ppFilterElements[n]->strRename); break;
		case FILTER_ACTION_COMPILE_INCL: AppendString(arLine, arTempString, W_FILTER_ACTION_COMPILE_INCL); break;
		case FILTER_ACTION_MOVE_TO: AppendString(arLine, arTempString, W_FILTER_ACTION_MOVE_TO); break;
		case FILTER_ACTION_FORCE_COPY: AppendString(arLine, arTempString, W_FILTER_ACTION_FORCE_COPY); break;
		case FILTER_ACTION_COMPILE_INCL_KILL: AppendString(arLine, arTempString, W_FILTER_ACTION_COMPILE_INCL_KILL); break;
		default:
			DebugAssert(false);
		}
		if (ppFilterElements[n]->bSetXBit)
		{
			AppendString(arLine, " " FILTER_FLAG);
			AppendString(arLine, arTempString, W_FILTER_FLAG_SET_XBIT);
		}
		WriteLine(arLine, pFile);
	}

	pFile->Close();
}

void WriteDistriPrefs()
{
	PresetElement** ppElements = g_Presets.GetArray()->GetArray();
	Int32 n, lCount = g_Presets.GetElementCount();
	for (n = 0; n < lCount; n++)
	{
		//g_Presets.Write(pFile, PRESET_FILE_LEVEL);
		WriteElement(ppElements[n]);
	}
}

static void ReadDistriPrefs(const Filename &fnDir)
{
	AutoAlloc <BrowseFiles> pBrowse;
	if (!pBrowse)
		return;

	Filename fn = fnDir;
	Filename fnPreset;
	pBrowse->Init(fn, false);
	while (pBrowse->GetNext())
	{
		fnPreset = fn + pBrowse->GetFilename();

		ReadPreset(fnPreset);
	}
}

void ReadDistriPrefs()
{
	ReadDistriPrefs(GeGetC4DPath(C4D_PATH_PREFS) + String("ResEdit"));
	ReadDistriPrefs(GeGetC4DPath(C4D_PATH_RESOURCE) + String("distribution"));
}

Bool ReadPreset(const Filename& fnPreset)
{
	AutoAlloc <BaseFile> pFile;
	if (!pFile)
		return false;

	Filename fnTemp;
	PresetElement* pPreset;
	String strName;
	Int32 n;
	CDynamicArray <char> arLine;
	char* pszLine;
	Bool bHeaderRead = false;
	String str;

	fnTemp = fnPreset.GetFile();
	fnTemp.ClearSuffix();
	strName = fnTemp.GetString();
	bHeaderRead = false;

	for (n = 0; n < g_Presets.GetElementCount(); n++)
	{
		if (g_Presets[n]->strName == strName)
			break;
	}
	if (n != g_Presets.GetElementCount())
		return false;

	pPreset = g_Presets.GetNextObject();
	if (!pPreset)
		return false;

	pPreset->strName = strName;
	if (!pFile->Open(fnPreset))
		return false;

	while (ReadLine(pFile, arLine))
	{
		pszLine = arLine.GetArray();

		if (!bHeaderRead)
		{
			if (strstr(pszLine, FILE_HEADER) == pszLine)
			{
				bHeaderRead = true;
				continue;
			}
		}
		if (!bHeaderRead)
			break;

		while (*pszLine != '\0')
		{
			if (strstr(pszLine, ORIG_PATH) == pszLine)
			{
				pszLine += strlen(ORIG_PATH);
				ReadString(pszLine, pPreset->strOrigin);
			}
			if (strstr(pszLine, DEST_PATH) == pszLine)
			{
				pszLine += strlen(DEST_PATH);
				ReadString(pszLine, pPreset->strDestination);
			}
			if (strstr(pszLine, CREATE_ZIP) == pszLine)
			{
				pszLine += strlen(CREATE_ZIP);
				ReadBool(pszLine, pPreset->bCreateZipFile);
			}
			if (strstr(pszLine, CREATE_ZIP_COMPRESSION) == pszLine)
			{
				pszLine += strlen(CREATE_ZIP_COMPRESSION);
				ReadInt32(pszLine, pPreset->lCreateZipCompressionLevel);
			}
			if (strstr(pszLine, CHECK_VERSION) == pszLine)
			{
				pszLine += strlen(CHECK_VERSION);
				ReadBool(pszLine, pPreset->bCheckVersion);
			}
			if (strstr(pszLine, PARSE_SYMBOLS) == pszLine)
			{
				pszLine += strlen(PARSE_SYMBOLS);
				ReadBool(pszLine, pPreset->bParseSymbols);
			}
			if (strstr(pszLine, WRITE_BUILD) == pszLine)
			{
				pszLine += strlen(WRITE_BUILD);
				ReadBool(pszLine, pPreset->bWriteBuildInfo);
			}
			if (strstr(pszLine, REMOVE_SCC) == pszLine)
			{
				pszLine += strlen(REMOVE_SCC);
				ReadBool(pszLine, pPreset->bRemoveSCC);
			}
			if (strstr(pszLine, BATCH) == pszLine)
			{
				pszLine += strlen(BATCH);
				ReadBool(pszLine, pPreset->bBatch);
			}
			if (strstr(pszLine, PASSWORD) == pszLine)
			{
				pszLine += strlen(PASSWORD);
				ReadString(pszLine, pPreset->strPassword);
				pPreset->strPassword = DecryptPassword(pPreset->strPassword);
			}
			if (strstr(pszLine, FILTER_STRING) == pszLine)
			{
				pszLine += strlen(FILTER_STRING);
				FilterElement* pFilter = pPreset->arFilters.GetNextObject();
				if (!pFilter)
					continue;
				ReadString(pszLine, pFilter->str);

				if (strstr(pszLine, FILTER_CONDITION) == pszLine)
				{
					pszLine += strlen(FILTER_CONDITION);
					ReadString(pszLine, str);
					if (str == W_FILTER_COND_FN_IS) pFilter->lCondition = FILTER_COND_FN_IS;
					else if (str == W_FILTER_COND_FN_EXT) pFilter->lCondition = FILTER_COND_FN_EXT;
					else if (str == W_FILTER_COND_PATH_CONTAINS) pFilter->lCondition = FILTER_COND_PATH_CONTAINS;
					else if (str == W_FILTER_COND_PATH_IS) pFilter->lCondition = FILTER_COND_PATH_IS;
					else if (str == W_FILTER_COND_DIRECTORY_IS) pFilter->lCondition = FILTER_COND_DIRECTORY_IS;
					else DebugAssert(false);
				}
				if (strstr(pszLine, FILTER_ACTION) == pszLine)
				{
					pszLine += strlen(FILTER_ACTION);
					ReadString(pszLine, str);
					if (str == W_FILTER_ACTION_INCLUDE) pFilter->lAction = FILTER_ACTION_INCLUDE;
					else if (str == W_FILTER_ACTION_EXCLUDE) pFilter->lAction = FILTER_ACTION_EXCLUDE;
					else if (str == W_FILTER_ACTION_RENAME) pFilter->lAction = FILTER_ACTION_RENAME;
					else if (str == W_FILTER_ACTION_COMPILE_INCL) pFilter->lAction = FILTER_ACTION_COMPILE_INCL;
					else if (str == W_FILTER_ACTION_MOVE_TO) pFilter->lAction = FILTER_ACTION_MOVE_TO;
					else if (str == W_FILTER_ACTION_FORCE_COPY) pFilter->lAction = FILTER_ACTION_FORCE_COPY;
					else if (str == W_FILTER_ACTION_COMPILE_INCL_KILL) pFilter->lAction = FILTER_ACTION_COMPILE_INCL_KILL;
					else DebugAssert(false);
				}
				if (strstr(pszLine, FILTER_RENAME) == pszLine)
				{
					DebugAssert(pFilter->lAction == FILTER_ACTION_RENAME);
					pszLine += strlen(FILTER_RENAME);
					ReadString(pszLine, pFilter->strRename);
				}
				else
					DebugAssert(pFilter->lAction != FILTER_ACTION_RENAME);
				if (strstr(pszLine, FILTER_FLAG) == pszLine)
				{
					pszLine += strlen(FILTER_FLAG);
					ReadString(pszLine, str);
					if (str == W_FILTER_FLAG_SET_XBIT) pFilter->bSetXBit = true;
					else DebugAssert(false);
				}
			}
			//if (pszLastPos != pszLine) always skip until the end of the line
			break;
		}
	}
	pFile->Close();
	pPreset->ulCRC = GetPresetCrc(pPreset);

	return true;
}
