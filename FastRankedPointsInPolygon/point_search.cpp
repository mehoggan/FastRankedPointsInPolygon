#include "point_search.h"

#include <stdint.h>

__declspec(dllexport) SearchContext* __stdcall create(
  const Point* points_begin,
  const Point* points_end
)
{
	return nullptr;
}

__declspec(dllexport) int32_t __stdcall search(
  SearchContext* sc,
  const Rect rect,
  const int32_t count,
  Point* out_points
)
{
	return 0;
}

__declspec(dllexport) SearchContext* __stdcall destroy(
  SearchContext* sc
)
{
	return nullptr;
}