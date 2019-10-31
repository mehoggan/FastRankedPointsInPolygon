#ifndef QUAD_TREE_H
#define QUAD_TREE_H

#include <cstddef>
#include <cstdint>
#include <functional>
#include <iostream>
#include <list>
#include <sstream>
#include <tuple>
#include <vector>

#include "ipoint_search.h"

/// <summary>
/// To encrease the accuracy of subdivision and to allow for further depths
/// in a quad_tree.
/// </summary>
struct DoubleRect
{
  double lx;
  double ly;
  double hx;
  double hy;
};

inline std::ostream& operator<<(std::ostream& out, const DoubleRect& rect)
{
  out << "{" << rect.lx << ", " << rect.ly << ", " << rect.hx << ", "
    << rect.hy << "}";
  return out;
}

inline std::stringstream& operator<<(
  std::stringstream& ss,
  const DoubleRect& rect)
{
  ss << "{" << rect.lx << ", " << rect.ly << ", " << rect.hx << ", "
    << rect.hy << "}";
  return ss;
}

/// <summary>
/// A quad_tree class for spatially partioning points. Each node in the tree
/// is encoded using Morton Encoding. The depths 0, 1, and 2 can be visualized
/// as:
/// |-----------------------------|
/// |                             |
/// |                             |
/// |                             |
/// |                             |
/// |                             |
/// |              1              |
/// |                             |
/// |                             |
/// |                             |
/// |                             |
/// |                             |
/// |-----------------------------|
///
/// |-----------------------------|
/// |              |              |
/// |              |              |
/// |      6       |      7       |
/// |              |              |
/// |              |              |
/// |--------------|--------------|
/// |              |              |
/// |              |              |
/// |      4       |      5       |
/// |              |              |
/// |              |              |
/// |-----------------------------|
///
/// |-----------------------------|
/// | 26   |  27   |  30  |  31   |
/// |      |       |      |       |
/// |------|-------|------|-------|
/// | 24   |  25   |  28  |  29   |
/// |      |       |      |       |
/// |------|-------|------|-------|
/// | 18   |  19   |  22  |  23   |
/// |      |       |      |       |
/// |------|-------|------|-------|
/// | 16   |  17   |  20  |  21   |
/// |      |       |      |       |
/// |-----------------------------|
/// </summary>
class __declspec(dllexport) quad_tree
{
public:
  /// <summary>
  /// Return the index (0 based) of the most significatnt bit.
  /// </summary>
  /// <param name="x">The bits from with to search.</param>
  /// <returns>
  /// -1 if x == 0 or the index of the most significant bit up to index 58.
  /// </returns>
  static int8_t __stdcall msb64(register uint64_t x);

  /// <summary>
  /// Takes <paramref name="x"/> and spreads its bits appart by one space.
  /// For example <paramref name="x"/> == 0x6 (0110b) would result in
  /// 00101000b.
  /// </summary>
  /// <param name="x">The bit field to spread.</param>
  /// <returns><paramref name="x"/>'s bits spread by 1 bit.</returns>
  static uint64_t __stdcall spread_by_1_bit(int64_t x);

  /// <summary>
  /// Takes <paramref name="x"/> and compacts its bits by one space.
  /// For example <paramref name="x"/> == 00101000b would result in
  /// 0110b.
  /// </summary>
  /// <param name="x"></param>
  /// <returns></returns>
  static int64_t __stdcall compact_by_1_bit(int64_t x);

  /// <summary>
  /// The maximum depath a quad_tree can go to.
  /// </summary>
  /// <returns>29 always because anything deeper becomes innacturate.</returns>
  static uint8_t __stdcall max_depth();

  /// <summary>
  /// Given a point in floating point space this function return a morton
  /// encoded representation of that point. Used to avoid having to do bounds
  /// checks all the time to find out which node a <see cref="Point"/> belongs
  /// to.
  /// </summary>
  /// <param name="p">The <see cref="Point"/></param>
  /// <param name="depth">
  /// The depth into the quad_tree for which we are trying to generate a hash
  /// for.
  /// </param>
  /// <param name="bounds">
  /// The lower left and upper right corners that define the bounds for
  /// all points to be hashed.
  /// </param>
  /// <returns>A morton encoded representation of that point.</returns>
  static uint64_t __stdcall compute_quad_key(
    const Point& p,
    uint8_t depth,
    const DoubleRect &bounds);

