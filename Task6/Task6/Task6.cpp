﻿// Task_5.cpp : Определяет точку входа для приложения.
//

#include "framework.h"
#include "Task6.h"
#include "Render.h"
#include <dxgi.h>
#include <d3d11.h>
#include <wrl/client.h>
#include <iostream>

using Microsoft::WRL::ComPtr;

#pragma comment (lib, "d3d11.lib")
#pragma comment (lib, "dxgi.lib")

#define MAX_LOADSTRING 100

WCHAR szTitle[MAX_LOADSTRING] = L"Task6 Solomatov Alex.";
WCHAR szWindowClass[MAX_LOADSTRING] = L"Task6";

ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);

std::unique_ptr<Render> g_Render;

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR    lpCmdLine,
    _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    MyRegisterClass(hInstance);

    if (!InitInstance(hInstance, nCmdShow))
    {
        OutputDebugString(_T("Error in Init\n"));
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_TASK6));

    MSG msg = {};
    while (msg.message != WM_QUIT)
    {
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else
        {
            if (g_Render && (!IsIconic(g_Render->GetHWND())))
            {
                /*OutputDebugString(_T("Render\n"));*/
                g_Render->RenderStart();
            }
        }
    }

    if (g_Render) {
        g_Render->Terminate();
        g_Render.reset();
    }

    return (int)msg.wParam;
}

ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex = {};

    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.hInstance = hInstance;
    wcex.hbrBackground = NULL;
    wcex.lpszClassName = szWindowClass;

    return RegisterClassExW(&wcex);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    RECT rc = { 0, 0, 800, 600 };
    AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, TRUE);

    HWND hWnd = CreateWindowW(
        szWindowClass,
        szTitle,
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        0,
        rc.right - rc.left,
        rc.bottom - rc.top,
        nullptr,
        nullptr,
        hInstance,
        nullptr
    );

    if (!hWnd)
    {
        OutputDebugString(_T("Error in Create window\n"));
        return FALSE;
    }

    g_Render = std::make_unique<Render>(hWnd);
    if (FAILED(g_Render->Init(szTitle, szWindowClass)))
    {
        OutputDebugString(_T("Error in Init Renderer\n"));
        g_Render.reset();
        return FALSE;
    }

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    return TRUE;
}

void HandleResize(WPARAM wParam)
{
    if (g_Render != nullptr && wParam != SIZE_MINIMIZED)
    {
        g_Render->Resize();
    }
}

bool IsCameraControlKey(WPARAM wParam)
{
    switch (wParam)
    {
    case 'W':
    case 'S':
    case 'A':
    case 'D':
    case VK_UP:
    case VK_DOWN:
    case VK_LEFT:
    case VK_RIGHT:
    case VK_ADD:
    case VK_SUBTRACT:
        return true;
    default:
        return false;
    }
}

void HandleKeyDown(WPARAM wParam)
{
    if (IsCameraControlKey(wParam))
    {
        g_Render->UpdateCamera(wParam);
    }
    else if (wParam == VK_ESCAPE)
    {
        PostQuitMessage(0);
    }
}



LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_SIZE:
        HandleResize(wParam);
        return 0;

    case WM_KEYDOWN:
        HandleKeyDown(wParam);
        return 0;

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
}


