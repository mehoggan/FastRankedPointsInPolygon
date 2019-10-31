#ifndef IO_H_
#define IO_H_

#include <iomanip>
#include <iostream>
#include <sstream>

#include "ipoint_search.h"

inline std::ostream& operator<<(std::ostream& out, const Rect& rect)
{
  out << "{" << rect.lx << ", " << rect.ly << ", " << rect.hx << ", "
    << rect.hy << "}";
  return out;
}

inline std::ostream& operator<<(std::ostream& out, const Point& point)
{
  out << "{" << static_cast<int>(point.id) << ", " << point.rank << ", "
    << std::setprecision(19) << point.x << ", "
    << std::setprecision(19) << point.y << "}"; 
  return out;

}

inline std::stringstream& operator<<(std::stringstream& ss, const Rect& rect)
{
  ss << "{" << rect.lx << ", " << rect.ly << ", " << rect.hx << ", "
    << rect.hy << "}";
  return ss;
}

inline std::stringstream& operator<<(std::stringstream& ss, const Point& point)
{
  ss << "{" << static_cast<int>(point.id) << ", " << point.rank << ", "
    << std::setprecision(19) << point.x << ", "
    << std::setprecision(19) << point.y << "}"; 
  return ss;
}

#endif
