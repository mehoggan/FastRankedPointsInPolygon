#include "quad_tree.h"

#include "io.h"
#include "point_search.h"

#include <algorithm>
#include <atomic>
#include <cassert>
#include <cmath>
#include <deque>
#include <fstream>
#include <functional>
#include <iostream>
#include <mutex>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

bool __stdcall intersect(
  const DoubleRect& a,
  const DoubleRect& b)
{
  bool ret = true;
  ret &= a.lx <= b.hx;
  ret &= a.hx >= b.lx;
  ret &= a.ly <= b.hy;
  ret &= a.hy >= b.ly;
  return ret;
}

bool __stdcall intersect_point(
  const Point& a,
  const DoubleRect& b)
{
  bool ret = true;
  ret &= a.x >= b.lx;
  ret &= a.x <= b.hx;
  ret &= a.y >= b.ly;
  ret &= a.y <= b.hy;
  return ret;
}

constexpr uint32_t x_integer_space_ = 0xFFFFFFFF;
constexpr uint32_t y_integer_space_ = 0xFFFFFFFF;

uint64_t __stdcall only_msb64_on(register uint64_t x)
{
  x |= (x >> 1);
  x |= (x >> 2);
  x |= (x >> 4);
  x |= (x >> 8);
  x |= (x >> 16);
  x |= (x >> 32);
  return x & ~(x >> 1ull);
}

int8_t __stdcall quad_tree::msb64(register uint64_t x)
{
  uint64_t depth = max_depth();

  uint64_t max_bit = (1ull << (2ull * max_depth()));
  uint64_t max_val = max_bit | (max_bit - 1);

  x = only_msb64_on(x);
  if (x & 0x0000000000000001ull) {
    depth = 0ull;
  } else if (x & 0x1000000000000000ull) {
    depth = max_depth();
  } else if (x > max_val) {
    throw std::runtime_error("Invalid key provided to msb64.");
  } else {
    uint64_t mask = 0x0000000080000000ull;
    uint64_t shift = 32ull;
    depth = 31ull;
    while (shift != 0) {
      shift /= 2;
      if (x > mask) {
        mask <<= shift;
        depth += shift;
      } else if (x < mask) {
        mask >>= shift;
        depth -= shift;
      } else {
        break;
      }
    }
  }
  return static_cast<int8_t>(depth);
}

uint64_t __stdcall quad_tree::spread_by_1_bit(int64_t x)
{
  x &= 0x00000000ffffffffull;
  x = (x | (x << 16)) & 0x0000ffff0000ffffull;
  x = (x | (x << 8)) & 0x00ff00ff00ff00ffull;
  x = (x | (x << 4)) & 0x0f0f0f0f0f0f0f0full;
  x = (x | (x << 2)) & 0x3333333333333333ull;
  x = (x | (x << 1)) & 0x5555555555555555ull;

  return x;
}

int64_t __stdcall quad_tree::compact_by_1_bit(int64_t x)
{
  x &= 0x5555555555555555ull;
  x = (x | (x >> 1)) & 0x3333333333333333ull;
  x = (x | (x >> 2)) & 0x0f0f0f0f0f0f0f0full;
  x = (x | (x >> 4)) & 0x00ff00ff00ff00ffull;
  x = (x | (x >> 8)) & 0x0000ffff0000ffffull;
  x = (x | (x >> 16)) & 0x00000000ffffffffull;

  return x;
}

uint8_t __stdcall quad_tree::max_depth()
{
  return 29u;
}