  /// <summary>
  /// Each depth in the quad_tree has a minimum and maximum uint64_t
  /// morton encoded value. For examle at depth 0
  /// <see cref="quad_tree::min_id()"/> and <see cref="quad_tree::min_id()"/>
  /// are both 1. At depth  depth 1 they are 4 and 7 respectivly. At depth
  /// 2 they are 16 and 31 respectivly.
  /// </summary>
  /// <param name="depth">
  /// The depth at which we wish to know the minimum value.
  /// </param>
  /// <returns>
  /// The minimum morton encoding at <paramref name="depth"/>
  /// </returns>
  static uint64_t __stdcall min_id(uint8_t depth);

  /// <summary>
  /// Each depth in the quad_tree has a minimum and maximum uint64_t
  /// morton encoded value. For examle at depth 0
  /// <see cref="quad_tree::min_id()"/> and <see cref="quad_tree::min_id()"/>
  /// are both 1. At depth  depth 1 they are 4 and 7 respectivly. At depth
  /// 2 they are 16 and 31 respectivly.
  /// </summary>
  /// <param name="depth">
  /// The depth at which we wish to know the maximum value.
  /// </param>
  /// <returns>
  /// The maximum morton encoding at <paramref name="depth"/>
  /// </returns>
  static uint64_t __stdcall max_id(uint8_t depth);

  /// <summary>
  /// Is not valid if quad_key == 0 || quad_key > max_val, where max_val is:
  ///   uint64_t max_bit = (1ull << (2ull * max_depth()));
  ///   uint64_t max_val = max_bit | (max_bit - 1);
  /// </summary>
  /// <param name="quad_key">The morton encoded value to check.</param>
  /// <returns></returns>
  static bool __stdcall is_valid(uint64_t quad_key);

  typedef uint64_t Children_t[4];
  /// <summary>
  /// By definition each child up to nodes at max_depth have children.
  /// The max number of children is 4 and the minimum is 0. If quad_key is
  /// not valid, <see cref="quad_tree::is_valid"/>, then an exception is
  /// thrown. The same occurs if parent is alread at maximum depth in the
  /// tree.
  /// </summary>
  /// <param name="parent">
  /// The morton encoded quad_key to get children for.
  /// </param>
  /// <param name="children">The ouptu of this function.</param>
  /// <returns></returns>
  static void __stdcall compute_children(uint64_t parent,
    Children_t& children) throw(std::runtime_error);

  /// <summary>
  /// By definition 4 quad_keys share the same parent, except for at depth 0
  /// which has no parent. If quad_key is
  /// not valid, <see cref="quad_tree::is_valid"/>, then an exception is
  /// thrown. The same occurs if parent is alread at minimum depth in the
  /// tree, depth 0. 
  /// </summary>
  /// <param name="child">
  /// The morton encoded quad_key to get parent for.
  /// </param>
  /// <returns></returns>
  static uint64_t __stdcall compute_parent(uint64_t child);

  static void __stdcall compute_bounds_for_quad_key(
    uint64_t quad_key,
    const DoubleRect& global_bounds,
    DoubleRect& out_bounds) throw(std::runtime_error);

  /// <summary>
  /// To encode in morton code we map two floating point numbers to
  /// a morton code represented by a square region which is a subset of
  /// the original region. The distance in intgere space from one square
  /// edge to another is computed by this function.
  /// </summary>
  /// <param name="depth">
  /// The depth at which we desire to know the step size.
  /// </param>
  /// <returns>
  /// The intgral step size of a cell at <paramref name="depth"/>
  /// </returns>
  static uint32_t __stdcall step_size_at_depth(uint8_t depth);

private:
  struct node
  {
    enum class ChildId {
      LowerLeft = 0,
      LowerRight = 1,
      UpperLeft = 2,
      UpperRight = 3
    };

    __stdcall node(uint64_t quad_key, const DoubleRect &point_bounds);

    __stdcall ~node();

    void __stdcall set_data(
      std::vector<Point *>::iterator begin,
      std::vector<Point *>::iterator end);

    void __stdcall set_child(const ChildId id, node* child);

    uint64_t quad_key_;
    std::vector<Point> points_;
    node* children_[4];
    DoubleRect point_bounds_;
  };

  typedef std::tuple<uint64_t, std::vector<Point*>, uint64_t> Bucket_t[4];

public:
  constexpr static std::size_t MAX_BLOCK_SIZE = 1000ull;
  constexpr static std::size_t MIN_BLOCK_SIZE = 10ull;

