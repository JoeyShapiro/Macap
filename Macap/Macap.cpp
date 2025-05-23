// Macap.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#define DURATION 500

HHOOK g_hKeyboardHook = NULL;
DWORD start = 0;
bool allowed = false;

// different than KeyboardProc
static LRESULT CALLBACK LowLevelKeyboardProc(int code, WPARAM wParam, LPARAM lParam)
{
	auto event = (PKBDLLHOOKSTRUCT)lParam;

	if (code < 0 || event->vkCode != VK_CAPITAL)
		return CallNextHookEx(g_hKeyboardHook, code, wParam, lParam);

	bool key_up = (event->flags & LLKHF_UP) == LLKHF_UP;
	if (allowed) {
		// should only be for the 2 faked ones
		if (key_up)
			allowed = false;
		return CallNextHookEx(g_hKeyboardHook, code, wParam, lParam);
	} else if (!key_up && start == 0) {
		// new key press, start the timer
		start = event->time;
		return 1;
	} else if (key_up && event->time - start < DURATION) {
		// didnt press long enough. reset
		start = 0;
		return 1;
	} else if (!key_up || event->time - start < DURATION) {
		// repeat or hasnt been long enough
		return 1;
	}
	
	start = 0;
	allowed = true;

	INPUT input[2] = {};
	input[0].type = INPUT_KEYBOARD;
	input[0].ki.wVk = VK_CAPITAL;
	input[0].ki.dwFlags = 0; // key down
	input[1].type = INPUT_KEYBOARD;
	input[1].ki.wVk = VK_CAPITAL;
	input[1].ki.dwFlags = KEYEVENTF_KEYUP;

	SendInput(2, input, sizeof(INPUT));

	return 1;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	g_hKeyboardHook = SetWindowsHookExA(WH_KEYBOARD_LL, LowLevelKeyboardProc, GetModuleHandleA(NULL), 0);
	if (!g_hKeyboardHook)
		return 1;

	// cant use while true
	// need message loop for some reason
	MSG msg;
	while (GetMessageA(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessageA(&msg);
	}

	UnhookWindowsHookEx(g_hKeyboardHook);
}
