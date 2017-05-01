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

		// Part 4 Soon Solution
		newAccelerators(hModule);

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
	DWORD offset = (DWORD)0x00010C04;//0x0000D404;//0x00007880;//0x0000D404;
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


/* Here lies my attempt to find a solution to the keyboard accelerator problem.
   I made my own accelerator within the freecell-dll.rc file in this solution,
   but calling LoadAccelerators() does not seem to impact the FreeCell process.
   Further investigation will continue from this point...
*/
void newAccelerators(HMODULE self) {

	bool tableDestroyed;

	char buf[200];
	ZeroMemory(buf, 10);

	HACCEL newFreeMenu = LoadAccelerators(self, L"FreeMenu");
	if (newFreeMenu != NULL) {
		OutputDebugStringA("Success Loading newFreeMenu");
		sprintf_s(buf, 200, "Handle points to 0x%X\n", newFreeMenu);
		OutputDebugStringA(buf);


		//HWND hwndMain = FindMyTopMostWindow();
		//HWND hwndDlgModeless = NULL;
		//MSG msg;
		//BOOL bRet;
		//// 
		//// Perform initialization and create a main window. 
		//// 

		//char buf[200];
		//sprintf_s(buf, 200, "HWND = %X\n", hwndMain);
		//OutputDebugStringA(buf);

		//OutputDebugStringA("Attempting to create custom message loop.");
		//while ((bRet = GetMessage(&msg, hwndMain, 0, 0)) != 0)
		//{
		//	OutputDebugStringA("Looping.");
		//	if (bRet == -1)
		//	{
		//		// handle the error and possibly exit
		//		sprintf_s(buf, 200, "Error = %X\n", bRet);
		//		OutputDebugStringA(buf);
		//	}
		//	else
		//	{
		//		if (hwndDlgModeless == (HWND)NULL ||
		//			!IsDialogMessage(hwndDlgModeless, &msg) &&
		//			!TranslateAccelerator(hwndMain, newFreeMenu,
		//				&msg))
		//		{

		//			sprintf_s(buf, 200, "Attempting to translate msg 0x%X", msg);
		//			OutputDebugStringA(buf);

		//			TranslateMessage(&msg);
		//			DispatchMessage(&msg);
		//		}
		//	}
		//}
		//OutputDebugStringA("bRet became 0.");
	}
}






//sprintf_s(buf, 200, "iResult is %d\n", iResult);
//OutputDebugStringA(buf);
//HACCEL origFREEMENU = LoadAccelerators(self, L"FREEMENU");


//SendMessage(HWND_BROADCAST, 0x77, 0, 0);

//HACCEL newFreeMenu = LoadAccelerators(self, L"FreeMenu");
//if (newFreeMenu != NULL) {
//	OutputDebugStringA("Success Loading newFreeMenu");
//	sprintf_s(buf, 200, "Handle points to 0x%X\n", newFreeMenu);
//	OutputDebugStringA(buf);

//	int cCopied = CopyAcceleratorTable(newFreeMenu, NULL, NULL);
//	if (cCopied == 0)
//	{
//		OutputDebugStringA("Could not copy original table.");
//	}
//	else
//	{
//		sprintf_s(buf, 200, "Copied %d\n", cCopied);
//		OutputDebugStringA(buf);

//		LPACCEL tempFreeMenu = (LPACCEL)LocalAlloc(LPTR, cCopied * sizeof(ACCEL));
//		
//		if (tempFreeMenu == NULL)
//		{
//			OutputDebugStringA("Could not allocate new table.");
//		}
//		CopyAcceleratorTable(newFreeMenu, tempFreeMenu, cCopied);
//		for (int i = 0; i < (UINT)cCopied; i++)
//		{
//			if (tempFreeMenu[i].cmd == (WORD)114)
//			{
//				OutputDebugStringA("Found Entry 114.");
//				sprintf_s(buf, 200, "Key Hex Value is 0x%X\n", tempFreeMenu[i].key);
//				OutputDebugStringA(buf);
//				//tempFreeMenu[i].fVirt = FVIRTKEY | FSHIFT | FCONTROL;
//				//tempFreeMenu[i].key = VK_F8;
//			}
//		}
//	}
//}
//else
//{
//	OutputDebugStringA("Failed");
//	DWORD error = GetLastError();
//	sprintf_s(buf, 200, "Error is %d\n", error);
//	OutputDebugStringA(buf);
//}



//HACCEL origFreeMenu = LoadAccelerators(NULL, L"FreeMenu");
//if (origFreeMenu != NULL) {
//	OutputDebugStringA("Success Loading origFreeMenu");
//	sprintf_s(buf, 200, "Handle points to 0x%X\n", origFreeMenu);
//	OutputDebugStringA(buf);

//	OutputDebugStringA("Trying to Destroy the Original Table");

//	//MSDN NOTES:
//	//
//	//If the function succeeds, the return value is nonzero. 
//	//However, if the table has been loaded more than one call to LoadAccelerators, 
//	//the function will return a nonzero value only when DestroyAcceleratorTable 
//	//has been called an equal number of times.
//	//
//	//This was called once in the FreeCell app, and once above. So call it twice.

