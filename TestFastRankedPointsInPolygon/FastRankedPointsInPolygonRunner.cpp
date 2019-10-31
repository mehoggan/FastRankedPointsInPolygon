#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

#include "churchill_data.h"

#include <point_search.h>
#include <io.h>

#include <algorithm>
#include <chrono>
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

typedef std::pair<Rect, std::vector<Point>> SingleResult_t;
typedef std::vector<SingleResult_t> DLLResultsType_t;
typedef std::pair<std::string, DLLResultsType_t> DllResult_t;
typedef std::vector<DllResult_t> CrossDLLResultsType_t;

std::ostream& operator<<(std::ostream& out,
  const CrossDLLResultsType_t& results)
{
  for (std::size_t query_index = 0;
    query_index < results.front().second.size();
    ++query_index) {

    for (std::size_t point_index = 0;
      point_index < results.front().second.front().second.size();
      ++point_index) {

      for (const DllResult_t& result : results) {
        out
          << " query: " << std::setw(5) << query_index
          << " point: " << std::setw(5) << point_index << " "
          << result.second[query_index].second[point_index] << std::endl
          << " dll: " << result.first
          << " rect: " << result.second[query_index].first
          << std::endl;
      }
    }
  }
  return out;
}

std::ostream& operator<<(std::ostream& out, const SingleResult_t& sr)
{
  out << "Rect = " << sr.first << std::endl;
  std::size_t index = 0ull;
  std::for_each(sr.second.begin(), sr.second.end(),
    [&](const Point& point)
    {
      out << std::setw(5) << index << ") " << point << std::endl;
      ++index;
    });
  out << "======================" << std::endl;
  return out;
}

struct CLI
{
  bool success_;
  std::vector<std::string> dlls_;
  std::string points_file_path_;

  static bool file_exists(const std::string &file)
  {
    std::ifstream in(file.c_str());
    bool ret = in.is_open();
    in.close();
    return ret;
  }
};

CLI loadCommandLine(
  int argc,
  char **argv)
{
  CLI ret = { true, {}, "" };

  if (argc < 3) {
        std::cerr << "Usage: TestFastRankedPointsInPolygon "
      << "--dlls [comma seperated list of dll names no spaces!!!]"
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
  }

  return ret;
}

std::function<float(double min, double max)> randf =
  [&](double min, double max)
  {
    return static_cast<float>(min + (rand() / (RAND_MAX / (max - min))));
  };

std::vector<Point> generateRandomPoints(
  std::size_t size = 10000000ull,
  const Rect& rect = {
    -(std::numeric_limits<float>::max)(),
    -(std::numeric_limits<float>::max)(),
    +(std::numeric_limits<float>::max)(),
    +(std::numeric_limits<float>::max)() })
{
  std::vector<Point> points(size);

  for (std::size_t i = 0; i < size; ++i) {
    points[i] = Point{
      static_cast<int8_t>(std::rand()),
      std::rand(),
      randf(rect.lx, rect.hx),
      randf(rect.ly, rect.hy) };
  }

  return points;
}

std::vector<Rect> generateRandRect(
  std::size_t query_count = 1000,
  const Rect& rect = {
    -(std::numeric_limits<float>::max)(),
    -(std::numeric_limits<float>::max)(),
    +(std::numeric_limits<float>::max)(),
    +(std::numeric_limits<float>::max)() })
{
  std::vector<Rect> ret(query_count);

  for (std::size_t i = 0; i < query_count; ++i) {
    float fx1 = randf(rect.lx, rect.hx);
    float fx2 = randf(rect.lx, rect.hx);
    float fy1 = randf(rect.ly, rect.hy);
    float fy2 = randf(rect.ly, rect.hy);

    ret[i] = (Rect{
      (std::min)(fx1, fx2),
      (std::min)(fy1, fy2),
      (std::max)(fx1, fx2),
      (std::max)(fy1, fy2)});

    if (ret[i].lx > ret[i].hx || ret[i].ly > ret[i].hy) {
      throw std::runtime_error("Failed to generate good rect.");
    }
  }

  return ret;
}

void compute_bounds(
  std::vector<Point>::iterator begin,
  std::vector<Point>::iterator end,
  Rect& out_rect)
{
  float max_y = -(std::numeric_limits<float>::max)();
  float min_y = +(std::numeric_limits<float>::max)();
  float max_x = -(std::numeric_limits<float>::max)();
  float min_x = +(std::numeric_limits<float>::max)();

  std::for_each(begin, end,
    [&](const Point& it)
    {
      if (it.x < min_x) min_x = it.x;
      if (it.x > max_x) max_x = it.x;
      if (it.y < min_y) min_y = it.y;
      if (it.y > max_y) max_y = it.y;
      int x = 0;
    });

  out_rect = { std::floor(min_x), std::floor(min_y),
    std::ceil(max_x), std::ceil(max_y) };
}

bool intersect(const Rect& a, const Rect& b)
{
  bool ret = true;
  ret &= a.lx <= b.hx;
  ret &= a.hx >= b.lx;
  ret &= a.ly <= b.hy;
  ret &= a.hy >= b.ly;
  return ret;
}

bool intersect_point(const Point& a, const Rect& b)
{
  bool ret = true;
  ret &= a.x >= b.lx;
  ret &= a.x <= b.hx;
  ret &= a.y >= b.ly;
  ret &= a.y <= b.hy;
  return ret;
}

