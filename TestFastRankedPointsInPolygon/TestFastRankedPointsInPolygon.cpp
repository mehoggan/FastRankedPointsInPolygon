#include <point_search.h>

#include <algorithm>
#include <iostream>
#include <iterator>
#include <string>
#include <sstream>
#include <vector>

#include <windows.h> 
#include <stdio.h>

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

const std::size_t POINT_COUNT = 20;
Point points[POINT_COUNT];
void generateKnownPoints()
{
  int8_t id = 0;
  int32_t rank = 0;
}

bool runDLL(const std::string& dllName)
{
  LPCSTR ptrDllName = dllName.c_str();
  HINSTANCE hinstLib = LoadLibrary(ptrDllName); 
  bool freeResult = false;
  bool runTimeLinkSuccess = true;

  if (hinstLib != nullptr) { 
    CREATEPROC ProcCreate = (CREATEPROC)GetProcAddress(hinstLib, "create");
    SEARCHPROC SearchProc = (SEARCHPROC)GetProcAddress(hinstLib, "search");
    DESTROYPROC DestroyProc = (DESTROYPROC)GetProcAddress(hinstLib, "destroy");
    if (ProcCreate == nullptr ||
      SearchProc == nullptr ||
      DestroyProc == nullptr) {

      std::cerr << "Failed to find create, search, or destroy as exported "
        << " symbols in " << dllName << std::endl;

      runTimeLinkSuccess = false;
    }

    if (runTimeLinkSuccess) {
      // 1. Generate Points
      generateKnownPoints();

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

int main(int argc, char *argv[])
{
  int ret = EXIT_SUCCESS;

  if (argc < 3) {
    std::cerr << "Usage: TestFastRankedPointsInPolygon "
      << "--dlls [comma seperated list of dll names no spaces!!!]"
      << std::endl;
    ret = EXIT_FAILURE;
  } else {
    std::string dllNameString = argv[2];
    std::stringstream ss(dllNameString);
    std::istream_iterator<std::string> begin(ss);
    std::istream_iterator<std::string> end;
    std::vector<std::string> dllNames(begin, end);
    std::for_each(dllNames.begin(), dllNames.end(),
      [&](const std::string& dllName)
      {
        if (!runDLL(dllName)) {
          std::cerr << "Failed to find " << dllName
            << " did not run anything." << std::endl;
        }
        ret = (ret && EXIT_FAILURE);
      });
  }

  return ret;
}