//	DestroyAcceleratorTable(origFreeMenu);
//	if (DestroyAcceleratorTable(origFreeMenu))
//	{
//		OutputDebugStringA("Successfully destroyed original table.");
//		tableDestroyed = true;
//	}
//}
//else
//{
//	OutputDebugStringA("Failed to load origFreeMenu");
//	DWORD error = GetLastError();
//	sprintf_s(buf, 200, "Error is %d\n", error);
//	OutputDebugStringA(buf);
//}
//
//
//	//LPACCEL *table = (LPACCEL*)origFreeMenu;
//
//	//sprintf_s(buf, 200, "Error is %d\n", *table);
//	//OutputDebugStringA(buf);
//	//for (int i = 0; i < (UINT)8; i++)
//	//{
//	//	OutputDebugStringA("In Loop.");
//	//	if ((*table)[i].cmd == (WORD)114)
//	//	{
//	//		OutputDebugStringA("Found Entry 114.");
//	//		(*table)[i].fVirt = FVIRTKEY | FSHIFT | FCONTROL;
//	//		(*table)[i].key = VK_F8;
//	//	}
//	//}
//
//	//HACCEL newFreeMenu = LoadAccelerators(self, L"FreeMenu");
//	//if (newFreeMenu != NULL) {
//	//	OutputDebugStringA("Success Loading newFreeMenu");
//	//	sprintf_s(buf, 200, "Handle points to 0x%X\n", newFreeMenu);
//	//	OutputDebugStringA(buf);
//	//}
//	//else
//	//{
//	//	OutputDebugStringA("Failed");
//	//	DWORD error = GetLastError();
//	//	sprintf_s(buf, 200, "Error is %d\n", error);
//	//	OutputDebugStringA(buf);
//	//}
//
//	//LPMSG msg = (LPMSG)(unsigned int)114;
//	//int a = TranslateAccelerator(NULL, newFreeMenu, msg);
//	//if (a == NULL) {
//	//	DWORD err = GetLastError();
//	//	sprintf_s(buf, 200, "Translate Returns %d\n", err);
//	//	OutputDebugStringA(buf);
//	//}
//	//else
//	//{
//	//	sprintf_s(buf, 200, "Translate Returns %d\n", a);
//	//	OutputDebugStringA(buf);
//	//}
//	//OutputDebugStringA("Attempting to overwrite old handle.");
//	//int cCopied = CopyAcceleratorTable(newFreeMenu, (LPACCEL)origFreeMenu, 8);
//	//if (cCopied == 0)
//	//{
//	//	OutputDebugStringA("Could not copy original table.");
//	//}
//	//else
//	//{
//	//	sprintf_s(buf, 200, "Copied %d\n", cCopied);
//	//	OutputDebugStringA(buf);
//	//}
//
//	//if (tableDestroyed)
//	//{
//	//	// This should work because separate thread?
//	//	OutputDebugStringA("Attempting to use new table in msg loop.");
//	//	MSG msg;
//	//	while (GetMessage(&msg, NULL, 0, 0))
//	//	{
//	//		OutputDebugStringA("Loop Running.");
//	//		if (!TranslateAccelerator(NULL, newFreeMenu, &msg))
//	//		{
//	//			TranslateMessage(&msg);
//	//			DispatchMessage(&msg);
//	//		}
//	//	}
//	//}
//	//OutputDebugStringA("Creating New Table");
//
//	//int cCopiedEntries = CopyAcceleratorTable(origFREEMENU, NULL, 0);
//	//if (cCopiedEntries == 0)
//	//{
//	//	OutputDebugStringA("Could not copy original table.");
//	//}
//
//	//LPACCEL newFREEMENU = (LPACCEL)LocalAlloc(LPTR, cCopiedEntries * sizeof(ACCEL));
//
//	//if (newFREEMENU == NULL)
//	//{
//	//	OutputDebugStringA("Could not allocate new table.");
//	//}
//	//CopyAcceleratorTable(origFREEMENU, newFREEMENU, cCopiedEntries);
//	//for (int i = 0; i < (UINT)cCopiedEntries; i++)
//	//{
//	//	if (newFREEMENU[i].cmd == (WORD)114)
//	//	{
//	//		OutputDebugStringA("Found Entry 114.");
//	//		newFREEMENU[i].fVirt = FVIRTKEY | FSHIFT | FCONTROL;
//	//		newFREEMENU[i].key = VK_F8;
//	//	}
//	//}
//	//OutputDebugStringA("Changed table address value?");
//	//HACCEL hNewFreeMenu = CreateAcceleratorTableW(newFREEMENU, cCopiedEntries);
//	//OutputDebugStringA("Changed table address value?");
//	//*origFREEMENU = *hNewFreeMenu;
//	//OutputDebugStringA("Changed table address value?");
//	////DestroyAcceleratorTable(origFREEMENU);
//	////OutputDebugStringA("Destroyed original table.");
//	////CopyAcceleratorTable(newFREEMENU, NULL, cCopiedEntries);
//	////HACCEL FREEMENU = CreateAcceleratorTable(newFREEMENU, cCopiedEntries);
//	///*OutputDebugStringA("Attempting to hook new table.");
//	//TranslateAcceleratorW(NULL, FREEMENU, lpMsg);*/
//
//}