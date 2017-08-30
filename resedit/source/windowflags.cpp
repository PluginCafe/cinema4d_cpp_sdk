#ifdef _WIN32

#include <windows.h>

void ChangeWindowFlags(void* pHandle)
{
	HWND hWnd = (HWND)pHandle;
	if (!IsWindow(hWnd))
		return;

	int nStyleOffset = GWL_EXSTYLE;

	DWORD dwAdd = WS_EX_TOOLWINDOW;
	DWORD dwRemove = 0;

	DWORD dwStyle = GetWindowLong(hWnd, nStyleOffset);
	DWORD dwNewStyle = (dwStyle & ~dwRemove) | dwAdd;
	if (dwStyle != dwNewStyle)
		SetWindowLong(hWnd, nStyleOffset, dwNewStyle);

	nStyleOffset = GWL_STYLE;
	dwAdd = WS_CAPTION | WS_BORDER | WS_SYSMENU;
	dwRemove = WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_THICKFRAME;
	dwStyle = GetWindowLong(hWnd, nStyleOffset);
	dwNewStyle = (dwStyle & ~dwRemove) | dwAdd;
	if (dwStyle != dwNewStyle)
		SetWindowLong(hWnd, nStyleOffset, dwNewStyle);
	UpdateWindow(hWnd);

	HMENU hMenu = GetSystemMenu(hWnd, false);
	if (hMenu)
	{
		RemoveMenu(hMenu, SC_MINIMIZE, MF_BYCOMMAND);
		RemoveMenu(hMenu, SC_MAXIMIZE, MF_BYCOMMAND);
		RemoveMenu(hMenu, SC_RESTORE, MF_BYCOMMAND);
		//RemoveMenu(hMenu, SC_SIZE, MF_BYCOMMAND);
	}
}

void SetModalResizeFlag(void* pHandle)
{
	HWND hWnd = (HWND)pHandle;
	if (!IsWindow(hWnd))
		return;

	int nStyleOffset = GWL_STYLE;

	DWORD dwAdd = WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX;
	DWORD dwRemove = 0;

	DWORD dwStyle = GetWindowLong(hWnd, nStyleOffset);
	DWORD dwNewStyle = (dwStyle & ~dwRemove) | dwAdd;
	if (dwStyle == dwNewStyle)
		return;
	SetWindowLong(hWnd, nStyleOffset, dwNewStyle);

	UpdateWindow(hWnd);
}

void ChangeWindowWidth(void* pHandle, long lDiffX, long lDiffY)
{
	HWND hWnd = (HWND)pHandle;
	if (!IsWindow(hWnd))
		return;

	WINDOWPLACEMENT wndpl;
	GetWindowPlacement(hWnd, &wndpl);
	wndpl.rcNormalPosition.left = 0;
	wndpl.rcNormalPosition.right -= lDiffX;
	wndpl.rcNormalPosition.bottom -= lDiffY;
	wndpl.flags = SW_MINIMIZE;
	SetWindowPlacement(hWnd, &wndpl);
}

void RedrawWindow(void* pHandle)
{
	HWND hWnd = (HWND)pHandle;
	if (!IsWindow(hWnd))
		return;

	InvalidateRect(hWnd, nullptr, true);
	UpdateWindow(hWnd);
}

char* GetOSEnvironmentVariable(const char* pszVarName, char* pszVar, unsigned int nVarLen)
{
	if (!GetEnvironmentVariable(pszVarName, pszVar, nVarLen))
		return nullptr;
	return pszVar;
}

#else
#include <stdlib.h>

void ChangeWindowFlags(void* pHandle)
{
}

void SetModalResizeFlag(void* pHandle)
{
}

void ChangeWindowWidth(void* pHandle, long lDiffX, long lDiffY)
{
}

void RedrawWindow(void* pHandle)
{
}

char* GetOSEnvironmentVariable(const char* pszVarName, char* pszVar, unsigned int nVarLen)
{
	return getenv(pszVarName);
}


#endif
