#include "point_search.h"

#include <algorithm>
#include <cstdint>
#include <vector>

#include "io.h"

#include "boost/geometry.hpp"
#include "boost/geometry/geometries/register/point.hpp"

// Register the point type
BOOST_GEOMETRY_REGISTER_POINT_2D(Point, float, cs::cartesian, x, y)

///////// Search Context /////////
typedef const Point* cPointPtr;
SearchContext::SearchContext(cPointPtr points_begin, cPointPtr points_end) :
  rtree_(new RTree(points_begin, points_end))
{}

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

  // Set all the ranks to max to assist in sorting in place in out_points.
  std::for_each(out_points, out_points + count,
    [&](Point& p)
    {
      p.rank = (std::numeric_limits<int32_t>::max)();
    });

  // DO THE ACTUAL QUERY!!!
  boost::geometry::model::box<bg::model::point<float, 2, bg::cs::cartesian>>
    query_rect( { rect.lx, rect.ly }, { rect.hx, rect.hy });
  auto query_it = bgi::qbegin(sc->rtree(), bgi::intersects(query_rect));

  // Process the results.
  int32_t end_i = 0;
  Point* end = nullptr;
  while (query_it != bgi::qend(sc->rtree())) {
    const Point* p = &(*(query_it));

    // If our ranks are all filled in and sorted and we find a new rank
    // greater than our current greatest then we do not need to cosider it.
    if (end_i == count && p->rank > end->rank) {
      ++query_it;
      continue;
    }

    auto it = std::lower_bound(out_points, out_points + end_i, *p);
    int i = std::distance(out_points, it);
    Point tmp = out_points[i];
    out_points[i] = *p;
    while (i <= end_i) {
      ++i;
      std::swap(tmp, out_points[i]);
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
