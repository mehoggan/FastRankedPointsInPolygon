#include "point_search.h"

#include <stdint.h>

#include <algorithm>
#include <deque>

#include "io.h"

// Globals
thread_local Rect searchBounds;

/*****************************************************************************/
/* To conduct a series of experiments this section was added to find the     */
/* fastest solution based on the overall strategy.                           */
/*****************************************************************************/
enum class Strategy
{
  NAIVE = 0,
  QUADTREE = 1,
};
Strategy strategy = Strategy::NAIVE;

SearchContext* __stdcall naive_create(
  const Point* points_begin,
  const Point* points_end
);

SearchContext* __stdcall quadtree_create(
  const Point* points_begin,
  const Point* points_end
);

int32_t __stdcall naive_search(
  SearchContext* sc,
  const Rect rect,
  const int32_t count, Point* out_points);

int32_t __stdcall quadtree_search(
  SearchContext* sc,
  const Rect rect,
  const int32_t count, Point* out_points);
/*****************************************************************************/

///////// Search Context /////////
SearchContext::SearchContext(Point *points, std::size_t count) :
  points_(points),
  size_(count)
{}

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


///////// Interface Functions /////////
__declspec(dllexport) SearchContext* __stdcall create(
  const Point *points_begin,
  const Point *points_end)
{
  SearchContext* sc = nullptr;
  switch (strategy) {
  case (Strategy::NAIVE): {
    sc = naive_create(points_begin, points_end);
  }
    break;
  case (Strategy::QUADTREE): {
    sc = quadtree_create(points_begin, points_end);
  }
    break;
  }

  searchBounds = bounds(sc);
  std::cout << "Entire bounds is " << searchBounds << std::endl;
  std::cout.flush();

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

  std::cout << std::endl << "Running query for " << rect << std::endl;
  std::cout.flush();

  switch(strategy) {
  case (Strategy::NAIVE): {
    return naive_search(sc, rect, count, out_points);
  }
  case (Strategy::QUADTREE): {
    return quadtree_search(sc, rect, count, out_points);
  }
  }

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
#define inRect(rect, point) \
  bool in = ((point.x >= rect.lx && point.x <= rect.hx) && \
    (point.y >= rect.ly && point.y <= rect.hy));

__declspec(dllexport) Rect __stdcall bounds(
  SearchContext* sc)
{
  if (sc == nullptr) {
    return {};
  }

  float maxY = -(std::numeric_limits<float>::max)();
  float minY = +(std::numeric_limits<float>::min)();
  float maxX = -(std::numeric_limits<float>::max)();
  float minX = +(std::numeric_limits<float>::min)();

  for (std::size_t i = 0; i < sc->size(); ++i) {
    Point& p = sc->points_[i];
      if (p.x < minX) minX = p.x;
      if (p.x > maxX) maxX = p.x;
      if (p.y < minY) minY = p.y;
      if (p.y > maxY) maxY = p.y;
  }

  return { minX, minY, maxX, maxY };
}

// Naive...
SearchContext* __stdcall naive_create(
  const Point* points_begin,
  const Point* points_end)
{
  std::ptrdiff_t points_count = std::distance(points_begin, points_end);

  if (points_count == 0) {
    return nullptr;
  }

  Point* points = new Point[points_count];
  std::memcpy(points, points_begin, points_count * sizeof(Point));
  return new SearchContext(points, points_count);
}

int32_t __stdcall naive_search(
  SearchContext* sc,
  const Rect rect,
  const int32_t count, Point* out_points)
{
  std::deque<Point*> points_in;

  for (std::size_t i = 0; i < sc->size(); ++i) {
    Point& point = sc->points_[i];
    inRect(rect, point);
    if (in) {
      points_in.push_back(&(sc->points_[i]));
    }
  }

  std::sort(points_in.begin(), points_in.end(),
    [&](const Point* p1, const Point* p2)
    {
      return p1->rank < p2->rank;
    });

  std::size_t copied = 0;
  for (;
    copied < (std::min)(points_in.size(), static_cast<size_t>(count));
    ++copied) {
    out_points[copied] = *(points_in[copied]);
  }

  return static_cast<int32_t>(copied);
}

// Quadtree...
SearchContext* __stdcall quadtree_create(
  const Point* points_begin,
  const Point* points_end)
{
  return nullptr;
}

int32_t __stdcall quadtree_search(
  SearchContext* sc,
  const Rect rect,
  const int32_t count, Point* out_points)
{
  return 0;
}