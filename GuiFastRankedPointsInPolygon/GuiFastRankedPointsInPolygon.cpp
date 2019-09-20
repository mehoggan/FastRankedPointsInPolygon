// GuiFastRankedPointsInPolygon.cpp : Defines the entry point for the
// application.

#include <algorithm>
#include <ctime>
#include <iterator>
#include <sstream>
#include <string>
#include <vector>

#include "framework.h"
#include "GuiFastRankedPointsInPolygon.h"

bool loadCommandLine(
  LPSTR commandLineArgs,
  std::vector<std::string>& outTokens)
{
  bool ret = true;
  std::string argsString(commandLineArgs);
  std::stringstream ss(argsString);
  std::istream_iterator<std::string> begin(ss);
  std::istream_iterator<std::string> end;
  std::vector<std::string> args(begin, end);

  outTokens.clear();

  if (args.size() != 3) {
    ret = false;
  } else {
    std::string dllsString(args[2]);
    std::stringstream ssDLLs(dllsString);
    std::string dll;
    while (std::getline(ssDLLs, dll, ',')) {
      outTokens.push_back(dll);
    }

    ret = !outTokens.empty();
  }

  return ret;
}

bool runDLL(const std::string& dllName)
{
  LPCSTR ptrDllName = dllName.c_str();
  HINSTANCE hinstLib = LoadLibraryA(ptrDllName); 
  bool freeResult = false;
  bool runTimeLinkSuccess = true;

  if (hinstLib != nullptr) { 
    CREATEPROC ProcCreate = (CREATEPROC)GetProcAddress(hinstLib, "create");
    SEARCHPROC SearchProc = (SEARCHPROC)GetProcAddress(hinstLib, "search");
    DESTROYPROC DestroyProc = (DESTROYPROC)GetProcAddress(hinstLib, "destroy");
    if (ProcCreate == nullptr ||
      SearchProc == nullptr ||
      DestroyProc == nullptr) {
      return false;
    }

    if (runTimeLinkSuccess) {
      /*
       * TODO (mhoggan):
       * 2. Add points to context using Create
       * 2. Do Search
       * 3. Check Result
       * 4. Destroy context
       */
    }
      
    freeResult = FreeLibrary(hinstLib); 
  }

  return runTimeLinkSuccess && freeResult;
}

void loadWellKnownSetOfPoints(std::vector<Point> &points)
{
  points.clear();

  std::srand(std::time(0));

  for (std::size_t id = 0; id < TOTAL_POINTS; ++id) {
    points.push_back({
      static_cast<std::int8_t>(id),
      std::rand(),
      static_cast<float>(std::rand() % szWindowWidth),
      static_cast<float>(std::rand() % szWindowHeight)});
  }
}

void drawPoints(
  const std::vector<Point>& points,
  HDC hdc,
  HWND hWnd,
  PAINTSTRUCT ps)
{
  for (const Point& point : points) {
    SetPixel(hdc, static_cast<int>(point.x), static_cast<int>(point.y),
      RGB(0, 255, 0));
  }
}

int APIENTRY wWinMain(
  _In_ HINSTANCE hInstance,
  _In_opt_ HINSTANCE hPrevInstance,
  _In_ LPWSTR    lpCmdLine,
  _In_ int       nCmdShow
)
{
  UNREFERENCED_PARAMETER(hPrevInstance);
  UNREFERENCED_PARAMETER(lpCmdLine);

  // TODO: Place code here.
  lpCmdLineInternal = GetCommandLineA();

  loadedCommandLine = loadCommandLine(lpCmdLineInternal, dllNames);

  // Initialize global strings
  LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
  LoadStringW(
    hInstance,
    IDC_GUIFASTRANKEDPOINTSINPOLYGON,
    szWindowClass,
    MAX_LOADSTRING);
  MyRegisterClass(hInstance);

  // Perform application initialization:
  if (!InitInstance (hInstance, nCmdShow)) {
    return FALSE;
  }

  HACCEL hAccelTable = LoadAccelerators(
    hInstance,
    MAKEINTRESOURCE(IDC_GUIFASTRANKEDPOINTSINPOLYGON)
  );

  MSG msg;

  // Main message loop:
  while (GetMessage(&msg, nullptr, 0, 0)) {
    if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg)) {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }
  }

  return (int) msg.wParam;
}

//
// FUNCTION: MyRegisterClass()
//
// PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
  WNDCLASSEXW wcex;

  wcex.cbSize = sizeof(WNDCLASSEX);

  wcex.style = CS_HREDRAW | CS_VREDRAW;
  wcex.lpfnWndProc = WndProc;
  wcex.cbClsExtra = 0;
  wcex.cbWndExtra = 0;
  wcex.hInstance = hInstance;
  wcex.hIcon = LoadIcon(
    hInstance,
    MAKEINTRESOURCE(IDI_GUIFASTRANKEDPOINTSINPOLYGON)
  );
  wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
  wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
  wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_GUIFASTRANKEDPOINTSINPOLYGON);
  wcex.lpszClassName = szWindowClass;
  wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

  return RegisterClassExW(&wcex);
}

//
// FUNCTION: InitInstance(HINSTANCE, int)
//
// PURPOSE: Saves instance handle and creates main window
//
// COMMENTS:
//
//      In this function, we save the instance handle in a global variable and
//      create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
  hInst = hInstance; // Store instance handle in our global variable

  HWND hWnd = CreateWindowW(
    szWindowClass,
    szTitle,
    WS_OVERLAPPED | WS_SYSMENU,
    CW_USEDEFAULT,
    0,
    szWindowWidth,
    szWindowHeight,
    nullptr,
    nullptr,
    hInstance,
    nullptr);

  if (!hWnd) {
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
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch (message) {
  case WM_CREATE: {
    if (!loadedCommandLine) {
      const std::string message = "Failed to parse " +
        std::string(lpCmdLineInternal) + ". Expected usage is " +
        "GuiFastRankedPointsInPolygon.exe --dlls " +
        "[comma seperated list no spaces]";
      MessageBoxA(hWnd, message.c_str(), "Command Line Args Fail",
        MB_OK | MB_ICONINFORMATION);
      DestroyWindow(hWnd);
    } else {
      std::string dllS;
      std::for_each(dllNames.begin(), dllNames.end(),
        [&](const std::string& dll)
        {
          dllS += dll;
        });
      MessageBoxA(hWnd, (dllS + " specified.").c_str(),
        "Command Line Args Success", MB_OK | MB_ICONINFORMATION);
    }
    loadWellKnownSetOfPoints(points);
  }
    break;
  case WM_COMMAND: {
    int wmId = LOWORD(wParam);
    // Parse the menu selections:
    switch (wmId)
    {
    case IDM_EXIT: {
      DestroyWindow(hWnd);
    }
      break;
    default: {
      return DefWindowProc(hWnd, message, wParam, lParam);
    }
    }
  }
    break;
  case WM_PAINT: {
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(hWnd, &ps);
    drawPoints(points, hdc, hWnd, ps);
    EndPaint(hWnd, &ps);
    painted = true;
  }
    break;
  case WM_DESTROY: {
    PostQuitMessage(0);
  }
    break;
  default: {
    return DefWindowProc(hWnd, message, wParam, lParam);
  }
  }
  return 0;
}
