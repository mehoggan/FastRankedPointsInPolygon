#pragma once

#ifndef POINT_SEARCH_H
#define POINT_SEARCH_H

#include "ipoint_search.h"

#include "boost/geometry.hpp"
#include <boost/geometry/geometries/register/point.hpp>
#include "boost/geometry/index/rtree.hpp"

#include <tuple>
#include <vector>

struct __declspec(dllexport) SearchContext
{
private:
  static constexpr std::size_t MAX_PARTITION_SIZE = 50;

public:
  typedef boost::geometry::index::rtree<Point,
    boost::geometry::index::dynamic_rstar> RTree;

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
