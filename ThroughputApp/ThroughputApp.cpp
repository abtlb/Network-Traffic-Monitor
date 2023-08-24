#include "framework.h"
#include "ThroughputApp.h"
#include <locale>
#include <codecvt>
#include <string>
#include <string.h>
#include <shellapi.h>
#include "Network.h"
#include <thread>
#include <sstream>
#include <iomanip>
#include <windowsx.h>

#define MAX_LOADSTRING 100

static int cntr = 0;

// Global Variables:
HINSTANCE hInst;                                // current instance
HWND hWnd;// i put that 
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name
NOTIFYICONDATA nid; //notification in system tray

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

void ManagePackets();
std::wstring DoubleToWString(const double& d, const int& precision);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    
    
    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_THROUGHPUTAPP, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_THROUGHPUTAPP));

    // Setup packet manager
    std::thread worker(ManagePackets);

    MSG msg;

    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    worker.join();// wait for the thread to finish
    return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_THROUGHPUTAPP));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+3);//black
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_THROUGHPUTAPP);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Store instance handle in our global variable

   hWnd = CreateWindowW(szWindowClass, L"Allahu zobry", 0,
       10, 10, 100, 100, nullptr, nullptr, hInstance, nullptr);
   SetWindowLong(hWnd, GWL_STYLE, 0);
   SetMenu(hWnd, NULL);

   //Add notification icon to the system tray
   nid = {};
   nid.cbSize = sizeof(nid);
   nid.hWnd = hWnd;
   nid.uID = 1; // Unique ID for the notification icon
   nid.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
   nid.uCallbackMessage = WM_USER + 1; // Custom message ID
   nid.hIcon = LoadIcon(NULL, IDI_INFORMATION); // Icon for the notification
   std::wstring message = L"0";
   wcscpy_s(nid.szTip, message.c_str());// Tooltip text

   Shell_NotifyIcon(NIM_ADD, &nid);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//

const wchar_t* output;
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    static bool isDragging = false;
    static int initialMousePosX, initialMousePosY;
    switch (message)
    {
    case WM_LBUTTONDOWN:
    {
        isDragging = true;
        initialMousePosX = GET_X_LPARAM(lParam);
        initialMousePosY = GET_Y_LPARAM(lParam);
    }
    break;
    case WM_MOUSEMOVE:
    {
        if (isDragging)
        {
            //SetWindowPos(hWnd, HWND_TOP, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), 100, 100, SWP_NOSIZE | SWP_NOZORDER);
            //SetWindowPos(hWnd, HWND_TOP, 500, 500, 100, 100, SWP_NOSIZE | SWP_NOZORDER);
            RECT rect;
            GetWindowRect(hWnd, &rect);
            int mouseDeltaX = GET_X_LPARAM(lParam) - initialMousePosX;
            int mouseDeltaY = GET_Y_LPARAM(lParam) - initialMousePosY;
            int x = rect.left + mouseDeltaX;
            int y = rect.top + mouseDeltaY;
            SetWindowPos(hWnd, HWND_TOPMOST, x, y, 100, 100, SWP_NOSIZE);

        }
    }
    break;
    case WM_LBUTTONUP:
    {
        isDragging = false;
    }
    break;
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Parse the menu selections:
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: Add any drawing code that uses hdc here...
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_PRINT:
    {
        HDC hDC = (HDC)wParam;
        DWORD dwFlags = (DWORD)lParam;

        // Perform custom printing operations
        RECT rect;
        GetClientRect(hWnd, &rect);
        //I draw the text once, remove singleline flag
        DrawText(hDC, output, -1, &rect, DT_CENTER);
    }
    break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

void PrintMessage(const double in, const double out, std::wstring process)
{
    std::wstring message = L"In: " + DoubleToWString(in, 2) + L" KB/s\n";
    message += L"Out: " + DoubleToWString(out, 2) + L" KB/s\n";
    message += L"Max usage process:\n " + process;
    output = message.c_str();

    HDC hdc = GetDC(hWnd);
    SendMessage(hWnd, WM_PRINT, (WPARAM)hdc, 0);
}

void ManagePackets()
{
    PacketManager pm;
    pm.setup();
}

std::wstring DoubleToWString(const double& d, const int& precision)
{
    std::stringstream stream;
    stream << std::fixed << std::setprecision(precision) << d;
    std::string str = stream.str();
    std::wstring res(str.begin(), str.end());
    return res;
}
