#ifndef POINT_SEARCH_H
#define POINT_SEARCH_H

#include "ipoint_search.h"

#include <iostream>
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

private:
  quad_tree* quad_tree_;
};

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