uint64_t __stdcall quad_tree::compute_quad_key(
  const Point& p,
  uint8_t depth,
  const DoubleRect& bounds)
{
  if (depth > max_depth()) {
    throw std::runtime_error(
      std::to_string(depth) + " must be less than or equal to " +
      std::to_string(max_depth()));
  }

  double domain = static_cast<double>(bounds.hx) -
    static_cast<double>(bounds.lx);
  double range = static_cast<double>(bounds.hy) -
    static_cast<double>(bounds.ly);

  double percent_x = (static_cast<double>(p.x)
    - static_cast<double>(bounds.lx)) / domain;
  double percent_y = (static_cast<double>(p.y) -
    static_cast<double>(bounds.ly)) / range;

  constexpr uint32_t max_32_bit_uint = (std::numeric_limits<uint32_t>::max)();

  uint32_t percent_x_i = (std::min)(
    static_cast<uint64_t>(percent_x * x_integer_space_),
    static_cast<uint64_t>(max_32_bit_uint));
  uint32_t percent_y_i = (std::min)(
    static_cast<uint64_t>(percent_y * y_integer_space_),
    static_cast<uint64_t>(max_32_bit_uint));

  uint64_t xbits = spread_by_1_bit(percent_x_i);
  uint64_t ybits = spread_by_1_bit(percent_y_i);
  uint64_t ybits_shifted = (ybits << 1);

  uint64_t morton = xbits | ybits_shifted;
  bool chop_bit_to_prevent_barrel_roll = morton & 0x8000000000000000;
  if (chop_bit_to_prevent_barrel_roll) {
    morton &= (~0x8000000000000000); // To prevent barrel rolling
  }

  uint64_t shift = (64ull - static_cast<uint64_t>(depth * 2ull));
  if (shift == 64) {
    shift = 63; // On x64 the next line will fail to shift mith msvc.
  }
  uint64_t morton_shifted_by_depth = (morton >> shift);
  uint64_t depth_bit = (0x1ull << (2 * depth));

  uint64_t morton_shifted_with_depth_bit = morton_shifted_by_depth |
    depth_bit;

  if (chop_bit_to_prevent_barrel_roll && depth != 0) {
    uint64_t y_bit_back_in = (0x1ull << (depth * 2ull - 1ull));
    morton_shifted_with_depth_bit |= y_bit_back_in;
  }
  return morton_shifted_with_depth_bit;
}

uint64_t __stdcall quad_tree::min_id(uint8_t depth)
{
  uint64_t depth_bit = (0x1ull << (2 * depth));
  uint64_t ret = 0ull | depth_bit;
  return ret;
}

uint64_t __stdcall quad_tree::max_id(uint8_t depth)
{
  uint64_t depth_bit = (0x1ull << (2 * depth));
  uint64_t ret = 0ull | depth_bit;
  ret |= (0x7FFFFFFFFFFFFFFF >> (63 - 2 * depth));
  return ret;
}

bool __stdcall quad_tree::is_valid(uint64_t quad_key)
{
  uint64_t max_bit = (1ull << (2ull * max_depth()));
  uint64_t max_val = max_bit | (max_bit - 1);

  bool ret = true;
  if (quad_key == 0 || quad_key > max_val) {
    ret = false;
  }
  return ret;
}

void __stdcall quad_tree::compute_children(
  uint64_t parent,
  Children_t& children)
{
  if (!is_valid(parent)) {
    throw std::runtime_error("Invalid child of " + std::to_string(parent));
  }
  else if (parent >= max_id(max_depth() - 1)) {
    throw std::runtime_error("You have reached the maximum depth.");
  }
  for (std::size_t i : {0, 1, 2, 3}) {
    children[i] = (parent << 2) + i;
  }
}

uint64_t __stdcall quad_tree::compute_parent(uint64_t child)
{
  if (!is_valid(child)) {
    throw std::runtime_error("Invalid child of " + std::to_string(child));
  }
  else if (child == min_id(0)) {
    throw std::runtime_error("Root key does not have a parent.");
  }
  child = child & 0x7FFFFFFFFFFFFFFF;
  uint64_t parent = child >> 2;
  return parent;
}

