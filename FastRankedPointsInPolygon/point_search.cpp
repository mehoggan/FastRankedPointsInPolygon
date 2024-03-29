#include "point_search.h"

#include <algorithm>
#include <cstdint>
#include <fstream>
#include <mutex>
#include <sstream>
#include <string>
#include <vector>

#include "io.h"

///////// Debug /////////
void write_churchill_points_to_file(const Point* begin, const Point* end,
  std::ofstream& out)
{
  out = std::ofstream("C:\\Users\\mehoggan\\Desktop\\churchhill.h",
    std::ios_base::out);
  std::vector<std::string> to_write = {
    "#include <vector>\n",
    "static const std::vector<Point> points = {\n"
  };
  std::for_each(to_write.begin(), to_write.end(),
    [&](const std::string& line)
    {
      out.write(line.c_str(), line.length());
      out.flush();
    });
  std::for_each(begin, end,
    [&](const Point& p)
    {
      std::stringstream ss;
      ss << "  " << p << "," << std::endl;
      out.write(ss.str().c_str(), ss.str().length());
      out.flush();
    });
  to_write = { "};\n" };
  std::for_each(to_write.begin(), to_write.end(),
    [&](const std::string& line)
    {
      out.write(line.c_str(), line.length());
      out.flush();
    });
  out.close();
}

///////// Search Context /////////
SearchContext::SearchContext(cPointPtr points_begin, cPointPtr points_end)
{
  std::ptrdiff_t size = std::distance(points_begin, points_end);
  quad_tree_ = new quad_tree(points_begin, points_end, 5, size / 512);
}

SearchContext::~SearchContext()
{
  delete quad_tree_;
}

quad_tree*& SearchContext::tree()
{
  return quad_tree_;
}

std::ofstream& SearchContext::write()
{
  return write_;
}

///////// Helper Function /////////
__declspec(dllexport) bool __stdcall intersect(
  const Rect& a,
  const Rect& b)
{
  bool ret = true;
  ret &= a.lx <= b.hx;
  ret &= a.hx >= b.lx;
  ret &= a.ly <= b.hy;
  ret &= a.hy >= b.ly;
  return ret;
}

__declspec(dllexport) bool __stdcall intersect_point(
  const Point& a,
  const Rect& b)
{
  bool ret = true;
  ret &= a.x >= b.lx;
  ret &= a.x <= b.hx;
  ret &= a.y >= b.ly;
  ret &= a.y <= b.hy;
  return ret;
}

///////// Interface Functions /////////
__declspec(dllexport) SearchContext* __stdcall create(
  const Point *points_begin,
  const Point *points_end)
{
  SearchContext* sc = nullptr;

  std::ptrdiff_t points_count = std::distance(points_begin, points_end);
  if (points_count > 0) {
    sc = new SearchContext(points_begin, points_end);
  }

  return sc;
}

static int check = 0;

__declspec(dllexport) int32_t __stdcall search(
  SearchContext* sc,
  const Rect rect,
  const int32_t count,
  Point* out_points)
{
  if (sc == nullptr || count <= 0 || out_points == nullptr) {
    return 0;
  }

  int32_t end_i = 0;
  sc->tree()->query(rect, count, end_i, out_points);

  return end_i;
}

__declspec(dllexport) SearchContext* __stdcall destroy(
  SearchContext *sc)
{
  if (sc != nullptr) {
    delete sc;
    sc = nullptr;
  }

  return sc;
}
