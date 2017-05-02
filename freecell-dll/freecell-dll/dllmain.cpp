// ***********************************************************************************************
// 
//       filename:  dllmain.cpp
// 
//    description:  Provides multiple functions used to hack the FreeCell program provided by
//                  Microsoft.
// 
//         author:  Arnold, Zackery
// 
//          class:  CPS 473
//     instructor:  Deep
//     assignment:  Lab 6
// 
//       assigned:  April 10, 2017
//            due:  May 1, 2017
// 
// ***********************************************************************************************

// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"
#include "resource.h"
#include <Windows.h>
#include <stdio.h>


/* typedefs for the functions called from the FreeCell program */
typedef void(*__stdcall NEW_GAME)(int, int);
typedef void(*__stdcall MOVECARDS)(HWND);


/* Helper Function Declarations */
void newAccelerators(HMODULE self);
void promptAutoWin();
void autoWin();
HWND FindMyTopMostWindow();
void nextMoveWins();
void wins1000();
void notInThisGame();
void APIENTRY foo(void);
BOOL InstallHook();


BOOL APIENTRY DllMain(HMODULE hModule,
	DWORD  ul_reason_for_call,
	LPVOID lpReserved
)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:

		// MessageBox for successfull Injection.
		MessageBoxA(NULL, "The DLL has been successfully injected into the freecell process.", "DLL Loaded Successfully", MB_OKCANCEL);

		// Part 1 Solution
		notInThisGame();

		// Part 2 Solution
		wins1000();

		// Part 3 Solution
		nextMoveWins();

		//while (1) {
		//	OutputDebugStringA("Inside Loop.");
		//	if ((GetKeyState(VK_SPACE) & 0x8000)) {// & GetKeyState(VK_CONTROL) & GetKeyState(VK_F6)) & 0x8000) {
		//		OutputDebugStringA("Keys Caught");
		//		autoWin();
		//	}
		//	Sleep(100);
		//}
		// Part 4 Soon Solution
		newAccelerators(hModule);

		if (InstallHook())
			OutputDebugStringA("Failed to set hook.");

		/*CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)InstallHook, (void*)NULL, 0, NULL);*/

		// Part 5 Partial Solution
		promptAutoWin();


	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}

/* This function sets the invalid move box to say "Not in this game."
   This was achieved by determining the address of that text from
   PE studio with a simple formula. The address is written with the
   new string.
*/
void notInThisGame() {

	char buf[200];
	wchar_t wbuf[1000];
	// Get the base address of the program.
	DWORD start = (DWORD)GetModuleHandle(NULL);
	DWORD offset = (DWORD)0x00010C04;
	DWORD address = offset + start;

	sprintf_s(buf, 200, "Base is %X\n", start);
	OutputDebugStringA(buf);

	sprintf_s(buf, 200, "Offset is %X\n", offset);
	OutputDebugStringA(buf);

	sprintf_s(buf, 200, "Address is %X\n", address);
	OutputDebugStringA(buf);

	const size_t length = 36;
	wchar_t buffer[length] = L"Not in this game.";

	wchar_t* bufptr = (wchar_t*)(address);

	swprintf_s(wbuf, sizeof(wbuf) / sizeof(wchar_t), L"Old string at address is %s\n", bufptr);
	OutputDebugStringW(wbuf);

	DWORD old;
	VirtualProtect(bufptr, length, PAGE_EXECUTE_READWRITE, &old);
	memcpy(bufptr, buffer, length);
	VirtualProtect(bufptr, length, old, nullptr);

	swprintf_s(wbuf, sizeof(wbuf)/sizeof(wchar_t), L"New string at address is %s\n", bufptr);
	OutputDebugStringW(wbuf);

}