  /// <summary>
  /// Constructor for a quad_tree created from a contiguous block of
  /// <see cref="Point"/> between the addresses stored by
  /// <paramref name="point_begin"/> and <paramref name="point_end"/>
  /// </summary>
  /// <param name="point_begin">
  /// The first Point in the array of points.
  /// </param>
  /// <param name="point_end">
  /// One address past the end Point in the array of points.
  /// </param>
  /// <param name="min_block_size">Not used</param>
  /// <param name="max_block_size">
  /// The maximum number of cells allowed in a <see cref="quad_tree::node"/>
  /// </param>
  __stdcall quad_tree(
    const Point* point_begin,
    const Point* point_end,
    const std::size_t min_block_size = MIN_BLOCK_SIZE,
    const std::size_t max_block_size = MAX_BLOCK_SIZE);

  /// <summary>
  /// Constructor for a quad_tree created from a contiguous block of
  /// <see cref="Point"/> between the addresses stored by
  /// <paramref name="point_begin"/> and <paramref name="point_end"/>
  /// </summary>
  /// <param name="point_begin">
  /// The first Point in the array of points.
  /// </param>
  /// <param name="point_end">
  /// One address past the end Point in the array of points.
  /// </param>
  /// <param name="min_block_size">Not used</param>
  /// <param name="max_block_size">
  /// The maximum number of cells allowed in a <see cref="quad_tree::node"/>
  /// </param>
  __stdcall quad_tree(
    std::vector<Point *>::iterator begin,
    std::vector<Point *>::iterator end,
    const std::size_t min_block_size = MIN_BLOCK_SIZE,
    const std::size_t max_block_size = MAX_BLOCK_SIZE);

  /// <summary>
  /// Destroys a quad_tree.
  /// </summary>
  __stdcall ~quad_tree();

  /// <summary>
  /// When a quad_tree is constructed it finds the smallest axis aligned
  /// bounding box for all points provided. This function retrieves that
  /// bounds.
  /// </summary>
  /// <returns></returns>
  const DoubleRect& __stdcall global_bounds() const;

  /// <summary>
  /// This is how users find out what points are contained within the tree.
  /// </summary>
  /// <param name="query_rect">
  /// The query_rect
  /// </param>
  /// <param name="contained_callback">
  /// For each point that is intersected by <paramref name="query_rect"/>
  /// this function is called with a reference to that point.
  /// </param>
  /// <returns></returns>
  void __stdcall query(const Rect& query_rect,
    std::function<void(const Point & point)> contained_callback);

  /// <summary>
  /// Computes the number of points stored within the tree. O(log4 (N)) where
  /// N is the number of <see cref="quad_tree::node"/> s.
  /// </summary>
  /// <returns></returns>
  std::size_t __stdcall size() const;

public:
  /// <summary>
  /// This function finds the smallest axis aligned bounding box for
  /// a set of points stored between <paramref name="begin"/> and
  /// <paramref name="end"/>
  /// </summary>
  /// <param name="begin">
  /// A stl vector iterator to the first point in a vector of
  /// <see cref="Point"/>.
  /// </param>
  /// <param name="end">
  /// A stl vector iterator to one past the end point in a vector of
  /// <see cref="Point"/>.
  /// </param>
  /// <param name="out_rect">Teh smallest axis aligned bounding box.</param>
  static void __stdcall compute_bounds(
    std::vector<Point *>::iterator begin,
    std::vector<Point *>::iterator end,
    DoubleRect& out_rect);

private:
  inline std::size_t __stdcall compute_points_size(const Point* start_point,
    const Point* end_point)
  {
    return std::distance(start_point, end_point);
  }

  void __stdcall create(
    std::vector<Point*>::iterator begin,
    std::vector<Point*>::iterator end,
    const std::size_t min_block_size = MIN_BLOCK_SIZE,
    const std::size_t max_block_size = MAX_BLOCK_SIZE);

  void __stdcall build_tree(node* node,
    std::vector<Point *>::iterator begin,
    std::vector<Point *>::iterator end,
    uint8_t depth,
    const std::size_t min_block_size,
    const std::size_t max_block_size);

  void __stdcall traverse_tree_by_bounds(
    node* curr,
    const DoubleRect& bounds,
    std::function<void(const Point & point)> contained_callback);

  void __stdcall print_tree(node* curr);

  void __stdcall destroy_tree(node* curr);

  void __stdcall size_recursive(node* curr, std::size_t& count) const;

private:
  static void __stdcall get_buckets(std::vector<Point *>::iterator begin,
    std::vector<Point*>::iterator end, uint8_t depth, std::size_t count,
    const DoubleRect& global_bounds, Bucket_t& out_buckets);

  static std::size_t __stdcall points_to_vector(const Point* point_begin,
    const Point* point_end, std::vector<Point*>& out_vec);

private:
  node* root_;
  DoubleRect global_bounds_;
};

#endif



