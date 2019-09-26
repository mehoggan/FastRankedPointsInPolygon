#include "point_search.h"

#include <stdint.h>

#include <algorithm>
#include <vector>

/*****************************************************************************/
/* To conduct a series of experiments this section was added to find the     */
/* fastest solution.                                                         */
/*****************************************************************************/
enum class SearchMechanic
{
  NAIVE = 0
};
SearchMechanic searchMech = SearchMechanic::NAIVE;

int32_t __stdcall naive_search(
  SearchContext* sc,
  const Rect rect,
  const int32_t count, Point* out_points);
/*****************************************************************************/

SearchContext::SearchContext(const std::vector<Point> &points) :
  points_(new Point[points.size()]),
  size_(points.size())
{
  std::copy(points.begin(), points.end(), &points_[0]);
}

SearchContext::~SearchContext()
{
  delete [] points_;
  points_ = nullptr;
}

const Point* const SearchContext::points_const_ref() const
{
  return points_;
}


Point* const SearchContext::points_ref()
{
  return points_;
}

std::size_t SearchContext::size() const
{
  return size_;
}

__declspec(dllexport) SearchContext* __stdcall create(
  const Point *points_begin,
  const Point *points_end
)
{
  std::ptrdiff_t points_count = std::distance(points_begin, points_end) + 1;
  std::vector<Point> points(points_count);
  std::size_t index = 0;

  const Point* it = points_begin;
  while (it != points_end + 1) {
      points[index] = *it;
      ++it;
      ++index;
  }
  return new SearchContext(points);
}

__declspec(dllexport) int32_t __stdcall search(
  SearchContext *sc,
  const Rect rect,
  const int32_t count,
  Point *out_points
)
{
  if (sc == nullptr || count <= 0) {
    return 0;
  }

  switch(searchMech) {
  case (SearchMechanic::NAIVE): {
    return naive_search(sc, rect, count, out_points);
  }
    break;
  }

}

__declspec(dllexport) SearchContext* __stdcall destroy(
  SearchContext *sc
)
{
  if (sc != nullptr) {
    delete sc;
    sc = nullptr;
  }

  return sc;
}

int32_t __stdcall naive_search(
  SearchContext* sc,
  const Rect rect,
  const int32_t count, Point* out_points)
{
  std::vector<Point *> points_in_rect;
  std::size_t index = 0;
  std::for_each(
    &(sc->points_ref()[0]),
    &(sc->points_ref()[sc->size()]),
    [&](Point& point)
    {
      bool withInX = (point.x >= rect.lx && point.x <= rect.hx);
      bool withInY = (point.y >= rect.ly && point.y <= rect.hy);
      if (withInX && withInY) {
        points_in_rect.push_back(&point);
        ++index;
      }
    });

  std::sort(points_in_rect.begin(), points_in_rect.end(),
    [&](Point* a, Point* b) {
      return a->rank < b->rank;
    });

  std::vector<Point *> smallest_rank(count);
  for (std::size_t i = 0; i < count; ++i) {
    if (i < points_in_rect.size()) {
      smallest_rank[i] = points_in_rect[i];
    }
  }

  for (std::size_t i = 0; i < count; ++i) {
    out_points[i] = *(smallest_rank[i]);
  }

  return static_cast<int32_t>(smallest_rank.size());
}