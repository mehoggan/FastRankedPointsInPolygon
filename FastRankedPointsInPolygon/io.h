#pragma once

#ifndef IO_H_
#define IO_H_

#include <iomanip>
#include <iostream>

#include "ipoint_search.h"

inline std::ostream& operator<<(std::ostream& out, const Rect& rect)
{
  out << "<[" << rect.lx << ", " << rect.ly << "], ["
    << rect.hx << ", " << rect.hy << "]>";
  return out;
}

inline std::ostream& operator<<(std::ostream& out, const Point& point)
{
  out << " id  = " << std::setw(10) << std::setfill(' ')
    << static_cast<int>(point.id)
    << " rank = " << std::setw(20) << std::setfill(' ') << point.rank
    << " x  = " << std::setw(20) << std::setfill(' ')
    << std::setprecision(19) << point.x
    << " y = " << std::setw(20) << std::setfill(' ')
    << std::setprecision(19) << point.y;
  return out;
}

#endif