bool validate(CrossDLLResultsType_t& results)
{
  bool good = true;
  for (std::size_t dll_index = 0;
    dll_index < results.size() - 1 && good;
    ++dll_index) {

    DllResult_t& first_dll = results[dll_index + 0];
    DllResult_t& secon_dll = results[dll_index + 1];

    if (first_dll.second.size() != secon_dll.second.size()) {
      std::cerr << first_dll.first << " differs in the number of queries "
        << "in " << secon_dll.first << std::endl;
      good = false;
    }

    for (std::size_t query_index = 0;
      query_index < first_dll.second.size() && good;
      ++query_index) {

      if (first_dll.second[query_index].second.size() !=
        secon_dll.second[query_index].second.size()) {
        std::cerr << first_dll.first << " differs in the number of points "
          << "in " << secon_dll.first << " at query "
          << query_index << std::endl;
        std::cerr << first_dll.first << " has " <<
          first_dll.second[query_index].second.size() << " " << secon_dll.first
          << " has " << secon_dll.second[query_index].second.size()
          << std::endl;
        good = false;
      }

      if (first_dll.second[query_index].first !=
        secon_dll.second[query_index].first) {
        std::cerr << first_dll.first << " differs in the query rectangle "
          << "used by " << secon_dll.first << " at query "
          << query_index << std::endl;
        good = false;
      }

      for (std::size_t point_index = 0;
        point_index < first_dll.second[query_index].second.size() && good;
        ++point_index) {
        const Point& first_dll_point =
          first_dll.second[query_index].second[point_index];
        const Point& secon_dll_point =
          secon_dll.second[query_index].second[point_index];


        if (first_dll_point != secon_dll_point) {
          std::cerr
            << "Points in the results returned from the query index = "
            << query_index << " point index = " << point_index
            << " do not match between " << first_dll.first << " and "
            << secon_dll.first << std::endl;
          good = false;
          break;
        }
      }
    }
  }
  
  return good;
}

constexpr int32_t EXPECTED_SIZE = 10;

bool runDLL(const std::string& dllName,
  const std::vector<Point> &points,
  const std::vector<Rect> &query_rects,
  DLLResultsType_t& results)
{
  results.clear();

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

      auto start = std::chrono::steady_clock::now();
      SearchContext *sc = nullptr;
      sc = (*CreateProc)(
        &points[0], (points.data() + points.size()));
      auto duration = std::chrono::duration_cast<std::chrono::milliseconds>
        (std::chrono::steady_clock::now() - start);
      std::cout << "Createing SearchContext took "
        << duration.count() << " milliseconds." << std::endl;

      std::vector<double> search_times(query_rects.size());
      std::size_t time_index = 0;
      for (const Rect& rect : query_rects) {
        Point* answer = new Point[EXPECTED_SIZE];
        start = std::chrono::steady_clock::now();
        int32_t pointsCopied = (*SearchProc)(
          sc,
          rect,
          EXPECTED_SIZE,
          answer);
        std::chrono::duration<double, std::nano> nanos = 
          std::chrono::steady_clock::now() - start;
        search_times[time_index] = nanos.count();
        ++time_index;
        results.push_back(std::make_pair(
          rect,
          std::vector<Point>(answer, answer + pointsCopied)));
        delete[] answer;
      }
      double total_time = 0.0f;
      std::for_each(search_times.begin(), search_times.end(),
        [&](double nanos)
        {
          total_time += (nanos / 1000000.0);
        });
      double average_time = total_time /
        static_cast<float>(search_times.size());
      std::cout << "Total time " << total_time << " milliseconds "
        << " average time = " << average_time << " milliseconds."
        << std::endl;

      start = std::chrono::steady_clock::now();
      sc = (*DestroyProc)(sc);
      duration = std::chrono::duration_cast<std::chrono::milliseconds>
        (std::chrono::steady_clock::now() - start);
      std::cout << "Destroy took " << duration.count()
        << " milliseconds." << std::endl;
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

  srand(time(nullptr));

  CLI cli = loadCommandLine(argc, argv);
  if (cli.success_) {

    std::vector<Point> points = (churchill_points.empty()) ?
      std::move(generateRandomPoints(10000)) :
      churchill_points;

    std::vector<Rect> query_rects = (churchill_query_rects.empty()) ?
      std::move(generateRandRect(100)) :
      churchill_query_rects;

    std::cout << "Running " << query_rects.size() << " queries against "
      << points.size() << " points. With expected results to be "
      << " less than or equal to " << EXPECTED_SIZE << std::endl;

    CrossDLLResultsType_t dll_results(cli.dlls_.size());
    Rect global_bounds;
    compute_bounds(points.begin(), points.end(), global_bounds);
    for (const Point& point : points) {
      if (!intersect_point(point, global_bounds)) {
        throw std::runtime_error(
          "Failed to generate point in correct bounds.");
      }
    }
    for (const Rect& rect : query_rects) {
      if (!intersect(rect, global_bounds)) {
        throw std::runtime_error(
          "Failed to generate valid rect.");
      }
    }

    std::size_t result_index = 0;
    std::for_each(cli.dlls_.begin(), cli.dlls_.end(),
      [&](const std::string& dllName)
      {
        DLLResultsType_t results;
        bool ran = runDLL(
          dllName,
          points,
          query_rects,
          results);
        if (!ran) {
          std::cerr << "Failed to run " << dllName << std::endl;
          ret = (ret && EXIT_FAILURE);
        } else {
          dll_results[result_index] = std::move(
            std::make_pair(dllName, results));
          ret = (ret && EXIT_SUCCESS);
          ++result_index;
        }
      });

    bool valid = validate(dll_results);
    if (valid) {
      std::cout << dll_results << std::endl;
    }
    if (!valid) {
      ret = EXIT_FAILURE;
      std::cout << "FAILURE" << std::endl;
    } else {
      std::cout << "SUCCESS" << std::endl;
    }
  } else {
    ret = EXIT_FAILURE;
  }

  _CrtDumpMemoryLeaks();

  return ret;
}
