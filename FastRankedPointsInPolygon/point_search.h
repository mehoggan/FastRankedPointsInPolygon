#pragma once

#ifndef POINT_SEARCH_H
#define POINT_SEARCH_H

#include "ipoint_search.h"

#include <tuple>
#include <vector>

#include "boost/geometry/index/rtree.hpp"

namespace bg = boost::geometry;
namespace bgi = boost::geometry::index;

struct __declspec(dllexport) SearchContext
{
private:
  static constexpr std::size_t MAX_PARTITION_SIZE = 500;

public:
  typedef bgi::rtree<Point, bgi::linear<MAX_PARTITION_SIZE>> RTree;

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

bool operator<(const Point& pleft, const Point& pright) noexcept
{
  return pleft.rank < pright.rank;
}

#endif