void __stdcall quad_tree::compute_bounds_for_quad_key(
  uint64_t quad_key,
  const DoubleRect& global_bounds,
  DoubleRect& out_bounds)
{
  const double domain = static_cast<double>(global_bounds.hx) -
    static_cast<double>(global_bounds.lx);
  const double range = static_cast<double>(global_bounds.hy) -
    static_cast<double>(global_bounds.ly);

  if (std::isinf(domain) || std::isinf(range)) {
    std::string error_message = std::string(__FUNCTION__)
      + std::string(" cannot handle such a large space consider")
      + std::string(" reducing search space such that")
      + std::string(" lx and ly are greater than or equal to ")
      + std::to_string(-(std::numeric_limits<float>::max)())
      + std::string(" and hx and hy are less than or equal to ")
      + std::to_string(+(std::numeric_limits<float>::max)());
    throw std::runtime_error(error_message);
  }

  int8_t depth = msb64(quad_key) / 2;
  uint64_t step_size = step_size_at_depth(depth);
  uint64_t remove_only_msb = ~only_msb64_on(quad_key);
  uint64_t quad_key_with_no_depth_bit = quad_key & remove_only_msb;
  uint64_t x_bits = quad_key_with_no_depth_bit & 0x5555555555555555ull;
  uint32_t x_bits_compressed = compact_by_1_bit(x_bits);
  uint64_t y_bits = quad_key_with_no_depth_bit & 0xAAAAAAAAAAAAAAAAull;
  uint64_t y_bits_shifted = y_bits >> 1;
  uint32_t y_bits_compressed = compact_by_1_bit(y_bits_shifted);

  const uint32_t shift = 32u - depth;
  uint64_t x_bits_pos_ll = static_cast<uint64_t>(x_bits_compressed) << shift;
  uint64_t y_bits_pos_ll = static_cast<uint64_t>(y_bits_compressed) << shift;
  uint64_t x_bits_pos_ur = x_bits_pos_ll + step_size;
  uint64_t y_bits_pos_ur = y_bits_pos_ll + step_size;

  double x_integer_space_as_double = static_cast<double>(
    static_cast<double>(x_integer_space_));
  double y_integer_space_as_double = static_cast<double>(
    static_cast<double>(y_integer_space_));
 
  double ll_x_bits_as_double = static_cast<double>(x_bits_pos_ll);
  double ll_percent_x = ll_x_bits_as_double / x_integer_space_as_double;

  double ll_y_bits_as_double = static_cast<double>(y_bits_pos_ll);
  double ll_percent_y = ll_y_bits_as_double / y_integer_space_as_double;

  double ur_x_bits_as_double = static_cast<double>(x_bits_pos_ur);
  double ur_percent_x = ur_x_bits_as_double / x_integer_space_as_double;

  double ur_y_bits_as_double = static_cast<double>(y_bits_pos_ur);
  double ur_percent_y = ur_y_bits_as_double / y_integer_space_as_double;

  double ll_x = ll_percent_x * domain;
  ll_x = global_bounds.lx + ll_x;
  if (std::isinf(ll_x)) {
    ll_x = -(std::numeric_limits<double>::max)();
  }
  double ll_y = ll_percent_y * range;
  ll_y = global_bounds.ly + ll_y;
  if (std::isinf(ll_y)) {
    ll_y = -(std::numeric_limits<double>::max)();
  }

  double ur_x = ur_percent_x * domain;
  ur_x = global_bounds.lx + ur_x;
  if (std::isinf(ur_x)) {
    ur_x = +(std::numeric_limits<double>::max)();
  }
  double ur_y = ur_percent_y * range;
  ur_y = global_bounds.ly + ur_y;
  if (std::isinf(ur_y)) {
    ur_y = +(std::numeric_limits<double>::max)();
  }

  out_bounds = DoubleRect{ ll_x, ll_y, ur_x, ur_y };
}

uint32_t __stdcall quad_tree::step_size_at_depth(uint8_t depth)
{
  return x_integer_space_ >> depth;
}

quad_tree::node::node(uint64_t quad_key, const DoubleRect& point_bounds) :
  quad_key_(quad_key),
  point_bounds_(point_bounds)
{
  children_[0] = nullptr;
  children_[1] = nullptr;
  children_[2] = nullptr;
  children_[3] = nullptr;
}

quad_tree::node::~node()
{
  points_.clear();
}

void __stdcall quad_tree::node::set_data(
  std::vector<Point*>::iterator begin,
  std::vector<Point*>::iterator end)
{
  auto size = std::distance(begin, end);
  points_.resize(size);
  std::vector<Point*>::iterator it = begin;
  std::size_t index = 0;
  while (it < end) {
    points_[index] = **it;
    ++it;
    ++index;
  }
}

void __stdcall quad_tree::node::set_child(const ChildId id, node* child)
{
  children_[static_cast<std::uint8_t>(id)] = child;
}

__stdcall quad_tree::quad_tree(
  const Point* point_begin,
  const Point* point_end,
  const std::size_t min_block_size,
  const std::size_t max_block_size) :
  root_(nullptr),
  global_bounds_({})
{
  if (point_begin == nullptr || point_end == nullptr) {
    return;
  }
  std::ptrdiff_t size = std::distance(point_begin, point_end);
  Point* citer = const_cast<Point*>(point_begin);
  std::vector<Point*> points;
  points_to_vector(point_begin, point_end, points);
  create(points.begin(), points.end(), min_block_size, max_block_size);
}

__stdcall quad_tree::quad_tree(
  std::vector<Point*>::iterator begin,
  std::vector<Point*>::iterator end,
  const std::size_t min_block_size,
  const std::size_t max_block_size) :
  root_(nullptr),
  global_bounds_({})
{
  if (begin == end) {
    return;
  }
  create(begin, end, min_block_size, max_block_size);
}

__stdcall quad_tree::~quad_tree()
{
  destroy_tree(root_);
}

const DoubleRect& __stdcall quad_tree::global_bounds() const
{
  return global_bounds_;
}

