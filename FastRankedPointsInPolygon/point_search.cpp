#include "point_search.h"

#include <algorithm>
#include <cstdint>
#include <vector>

#include "io.h"

///////// Search Context /////////
SearchContext::SearchContext(cPointPtr points_begin, cPointPtr points_end) :
  quad_tree_(new quad_tree(points_begin, points_end, 5, 75))
{}

SearchContext::~SearchContext()
{
  delete quad_tree_;
}

quad_tree*& SearchContext::tree()
{
  return quad_tree_;
}

///////// Helper Function     /////////
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
  Point* end = nullptr;
  std::function<void(const Point & p)> lambda = [&](const Point& point)
  {
    // If our ranks are all filled in and sorted and we find a new rank
    // greater than our current greatest then we do not need to cosider it.
    if (end_i == count && point.rank > end->rank) {
      return;
    }

    // Binary search for the item that this sits below.
    auto it = std::lower_bound(out_points, out_points + end_i, point);
    int i = std::distance(out_points, it);

    // Scoot everything down one to make room for the new point with
    // the lowest rank.
    Point tmp = out_points[i];
    out_points[i] = point;
    while (i <= end_i) {
      ++i;
      std::swap(tmp, out_points[i]);
    }

    // Update where the end of the current sorted ranks are.
    if (i <= count && i > end_i) {
      end_i = i;
      end = &out_points[(std::min)(end_i, count - 1)];
    }
  };

  // sc->tree()->query(rect, lambda);

  std::cout << "Returning " << end_i << " elements." << std::endl;
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
