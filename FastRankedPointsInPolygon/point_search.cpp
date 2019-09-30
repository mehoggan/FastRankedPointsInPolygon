#include "point_search.h"

#include <algorithm>
#include <bitset>
#include <chrono>
#include <cstdint>
#include <unordered_map>
#include <vector>

#include "io.h"

namespace bg = boost::geometry;
namespace bgi = boost::geometry::index;
// Register the point type

BOOST_GEOMETRY_REGISTER_POINT_2D(Point, float, cs::cartesian, x, y)

///////// Search Context /////////
SearchContext::SearchContext(
  const Point *points_begin, const Point *points_end)
{
  auto start = std::chrono::high_resolution_clock::now();
  rtree_ = new RTree(points_begin, points_end,
    boost::geometry::index::dynamic_rstar(MAX_PARTITION_SIZE));
  auto stop = std::chrono::high_resolution_clock::now();
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

  auto start = std::chrono::high_resolution_clock::now();
  std::vector<Point> query_results;
  typedef bg::model::point<float, 2, bg::cs::cartesian> point;
  boost::geometry::model::box<point> query_rect(
    { rect.lx, rect.ly }, { rect.hx, rect.hy });
  sc->rtree().query(boost::geometry::index::intersects(query_rect),
    std::back_inserter(query_results));
  auto stop = std::chrono::high_resolution_clock::now();
  auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
    stop - start);
  // std::cout << std::endl << "It took " << duration.count() << " milliseconds "
  //   << " to query for " << rect << std::endl;

  // TODO: Actually return results.

  return 0;
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

///////// Internal Functions with Implementation /////////
inline bool in_rect(const Rect& rect, const Point& point)
{
  return ((point.x >= rect.lx && point.x <= rect.hx) &&
    (point.y >= rect.ly && point.y <= rect.hy));
}
