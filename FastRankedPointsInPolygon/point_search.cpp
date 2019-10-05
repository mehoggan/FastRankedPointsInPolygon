#include "point_search.h"

#include <algorithm>
#include <bitset>
#include <chrono>
#include <cstdint>
#include <unordered_map>
#include <vector>

#include "io.h"

#include "boost/geometry.hpp"
#include "boost/geometry/geometries/register/point.hpp"

namespace bg = boost::geometry;
namespace bgi = boost::geometry::index;
// Register the point type

BOOST_GEOMETRY_REGISTER_POINT_2D(Point, float, cs::cartesian, x, y)

///////// Search Context /////////
SearchContext::SearchContext(
  const Point* points_begin, const Point* points_end)
{
  auto start = std::chrono::high_resolution_clock::now();
  auto stop = std::chrono::high_resolution_clock::now();
  rtree_ = new RTree(points_begin, points_end);
  auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
    stop - start);
}

SearchContext::~SearchContext()
{
  delete rtree_;
}

SearchContext::RTree& SearchContext::rtree()
{
  return *rtree_;
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

__declspec(dllexport) int32_t __stdcall search(
  SearchContext *sc,
  const Rect rect,
  const int32_t count,
  Point *out_points)
{
  if (sc == nullptr || count <= 0 || out_points == nullptr) {
    return 0;
  }

  std::for_each(out_points, out_points + count,
    [&](Point& p)
    {
      p.rank = (std::numeric_limits<int32_t>::max)();
    });

  boost::geometry::model::box<bg::model::point<float, 2, bg::cs::cartesian>>
    query_rect( { rect.lx, rect.ly }, { rect.hx, rect.hy });
  auto query_it = bgi::qbegin(sc->rtree(), bgi::intersects(query_rect));

  int32_t end_i = 0;
  Point* end = nullptr;
  while (query_it != bgi::qend(sc->rtree())) {
    const Point* p = &(*(query_it));
    if (end != nullptr && p->rank > end->rank) {
      ++query_it;
      continue;
    }

    int32_t i = 0;
    Point tmp = out_points[i];
    while (i < count) {
      const int32_t prank = p->rank;
      const int32_t irank = out_points[i].rank;
      if (prank < irank) {
        Point tmp = out_points[i];
        out_points[i] = *p;
        while (i <= end_i) {
          ++i;
          std::swap(tmp, out_points[i]);
        }
        break;
      }
      ++i;
    }

    if (i <= count && i > end_i) {
      end_i = i;
      end = &out_points[(std::min)(end_i, count - 1)];
    }
    ++query_it;
  }

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
