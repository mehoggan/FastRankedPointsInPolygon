#pragma once

#ifndef POINT_SEARCH_H
#define POINT_SEARCH_H

#include "ipoint_search.h"

#include <tuple>
#include <vector>

#include "boost/geometry/index/rtree.hpp"

struct __declspec(dllexport) SearchContext
{
private:
  static constexpr std::size_t MAX_PARTITION_SIZE = 100;

public:
  typedef boost::geometry::index::rtree<Point,
    boost::geometry::index::quadratic<MAX_PARTITION_SIZE>> RTree;

public:
  SearchContext(const Point *points_begin, const Point *points_end);

  ~SearchContext();

  RTree& rtree();

private:
  RTree* rtree_;
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

#endif
