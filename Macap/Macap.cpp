// Macap.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <windows.h>

#define DURATION 2000

HHOOK g_hKeyboardHook = NULL;
DWORD start = 0;
bool allowed = false;

// different than KeyboardProc
LRESULT CALLBACK LowLevelKeyboardProc(int code, WPARAM wParam, LPARAM lParam)
{
	auto event = (PKBDLLHOOKSTRUCT)lParam;

	if (code < 0 || event->vkCode != VK_CAPITAL)
		return CallNextHookEx(g_hKeyboardHook, code, wParam, lParam);

	bool key_up = (event->flags & LLKHF_UP) == LLKHF_UP;
	if (allowed) {
		if (key_up)
			allowed = false;
		return CallNextHookEx(g_hKeyboardHook, code, wParam, lParam);
	}
	else if (event->time-start < DURATION)
		return 1;
	else if (!key_up && start == 0) {
		start = event->time;
		return 1;
	}
	else if (!key_up)
		return 1;
	
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

int main()
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
