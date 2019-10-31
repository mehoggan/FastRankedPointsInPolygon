#ifndef POINT_SEARCH_H
#define POINT_SEARCH_H

#include "ipoint_search.h"

#include <iostream>
#include <fstream>
#include <tuple>
#include <vector>

#include "quad_tree.h"

struct __declspec(dllexport) SearchContext
{
public:
  typedef const Point* cPointPtr;
  SearchContext(cPointPtr points_begin, cPointPtr points_end);

  ~SearchContext();

  quad_tree*& tree();

  std::ofstream& write();

private:
  quad_tree* quad_tree_;
  std::ofstream write_;
};

inline bool operator==(const Point& lhs, const Point& rhs)
{
  return lhs.id == rhs.id && lhs.rank == rhs.rank && lhs.x == rhs.x
    && lhs.y == rhs.y;
}

inline bool operator!=(const Point& lhs, const Point& rhs)
{
  return !(lhs == rhs);
}

inline bool operator==(const Rect& lhs, const Rect& rhs)
{
  return lhs.lx == rhs.lx && lhs.ly == rhs.ly && lhs.hx == rhs.hx
    && lhs.hy == rhs.hy;
}

inline bool operator!=(const Rect& lhs, const Rect& rhs)
{
  return !(lhs == rhs);
}

extern "C" __declspec(dllexport) bool __stdcall intersect(
  const Rect& a,
  const Rect& b);

extern "C" __declspec(dllexport) bool __stdcall intersect_point(
  const Point& a,
  const Rect& b);

extern "C" __declspec(dllexport) SearchContext* __stdcall create(
	const Point* points_begin,
	const Point* points_end
);

extern "C" __declspec(dllexport) int32_t __stdcall search(
	SearchContext* sc,
	const Rect rect,
	const int32_t count, Point* out_points);

extern "C" __declspec(dllexport) SearchContext* __stdcall destroy(
	SearchContext* sc
);

inline bool operator<(const Point& pleft, const Point& pright) noexcept
{
  return pleft.rank < pright.rank;
}

#endif