void __stdcall quad_tree::query(
  const Rect& query_rect,
  std::function<void(const Point & point)> contained_callback)
{
  DoubleRect internal_rect = {
    query_rect.lx,
    query_rect.ly,
    query_rect.hx,
    query_rect.hy
  };
  traverse_tree_by_bounds(root_, internal_rect, contained_callback);
}

void __stdcall quad_tree::compute_bounds(
    std::vector<Point*>::iterator begin,
    std::vector<Point*>::iterator end,
    DoubleRect& out_rect)
{
  float max_y = -(std::numeric_limits<float>::max)();
  float min_y = +(std::numeric_limits<float>::max)();
  float max_x = -(std::numeric_limits<float>::max)();
  float min_x = +(std::numeric_limits<float>::max)();

  std::for_each(begin, end,
    [&](const Point* it)
    {
      if (it->x < min_x) min_x = it->x;
      if (it->x > max_x) max_x = it->x;
      if (it->y < min_y) min_y = it->y;
      if (it->y > max_y) max_y = it->y;
      int x = 0;
    });

  out_rect = { std::floor(min_x), std::floor(min_y),
    std::ceil(max_x), std::ceil(max_y) };
}

void __stdcall quad_tree::create(
  std::vector<Point*>::iterator begin,
  std::vector<Point*>::iterator end,
  const std::size_t min_block_size,
  const std::size_t max_block_size)
{
  compute_bounds(begin, end, global_bounds_);
  root_ = new node(compute_quad_key(**begin, 0u, global_bounds_),
    global_bounds_);
  build_tree(root_, begin, end, 0u, min_block_size, max_block_size);
}

void __stdcall quad_tree::build_tree(node* node, 
  std::vector<Point*>::iterator begin,
  std::vector<Point*>::iterator end,
  uint8_t depth,
  const std::size_t min_block_size,
  const std::size_t max_block_size)
{
  std::size_t count = std::distance(begin, end);

  if (node == nullptr || count == 0) {
    return;
  }

  typedef std::tuple<uint64_t, std::vector<Point*>, std::size_t> BucketItem_t;
  std::function<quad_tree::node * (BucketItem_t&)>
    build_node =
    [&](BucketItem_t& bucket)
    {
      auto& id = std::get<0>(bucket);
      std::uint8_t depth = msb64(id) / 2;

      DoubleRect point_bounds;
      if (depth <= 0) {
        compute_bounds_for_quad_key(id, global_bounds_, point_bounds);
      } else {
        compute_bounds(
          std::get<1>(bucket).begin(),
          std::get<1>(bucket).begin() + std::get<2>(bucket),
          point_bounds);
      }

      quad_tree::node* ret = new quad_tree::node(id, point_bounds);
      return ret;
    };

  if (count > max_block_size && depth != max_depth()) {
    Bucket_t buckets;
    get_buckets(begin, end, depth, count, global_bounds_, buckets);

    if (std::get<2>(buckets[0]) != 0) {
      quad_tree::node* child0 = build_node(buckets[0]);
      node->children_[0] = child0;
      build_tree(
        child0,
        std::get<1>(buckets[0]).begin(),
        std::get<1>(buckets[0]).begin() + std::get<2>(buckets[0]),
        depth + 1,
        min_block_size,
        max_block_size);
    }

    if (std::get<2>(buckets[1]) != 0) {
      quad_tree::node* child1 = build_node(buckets[1]);
      node->children_[1] = child1;
      build_tree(
        child1,
        std::get<1>(buckets[1]).begin(),
        std::get<1>(buckets[1]).begin() + std::get<2>(buckets[1]),
        depth + 1,
        min_block_size,
        max_block_size);
    }

    if (std::get<2>(buckets[2]) != 0) {
      quad_tree::node* child2 = build_node(buckets[2]);
      node->children_[2] = child2;
      build_tree(
        child2,
        std::get<1>(buckets[2]).begin(),
        std::get<1>(buckets[2]).begin() + std::get<2>(buckets[2]),
        depth + 1,
        min_block_size,
        max_block_size);
    }

    if (std::get<2>(buckets[3]) != 0) {
      quad_tree::node* child3 = build_node(buckets[3]);
      node->children_[3] = child3;
      build_tree(
        child3,
        std::get<1>(buckets[3]).begin(),
        std::get<1>(buckets[3]).begin() + std::get<2>(buckets[3]),
        depth + 1,
        min_block_size,
        max_block_size);
    }
  } else {
    node->set_data(begin, end);
  }

}

