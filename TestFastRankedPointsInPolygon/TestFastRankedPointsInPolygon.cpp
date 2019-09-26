#include <point_search.h>

#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <limits>
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

struct CLI
{
  bool success_;
  std::vector<std::string> dlls_;
  std::string points_file_path_;

  static bool file_exists(const std::string &file)
  {
    WIN32_FIND_DATA FindFileData;
    HANDLE handle = FindFirstFile(file.c_str(), &FindFileData);
    int found = handle != INVALID_HANDLE_VALUE;
    if(found)  {
       FindClose(handle);
    }
    return found;
  }
};

CLI loadCommandLine(
  int argc,
  char **argv)
{
  CLI ret = { true, {}, "" };

  if (argc < 3) {
        std::cerr << "Usage: TestFastRankedPointsInPolygon "
      << "--dlls [comma seperated list of dll names no spaces!!!] "
      << "--points_file [Absolute windows path to points file] "
      << std::endl;
    ret.success_ = false;
  } else {
    std::cout << "Running command " << std::endl;
    for (int s = 0; s < argc; ++s) {
      std::cout << argv[s];
      if (s != argc - 1) {
        std::cout << " ";
      }
    }
    std::cout << std::endl;
    std::string dllsString(argv[2]);
    std::stringstream ssDLLs(dllsString);
    std::string dll;
    while (std::getline(ssDLLs, dll, ',')) {
      ret.dlls_.push_back(dll);
    }
    std::cout << "dlls = ";
    std::copy(ret.dlls_.begin(), ret.dlls_.end(),
      std::ostream_iterator<std::string>(std::cout, ", "));
    std::cout << std::endl;

    ret.success_ = ret.success_ && !ret.dlls_.empty();

    if (argc == 5) {
      ret.points_file_path_ = argv[4];

      ret.success_ = ret.success_ && CLI::file_exists(ret.points_file_path_);
    } else {
      std::cout << "No points file specified." << std::endl;
    }
  }

  return ret;
}

void loadPoints(const std::string& file_name, std::vector<Point> &outPoints)
{
  std::cout << "Loading points..." << std::endl;
  std::ifstream reader(file_name.c_str());
  if (reader.is_open()) {
    int16_t id;
    int32_t rank;
    float x;
    float y;

    outPoints.clear();

    std::string line;
    while (std::getline(reader, line)) {
      std::stringstream ss(line);
      ss >> id;
      ss >> rank;
      ss >> x;
      ss >> y;
      outPoints.push_back({ static_cast<int8_t>(id), rank, x, y });
    }
  }
  std::cout << "Done loading points." << std::endl;
}

void randomRectFromPoints(const std::vector<Point> &points, Rect& outRect)
{
  std::cout << "Generating rectangle..." << std::endl;
  float maxY = -(std::numeric_limits<float>::max)();
  float minY = +(std::numeric_limits<float>::min)();
  float maxX = -(std::numeric_limits<float>::max)();
  float minX = +(std::numeric_limits<float>::min)();

  std::for_each(points.cbegin(), points.cend(),
    [&](const Point& p)
    {
      if (p.x < minX) {
        minX = p.x;
      }

      if (p.x > maxX) {
        maxX = p.x;
      }

      if (p.y < minY) {
        minY = p.y;
      }

      if (p.y > maxY) {
        maxY = p.y;
      }
    });

  outRect = { minX, minY, maxX, maxY };
  std::cout << "Generated rectangle: lx = " << outRect.lx << " ly = "
    << outRect.ly << " hx = " << outRect.hx << " hy = " << outRect.hy
    << std::endl;
}

bool runDLL(const std::string& dllName, const std::vector<Point> &points)
{
  if (points.empty()) {
    std::cerr << "Failed to load points please specify a file to load from."
      << std::endl;
    return false;
  }

  LPCSTR ptrDllName = dllName.c_str();
  HINSTANCE hinstLib = LoadLibrary(ptrDllName); 
  bool freeResult = false;
  bool runTimeLinkSuccess = true;

  if (hinstLib != nullptr) { 
    CREATEPROC CreateProc = (CREATEPROC)GetProcAddress(hinstLib, "create");
    SEARCHPROC SearchProc = (SEARCHPROC)GetProcAddress(hinstLib, "search");
    DESTROYPROC DestroyProc = (DESTROYPROC)GetProcAddress(hinstLib, "destroy");
    if (CreateProc == nullptr ||
      SearchProc == nullptr ||
      DestroyProc == nullptr) {

      std::cerr << "Failed to find create, search, or destroy as exported "
        << " symbols in " << dllName << std::endl;

      runTimeLinkSuccess = false;
    }

    if (runTimeLinkSuccess) {
      std::cout << "Running " << dllName << "..." << std::endl;

      SearchContext *sc = (*CreateProc)(
        &points[0], &points[points.size() - 1]);

      std::int32_t expected = static_cast<int32_t>(0.1 * points.size());

      Rect rect = { 0.0f, 0.0f, 0.0f, 0.0f };
      Point *answer = new Point[expected];
      randomRectFromPoints(points, rect);
      std::cout << "Searching for top " << expected << " points in "
        << " lx = " << rect.lx << " ly = " << rect.ly
        << " hx = " << rect.hx << " hy = " << rect.hy
    << std::endl;
      int32_t pointsCopied = (*SearchProc)(
        sc, 
        rect,
        expected,
        answer);

      std::cout << "In order points are..." << std::endl;
      for (int32_t point_i = 0; point_i < pointsCopied; ++point_i) {
        Point* p = &(answer[point_i]);
        std::cout << "p:"
          << " id  = " << std::setw(10) << std::setfill(' ')
            << static_cast<int>(p->id)
          << " rank = " << std::setw(20) << std::setfill(' ') << p->rank
          << " x  = " << std::setw(20) << std::setfill(' ')
            << std::setprecision(19) << p->x
          << " y = " << std::setw(20) << std::setfill(' ')
            << std::setprecision(19) << p->y
          << std::endl;
      }

      sc = (*DestroyProc)(sc);
      if (sc != nullptr) {
        std::cerr << "Failed to destroy SearchContext!!!" << std::endl;
      }
    } else {
      std::cerr << "Failed to load create, search, or destroy from "
        << dllName << std::endl;
    }
      
    freeResult = FreeLibrary(hinstLib); 
  }

  return runTimeLinkSuccess && freeResult;
}

int main(int argc, char *argv[])
{
  int ret = EXIT_SUCCESS;

  CLI cli = loadCommandLine(argc, argv);
  if (cli.success_) {
    std::for_each(cli.dlls_.begin(), cli.dlls_.end(),
      [&](const std::string& dllName)
      {
        std::vector<Point> points;
        if (!cli.points_file_path_.empty()) {
          loadPoints(cli.points_file_path_, points);
        }

        bool ran = runDLL(dllName, points);
        if (!ran) {
          std::cerr << "Failed to run " << dllName << std::endl;
          ret = (ret && EXIT_FAILURE);
        } else {
          ret = (ret && EXIT_SUCCESS);
        }
      });
  }
  else {
    ret = EXIT_FAILURE;
  }

  return ret;
}
