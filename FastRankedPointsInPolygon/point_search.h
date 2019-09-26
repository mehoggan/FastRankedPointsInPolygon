#pragma once

#include "ipoint_search.h"

#include <vector>

struct __declspec(dllexport) SearchContext
{
private:
  Point* points_;
  std::size_t size_;

public:
  explicit SearchContext(const std::vector<Point> &points);

  ~SearchContext();

  const Point* const points_const_ref() const;

  Point* const points_ref();

  std::size_t size() const;
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