/* This function sets the number of wins read by the program to 1000.
   This was achieved by editing the program's registry entry to set
   a customized number of wins.
*/
void wins1000() {

	char buf[200];
	wchar_t wbuf[1000];

	HKEY newKey;
	LONG lRes = RegOpenKeyExW(HKEY_CURRENT_USER, 
		L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Applets\\FreeCell",
		0, 
		KEY_READ | KEY_SET_VALUE,
		&newKey);

	if (lRes != ERROR_SUCCESS) {
		OutputDebugStringA("Failed to load registry.");
		sprintf_s(buf, 200, "Error is %d\n", lRes);
		OutputDebugStringA(buf);
		return;
	}
	else
	{
		OutputDebugStringA("Successfully loaded key?");
	}

	OutputDebugStringA("Attempting to read registry.");

	// Buffer for the registry value.
	wchar_t tBuf[512] = { '\0' }; // Text Translation Buffer
	BYTE regBuf[512];  // Buffer to hold values from registry.

	// WINS1000 is the byte array that will set wins to 1000.
	BYTE WINS1000[4] = { 0xE8, 0x03, 0x00, 0x00 };

	DWORD type = REG_BINARY;//(LPDWORD)0x3;
	LPDWORD lpcbData = (LPDWORD)512;
	DWORD dwBufSize = sizeof(regBuf);

	lRes = RegQueryValueExW(newKey, /*bufptr*/TEXT("won"), 0, NULL, (LPBYTE)regBuf, &dwBufSize);
	if (lRes != ERROR_SUCCESS) {
		OutputDebugStringA("Failed to load registry.");
		sprintf_s(buf, 200, "Error is %d\n", lRes);
		OutputDebugStringA(buf);
		return;
	}
	else
	{
		OutputDebugStringA("Successfully read registry?");
		sprintf_s(buf, 200, "dwBufSize is %d\n", dwBufSize);
		OutputDebugStringA(buf);
		for (int i = 0; i < (int)dwBufSize; i++) {
			swprintf_s(tBuf, sizeof(tBuf) / sizeof(wchar_t), L"%s%02X", tBuf, regBuf[i]);
		}
		swprintf_s(wbuf, sizeof(wbuf) / sizeof(wchar_t), L"Reg Value was 0x%s\n", tBuf);
		OutputDebugStringW(wbuf);
	}

	lRes = RegSetKeyValue(newKey, NULL, TEXT("won"), REG_BINARY, WINS1000, 4);
	if (lRes != ERROR_SUCCESS) {
		OutputDebugStringA("Failed to save registry.");
		sprintf_s(buf, 200, "Error is %d\n", lRes);
		OutputDebugStringA(buf);
		return;
	}
	else
	{
		OutputDebugStringA("Successfully set registry to 0xE8030000");
	}
}

/* This function makes it so the next move made by the player instantly wins the game.
   To do this the 'bCheating' flag is set to a value of 2, the same value used by the
   CTRL+SHIFT+F10 cheat code. The next move then instantly wins the game.
*/
void nextMoveWins() {

	char buf[512];

	// Set Number of Cards to 0.
	unsigned int oldCheating, newCheating;
	oldCheating = *(unsigned int *)0x01007130;
	sprintf_s(buf, "oldGameInProg = %x", oldCheating);
	OutputDebugStringA(buf);

	// Set the cheating flag to 2, the same
	// value used by the cheat code.
	*(unsigned int *)0x01007130 = 2;

	// Continue.
	newCheating = *(unsigned int *)0x01007130;
	sprintf_s(buf, "newGameInProg = %x", newCheating);
	OutputDebugStringA(buf);

	return;
}

