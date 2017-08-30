#ifdef __PC

#include <windows.h>
#include <Shlobj.h>
#include <objidl.h>

unsigned short* g_pChInitPath;

void* C4DAlloc(int nSize);
void C4DFree(void* p);

int CALLBACK BrowseCallbackProc(HWND hwnd,UINT uMsg,LPARAM lp, LPARAM pData)
{
		//TRACE( _T("BrowseCallbackProc(0x%x,%d,%p,%p)\n"), hwnd, uMsg, lp, pData );
		TCHAR szDir[MAX_PATH];

		switch(uMsg)
		{
		case BFFM_INITIALIZED:
				//Initial directory is set here
				//GetWindowsDirectory(szDir, MAX_PATH);
				{
						// WParam is true since you are passing a path.
						// It would be false if you were passing a pidl.
						SendMessage(hwnd,BFFM_SETSELECTION,true,(LPARAM)g_pChInitPath);
				}
				break;
		case BFFM_SELCHANGED:
				// Set the status window to the currently selected path.
				if( SHGetPathFromIDList( (LPITEMIDLIST)lp ,szDir ) )
				{
						SendMessage(hwnd,BFFM_SETSTATUSTEXT,0,(LPARAM)szDir);
				}
				break;
		default:
				break;
		}
		return 0;
}

int FileSelWin(char* pInitPathUTF8, char** chRet)
{
	int iRet = 0;
	LPMALLOC pMalloc;
	*chRet = nullptr;
	TCHAR szDir[MAX_PATH];
	int nMBLen;
	char* pCh;
	g_pChInitPath = nullptr;

	int nWideLen = MultiByteToWideChar(CP_UTF8, 0, pInitPathUTF8, -1, nullptr, 0);
	unsigned short* pWideChar = (unsigned short*)C4DAlloc(sizeof(unsigned short) * nWideLen);
	if (MultiByteToWideChar(CP_UTF8, 0, pInitPathUTF8, -1, pWideChar, nWideLen) != nWideLen)
		goto _error;
	g_pChInitPath = pWideChar;

	if (SUCCEEDED(SHGetMalloc(&pMalloc)))
	{
		//TCHAR szTitle[] = __TEXT("Blah blah... this is quite cool stuff.");
		//pChInitPath = (TCHAR*)pInitPath;
		BROWSEINFO bi;
		ZeroMemory( &bi, sizeof( bi ) );
		bi.hwndOwner = nullptr;
		bi.pszDisplayName = nullptr;
		bi.lpszTitle = nullptr;//szTitle;
		bi.pidlRoot = nullptr;
		bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_STATUSTEXT;
		bi.lpfn = BrowseCallbackProc;

		LPITEMIDLIST pidl = SHBrowseForFolder(&bi);
		if (pidl)
		{
			if(SHGetPathFromIDList(pidl, szDir))
			{
				//MessageBox(szDir,"Picked",MB_OK);
			}
			else
			{
				//TRACE( _T("ERROR : SHGetPathFromIDList failed at %s, %d\n"),  __FILE__, __LINE__ );
			}
			pMalloc->Free(pidl);
			pMalloc->Release();
		}
		else
		{
			//TRACE( _T("User selected Cancel\n") );
			goto _error;
		}
	}
	else
	{
		//TRACE( _T("ERROR : SHGetMalloc failed at %s, %d\n"), __FILE__, __LINE__ );
		goto _error;
	}
	nMBLen = WideCharToMultiByte(CP_UTF8, 0, szDir, -1, nullptr, 0, nullptr, nullptr);
	pCh = (char*)C4DAlloc(nMBLen);
	if (WideCharToMultiByte(CP_UTF8, 0, szDir, -1, pCh, nMBLen, nullptr, nullptr) != nMBLen)
		goto _error;
	*chRet = pCh;

	iRet = 1;
_error:
	C4DFree(pWideChar);

	return iRet;
}

#else
void MacFakeFunction()
{
}
#endif
