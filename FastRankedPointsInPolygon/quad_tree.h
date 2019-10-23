#ifndef QUAD_TREE_H
#define QUAD_TREE_H

#include <cstddef>
#include <cstdint>
#include <functional>
#include <list>
#include <tuple>
#include <vector>

#include "ipoint_search.h"

class __declspec(dllexport) quad_tree
{
public:
  static int8_t __stdcall msb64(register uint64_t x);

  static uint64_t __stdcall spread_by_1_bit(int64_t x);

  static int64_t __stdcall compact_by_1_bit(int64_t x);

  static uint8_t __stdcall max_depth();

  static uint32_t __stdcall max_rows(uint8_t depth);

  static uint32_t __stdcall max_cols(uint8_t depth);

  static uint64_t __stdcall compute_quad_key(
    const Point& p,
    uint8_t depth,
    const Rect &bounds);

  static uint64_t __stdcall min_id(uint8_t depth);

  static uint64_t __stdcall max_id(uint8_t depth);

  static bool __stdcall is_valid(uint64_t quad_key);

  typedef uint64_t Children_t[4];
  static void __stdcall compute_children(uint64_t parent,
    Children_t& children);

  static uint64_t __stdcall compute_parent(uint64_t child);

  static void __stdcall compute_bounds_for_quad_key(
    uint64_t quad_key,
    const Rect& global_bounds,
    Rect& out_bounds);

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

    node(uint64_t quad_key, quad_tree& parent);

    ~node();

    void __stdcall set_data(
      std::vector<Point *>::iterator begin,
      std::vector<Point *>::iterator end);

    void __stdcall set_child(const ChildId id, node* child);

    uint64_t quad_key_;
    std::vector<Point> points_;
    node* children_[4];
    quad_tree& parent_;
  };

  typedef std::tuple<uint64_t, std::vector<Point*>, uint64_t> Bucket_t[4];

public:
  constexpr static std::size_t MAX_BLOCK_SIZE = 1000ull;
  constexpr static std::size_t MIN_BLOCK_SIZE = 10ull;

  __stdcall quad_tree(
    const Point* point_begin,
    const Point* point_end,
    const std::size_t min_block_size = MIN_BLOCK_SIZE,
    const std::size_t max_block_size = MAX_BLOCK_SIZE);

  __stdcall quad_tree(
    std::vector<Point *>::iterator begin,
    std::vector<Point *>::iterator end,
    const std::size_t min_block_size = MIN_BLOCK_SIZE,
    const std::size_t max_block_size = MAX_BLOCK_SIZE);

  __stdcall ~quad_tree();

  const Rect& __stdcall global_bounds() const;

  void __stdcall query(const Rect& query_rect,
    std::function<void(const Point & point)> contained_callback);

  void __stdcall update_point_count(std::size_t count);

  std::size_t __stdcall point_count() const;

public:
  static void __stdcall compute_bounds(
    std::vector<Point *>::iterator begin,
    std::vector<Point *>::iterator end,
    Rect& out_rect);

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
    const Rect& bounds,
    std::function<void(const Point & point)> contained_callback);

  void __stdcall print_tree(node* curr);

private:
  static void __stdcall get_buckets(std::vector<Point *>::iterator begin,
    std::vector<Point*>::iterator end, uint8_t depth, std::size_t count,
    const Rect& global_bounds, Bucket_t& out_buckets);

  static std::size_t __stdcall points_to_vector(const Point* point_begin,
    const Point* point_end, std::vector<Point*>& out_vec);

private:
  node* root_;
  Rect global_bounds_;
  std::size_t point_count_;
};

#endif