/* This function, when called, forces the system to automatically
   concede victory to the player. This is accomplished by calling
   the MoveCards(hWnd) function remotely. The number of cards is
   also set to 0 and the move index is set to a nonzero value. This
   tricks the game into thinking the player is moving the last card.
   The game then enters a wins state intstantly. Interestingly,
   this works even if the game has not yet been started.
*/
void autoWin() {

	HWND hWnd = FindMyTopMostWindow();

	char buf[200];
	sprintf_s(buf, 200, "HWND = %X\n", hWnd);
	OutputDebugStringA(buf);

	OutputDebugStringA("Setting Move Index to 0");
	// Set the move index to a nonzero value.
	unsigned int oldMoveIndex, newMoveIndex;
	oldMoveIndex = *(unsigned int *)0x01007864;
	sprintf_s(buf, "_moveindex = %x", oldMoveIndex);
	OutputDebugStringA(buf);
	*(unsigned int *)0x01007864 = 1;
	newMoveIndex = *(unsigned int *)0x01007864;
	sprintf_s(buf, "_moveindex = %x", newMoveIndex);
	OutputDebugStringA(buf);

	OutputDebugStringA("Setting Card Count to 0");
	// Set the card count to a zero (winning) value.
	unsigned int oldCardCount, newCardCount;
	oldCardCount = *(unsigned int *)0x01007800;
	sprintf_s(buf, "_wCardCount = %x", oldCardCount);
	OutputDebugStringA(buf);
	*(unsigned int *)0x01007800 = 0;
	newCardCount = *(unsigned int *)0x01007800;
	sprintf_s(buf, "_wCardCount = %x", newCardCount);
	OutputDebugStringA(buf);
	
	OutputDebugStringA("Calling MoveCards with HWND");
	MOVECARDS mcFn = (MOVECARDS)0x01004FC7;
	mcFn(hWnd);
}

// Function was borrowed from Stack Overflow to find a way to get the HWND
// without having the direct refference.
//
// Source: http://stackoverflow.com/a/1125614/4067133
//
HWND FindMyTopMostWindow()
{
	DWORD dwProcID = GetCurrentProcessId();

	char buf[512];
	sprintf_s(buf, "ProcessID = %d", dwProcID);
	OutputDebugStringA(buf);

	HWND hWnd = GetTopWindow(GetDesktopWindow());
	while (hWnd)
	{
		DWORD dwWndProcID = 0;
		GetWindowThreadProcessId(hWnd, &dwWndProcID);
		if (dwWndProcID == dwProcID)
			return hWnd;
		hWnd = GetNextWindow(hWnd, GW_HWNDNEXT);
	}
	return NULL;
}

/* This function simply gives a box for the user to chose if they want to invoke the
   autoWin function. This will be replaced if keyboard accelerators can be figured out
   before the final submission deadline.
*/
void promptAutoWin() {

	const int result = MessageBoxA(NULL, "Do you want to Auto Win?", "Auto Win Function", MB_YESNO);
	switch (result)
	{
	case IDYES:
		OutputDebugStringA("Auto Win Accepted.");
		autoWin();
	case IDNO:
		OutputDebugStringA("User chose not to auto-win.");
		return;
	}
}

// Global Variables for the hooking procedure.
HHOOK hkb;
HACCEL newFreeMenu;
HACCEL origFreeMenu;

/* This hook callback function is used whenever a new message is received.
   The custom accelerator is used to translate those messages.
*/
LRESULT CALLBACK HookCallback(int nCode, WPARAM wParam, LPARAM lParam)
{
	char buf[200];

	LPMSG lMsg = (LPMSG)lParam;
	//HWND hWnd = FindMyTopMostWindow();
	HWND hWnd;
	hWnd = *(HWND *)0x01008374;

	if (lMsg->message == (VK_SHIFT | VK_F2 | VK_CONTROL))
	{
		OutputDebugStringA("F2 Detected.");
	}

	// Using Custom Accellerator Table Here.
	if (TranslateAccelerator(hWnd, newFreeMenu, lMsg) == 0)
	{
		TranslateMessage(lMsg);
		DispatchMessage(lMsg);
	}
	else {
		sprintf_s(buf, "msg = %x", lMsg->message);
		OutputDebugStringA(buf);

		sprintf_s(buf, "lParam = %x", lMsg->lParam);
		OutputDebugStringA(buf);


		sprintf_s(buf, "wParam = %x", lMsg->wParam);
		OutputDebugStringA(buf);

		if (lMsg->lParam == 0x3C0001)
		{
			autoWin();
			return 0;
		}

		OutputDebugStringA("TranslateMessage Passed");
	}

	// call the next hook in the hook chain. This is nessecary or your hook chain will break and the hook stops
	return CallNextHookEx(hkb, nCode, wParam, lParam);
}

