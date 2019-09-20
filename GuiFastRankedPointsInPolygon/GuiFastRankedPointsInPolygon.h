#pragma once

#include <point_search.h>

#include <string>
#include <tuple>
#include <vector>

#include "resource.h"

#define MAX_LOADSTRING 100
#define TOTAL_POINTS 50

// Global Variables:
HINSTANCE hInst; // current instance
WCHAR szTitle[MAX_LOADSTRING]; // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING]; // the main window class name
INT szWindowWidth = 800;
INT szWindowHeight = 600;

// Forward declarations of functions included in this code module:
ATOM MyRegisterClass(HINSTANCE hInstance);
BOOL InitInstance(HINSTANCE, int);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

// Application specific variables for handling DLLs and loading command line.
LPSTR lpCmdLineInternal;
std::vector<std::string> dllNames;
bool loadedCommandLine = false;

// Application specific variables for graphics.
bool painted = false;
std::vector<Point> points;
Rect selectionRect;

typedef SearchContext *(__stdcall *CREATEPROC)(
  const Point *points_begin,
  const Point *points_end);
typedef int32_t (_stdcall *SEARCHPROC)(
  SearchContext* sc,
  const Rect rect,
  const int32_t count,
  Point* out_points);
typedef SearchContext* (__stdcall *DESTROYPROC)(
  SearchContext* sc);

/*!
 * @brief A function to load the command line arguments into a easier to parse
 *        vector.
 *
 * @param[in] commandLineArgs: A string obtained by calling GetCommandLineA().
 * @param[out] outTokens: The command line split on spaces.
 */
bool loadCommandLine(
  LPSTR commandLineArgs,
  std::vector<std::string>& dllNames);

/*!
 * @brief A function that dynamically loads a DLL from the current working
 *        directory. Or from the loaders path.
 *
 * @param[in] dllName: The name of the DLL to be passed to LoadLibrary.
 * @param[out] outFunctions: The function required by this exercise.
 * 
 */
bool runDLL(const std::string& dllName);

/*!
 * @brief Load well known set of points at runtime.
 *
 * @param[out] points: The container to fill with points.
 */
void loadWellKnownSetOfPoints(std::vector<Point> &outPoints);

/*!
 * @brief Draws points specified converting floating point space to integer
 *        space.
 *
 * @param[in] points: The set of points to render.
 * @param[in] hdc: The Hardware Device Context to draw to hWnd.
 * @param[in] hWnd: The window to draw to.
 * @param[in] ps: The structrue used to pain to hWnd.
 */
void drawPoints(
  const std::vector<Point>& points,
  HDC hdc,
  HWND hWnd,
  PAINTSTRUCT ps);
