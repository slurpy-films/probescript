#pragma once
#include <windows.h>
#include <thread>

class Frame {
public:
    Frame() {
        messageThread = std::thread([this]() {
            run();
        });
    }

    void setVisible(bool isVisible) {
        while (!hwnd) std::this_thread::sleep_for(std::chrono::milliseconds(10));
        ShowWindow(hwnd, isVisible ? SW_SHOW : SW_HIDE);
        UpdateWindow(hwnd);
    }

private:
    HWND hwnd = nullptr;
    std::thread messageThread;

    void run() {
        const wchar_t CLASS_NAME[] = L"Test";

        WNDCLASSW wc = {};
        wc.lpfnWndProc = WindowProc;
        wc.hInstance = GetModuleHandleW(NULL);
        wc.lpszClassName = CLASS_NAME;

        RegisterClassW(&wc);

        hwnd = CreateWindowW(
            CLASS_NAME,
            L"My Window",
            WS_OVERLAPPEDWINDOW,
            CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
            NULL, NULL, GetModuleHandleW(NULL), NULL
        );

        if (!hwnd) {
            MessageBoxA(NULL, "Could not draw window", "Error", MB_OK);
            return;
        }

        UpdateWindow(hwnd);
        ShowWindow(hwnd, SW_HIDE);

        MSG msg = {};
        while (GetMessage(&msg, nullptr, 0, 0)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
        if (uMsg == WM_DESTROY) {
            PostQuitMessage(0);
            return 0;
        }
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
};