/* This function installs a hook to the WH_GETMESSAGE events.
   For some reason, my own GetMessage() function needs to be
   running for this thread to be able to intercept the messages.
   Whatever...
*/
BOOL InstallHook()
{

	char buf[200];

	OutputDebugStringA("Attempting to Install Hook");
	HWND hWnd = FindMyTopMostWindow();

	DWORD dwProcID = GetCurrentProcessId();
	sprintf_s(buf, "dwProcID = %x", dwProcID);
	OutputDebugStringA(buf);

	DWORD dwThreadID = GetWindowThreadProcessId(hWnd, &dwProcID);
	sprintf_s(buf, "dwThreadID = %x", dwThreadID);
	OutputDebugStringA(buf);


	HWND pgmHwnd;
	pgmHwnd = *(HWND *)0x01008374;
	sprintf_s(buf, "pgnHwnd = %x", pgmHwnd);
	OutputDebugStringA(buf);
	sprintf_s(buf, "myHwnd = %x", hWnd);
	OutputDebugStringA(buf);

	hkb = SetWindowsHookEx(WH_GETMESSAGE, (HOOKPROC)HookCallback, NULL, dwThreadID);

	OutputDebugStringA("Potentially Installed Hook");

	MSG msg;
	BOOL bRet;

	while (bRet = GetMessage(&msg, NULL, 0, 0));

	OutputDebugStringA("On the Other side.");
	if (hkb == NULL)
		return FALSE;
	else
		return TRUE;
}

/* Here lies my attempt to find a solution to the keyboard accelerator problem.
   I made my own accelerator within the freecell-dll.rc file in this solution,
   but calling LoadAccelerators() does not seem to impact the FreeCell process.
   Further investigation will continue from this point...
*/
void newAccelerators(HMODULE self) {

	bool tableDestroyed;
	char buf[200];
	ZeroMemory(buf, 10);

	origFreeMenu = LoadAccelerators(NULL, L"FreeMenu");
	if (origFreeMenu != NULL) {
		OutputDebugStringA("Success Loading origFreeMenu");
		sprintf_s(buf, 200, "Handle points to 0x%X\n", origFreeMenu);
		OutputDebugStringA(buf);

		OutputDebugStringA("Trying to Destroy the Original Table");

		//MSDN NOTES:
		//
		//If the function succeeds, the return value is nonzero. 
		//However, if the table has been loaded more than one call to LoadAccelerators, 
		//the function will return a nonzero value only when DestroyAcceleratorTable 
		//has been called an equal number of times.
		//
		//This was called once in the FreeCell app, and once above. So call it twice.

		/*DestroyAcceleratorTable(origFreeMenu);*/
		if (DestroyAcceleratorTable(origFreeMenu))
		{
			OutputDebugStringA("Successfully destroyed original table.");
			tableDestroyed = true;
		}
	}
	else
	{
		OutputDebugStringA("Failed to load origFreeMenu");
		DWORD error = GetLastError();
		sprintf_s(buf, 200, "Error is %d\n", error);
		OutputDebugStringA(buf);
	}

	newFreeMenu = LoadAccelerators(self, L"FreeMenu");
	if (newFreeMenu != NULL) {
		OutputDebugStringA("Success Loading newFreeMenu");
		sprintf_s(buf, 200, "Handle points to 0x%X\n", newFreeMenu);
		OutputDebugStringA(buf);
	}

}