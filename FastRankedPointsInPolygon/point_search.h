#pragma once

#ifndef POINT_SEARCH_H
#define POINT_SEARCH_H

#include "ipoint_search.h"

#include <vector>

struct __declspec(dllexport) SearchContext
{
public:
  SearchContext(Point *points, std::size_t count);

  ~SearchContext();

  const Point* const __stdcall points_const_ref() const;

  Point* const __stdcall points_ref();

  std::size_t __stdcall size() const;

  Point* points_;
  std::size_t size_;
};

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

extern "C" __declspec(dllexport) Rect __stdcall bounds(
	SearchContext* sc
);

#endif