void __stdcall quad_tree::traverse_tree_by_bounds(
  node* curr,
  const DoubleRect& bounds,
  std::function<void(const Point & point)> contained_callback)
{
  if (curr == nullptr) {
    return;
  }
  else {
    if (intersect(bounds, curr->point_bounds_)) {
      if (!curr->points_.empty()) {
        std::for_each(curr->points_.begin(), curr->points_.end(),
          [&](const Point& p)
          {
            if (intersect_point(p, bounds)) {
              contained_callback(p);
            }
          });
      } else {
        traverse_tree_by_bounds(
          curr->children_[0], bounds, contained_callback);
        traverse_tree_by_bounds(
          curr->children_[1], bounds, contained_callback);
        traverse_tree_by_bounds(
          curr->children_[2], bounds, contained_callback);
        traverse_tree_by_bounds(
          curr->children_[3], bounds, contained_callback);
      }
    }
  }
}

std::size_t __stdcall quad_tree::size() const
{
  std::size_t ret = 0;
  quad_tree::size_recursive(root_, ret);
  return ret;
}

void __stdcall quad_tree::destroy_tree(node* curr)
{
  if (curr == nullptr) {
    return;
  } else {
    destroy_tree(curr->children_[0]);
    destroy_tree(curr->children_[1]);
    destroy_tree(curr->children_[2]);
    destroy_tree(curr->children_[3]);
    delete curr;
    curr = nullptr;
  }
}

void __stdcall quad_tree::size_recursive(node* curr, std::size_t& count) const
{
  if (curr == nullptr) {
    return;
  } else if (!curr->points_.empty()) {
    count += curr->points_.size();
  } else {
    size_recursive(curr->children_[0], count);
    size_recursive(curr->children_[1], count);
    size_recursive(curr->children_[2], count);
    size_recursive(curr->children_[3], count);
  }
}

void __stdcall quad_tree::print_tree(node* curr)
{
  std::function<void (node*)> print_leaf = [this](node* to_print)
    {
      DoubleRect quad_key_bounds;
      quad_tree::compute_bounds_for_quad_key(
        to_print->quad_key_,
        global_bounds_,
        quad_key_bounds);
      std::cout << "key = " << to_print->quad_key_ << " bounds = "
        << quad_key_bounds << " points size " << std::dec
        << to_print->points_.size() << std::endl;
    };

  if (curr == nullptr) {
    return;
  } else {
    print_leaf(curr);
    print_tree(curr->children_[0]);
    print_tree(curr->children_[1]);
    print_tree(curr->children_[2]);
    print_tree(curr->children_[3]);
  }
}

void __stdcall quad_tree::get_buckets(std::vector<Point*>::iterator begin,
  std::vector<Point*>::iterator end, uint8_t depth, std::size_t count,
  const DoubleRect& global_bounds, Bucket_t& out_buckets)
{
  const Point& ip = **begin;
  uint64_t p_pid = compute_quad_key(ip, depth, global_bounds);
  Children_t children;
  compute_children(p_pid, children);
  out_buckets[0] = std::make_tuple(children[0], std::vector<Point*>(), 0ull);
  out_buckets[1] = std::make_tuple(children[1], std::vector<Point*>(), 0ull);
  out_buckets[2] = std::make_tuple(children[2], std::vector<Point*>(), 0ull);
  out_buckets[3] = std::make_tuple(children[3], std::vector<Point*>(), 0ull);
  for (std::size_t i = 0; i < 4; ++i) {
    auto& v = std::get<1>(out_buckets[i]);
    v.resize(count * .25, nullptr);
  }

  const uint64_t min_id = std::get<0>(out_buckets[0]);
  std::vector<Point*>::iterator it = begin;
  while (it != end) {
    Point& p = **it;
    uint64_t c_pid = compute_quad_key(p, depth + 1, global_bounds);
    std::size_t bucket_index = c_pid - min_id;
    auto& vec = std::get<1>(out_buckets[bucket_index]);
    std::size_t& index = std::get<2>(out_buckets[bucket_index]);
    if (index >= vec.size()) {
      vec.resize(vec.size() + (0.25 * count), nullptr);
    }
    vec[index] = &p;
    ++index;
    ++it;
  }
}

std::size_t __stdcall quad_tree::points_to_vector(const Point* point_begin,
  const Point* point_end, std::vector<Point*>& out_vec)
{
  std::ptrdiff_t size = std::distance(point_begin, point_end);
  Point* citer = const_cast<Point*>(point_begin);
  out_vec = std::vector<Point*>(size);
  std::size_t i = 0;
  while (citer != point_end) {
    out_vec[i] = citer;
    ++i;
    ++citer;
  }
  return size;
}
