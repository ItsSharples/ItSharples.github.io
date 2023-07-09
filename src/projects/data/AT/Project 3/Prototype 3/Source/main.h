#pragma once

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include "WinGDI.h"

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hInstPrev, PSTR cmdline, int cmdshow);

void updateWnd(_In_ MSG&);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
void ExitGame(void) noexcept;

