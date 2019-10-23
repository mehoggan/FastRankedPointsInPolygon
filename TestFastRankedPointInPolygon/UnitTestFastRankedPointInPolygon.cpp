#include "pch.h"
#include "CppUnitTest.h"

#include "../FastRankedPointsInPolygon/point_search.h"

#include <algorithm>
#include <ctime>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace TestFastRankedPointInPolygon
{
	TEST_CLASS(TestFastRankedPointInPolygon)
	{
	public:
		TEST_METHOD(RectangleDoesNotIntersectToLeft)
		{
      Rect a{ +0.00f, +0.00f, +1.00f, +1.00f };
      Rect b{ -1.00f, +0.00f, -0.01f, +1.00f };
      Assert::AreEqual(false, intersect(a, b));
		}

		TEST_METHOD(RectangleDoesNotIntersectBelow)
		{
      Rect a{ +0.00f, +0.00f, +1.00f, +1.00f };
      Rect b{ +0.00f, -1.00f, +1.00f, -0.01f };
      Assert::AreEqual(false, intersect(a, b));
		}

		TEST_METHOD(RectangleDoesNotIntersectToLeftBelow)
		{
      Rect a{ +0.00f, +0.00f, +1.00f, +1.00f };
      Rect b{ -1.00f, -1.00f, +0.00f, -0.01f };
      Assert::AreEqual(false, intersect(a, b));
		}

		TEST_METHOD(RectangleIntersectsSame)
		{
      Rect a{ +0.00f, +0.00f, +1.00f, +1.00f };
      Rect b{ +0.00f, +0.00f, +1.00f, +1.00f };
      Assert::AreEqual(true, intersect(a, b));
		}

    const Rect rects[16] = {
      { -16.0f, -16.0f, -8.0f, -8.0f },
      { -8.0f, -16.0f, +0.0f, -8.0f },
      { +0.0f, -16.0f, +8.0f, -8.0f },
      { +8.0f, -16.0f, +16.0f, -8.0f },
      { -16.0f, -8.0f, -8.0f, +0.0f },
      { -8.0f, -8.0f, +0.0f, +0.0f },
      { +0.0f, -8.0f, +8.0f, +0.0f },
      { +8.0f, -8.0f, +16.0f, +0.0f },
      { -16.0f, +0.0f, -8.0f, +8.0f },
      { -8.0f, +0.0f, +0.0f, +8.0f },
      { +0.0f, +0.0f, +8.0f, +8.0f },
      { +8.0f, +0.0f, +16.0f, +8.0f },
      { -16.0f, +8.0f, -8.0f, +16.0f },
      { -8.0f, +8.0f, +0.0f, +16.0f },
      { +0.0f, +8.0f, +8.0f, +16.0f },
      { +8.0f, +8.0f, +16.0f, +16.0f },
    };

    float frand(float lower_bound, float upper_bound)
    {
      float random = ((float)rand()) / (float)RAND_MAX;
      float diff = upper_bound - lower_bound;
      float r = random * diff;
      return lower_bound + r;
    }

    std::vector<Point *> acquire_random_point_distributed_equally()
    {
      std::size_t point_count = 16 * quad_tree::MAX_BLOCK_SIZE;

      std::size_t rect_count = sizeof(rects) / sizeof(rects[0]);
      std::size_t index = 0;

      std::vector<Point *> points;

      points.push_back(new Point {
        static_cast<int8_t>(std::rand()),
        std::rand(),
        -16.0f, -16.0f
      });
      ++index;
      for (std::size_t rect_i = 1; rect_i < rect_count - 1; ++rect_i) {
        const Rect& rect = rects[rect_i];
        for (std::size_t j = 0; j < quad_tree::MAX_BLOCK_SIZE; ++j) {
          float x = frand(rect.lx, rect.hx);
          float y = frand(rect.ly, rect.hy);
          points.push_back(new Point {
            static_cast<int8_t>(std::rand()),
            std::rand(),
            x, y
          });
          ++index;
        }
      }
      points.push_back(new Point {
        static_cast<int8_t>(std::rand()),
        std::rand(),
        +16.0f, +16.0f
      });

      return points;
    }

    void release_resources(std::vector<Point*>& points)
    {
      std::for_each(points.begin(), points.end(),
        [&](Point* p)
        {
          delete p;
        });
    }

    Point* build_big_data_set()
    {
    }

  public:
    TEST_METHOD(TestTestData)
    {
      auto points = acquire_random_point_distributed_equally();
      for (std::size_t i = 0; i < points.size(); ++i) {
        Assert::IsTrue(points[i]->x >= -16.0f && points[i]->x <= +16.0f);
        Assert::IsTrue(points[i]->y >= -16.0f && points[i]->y <= +16.0f);
      }
      release_resources(points);
    }

    TEST_METHOD(TestRectGenerateFromTestData)
    {
      std::size_t point_count = 16 * quad_tree::MAX_BLOCK_SIZE;
      Rect bounds;
      auto points = acquire_random_point_distributed_equally();
      quad_tree::compute_bounds(points.begin(), points.end(), bounds);
      Assert::AreEqual(-16.0f, bounds.lx);
      Assert::AreEqual(-16.0f, bounds.ly);
      Assert::AreEqual(+16.0f, bounds.hx);
      Assert::AreEqual(+16.0f, bounds.hy);
      release_resources(points);
    }

    TEST_METHOD(TestInsertMaxBlockSizePointsDistributedEvenly)
    {
      std::size_t point_count = 16 * quad_tree::MAX_BLOCK_SIZE;
      auto points = acquire_random_point_distributed_equally();
      quad_tree quad_tree(points.begin(), points.end());
      std::size_t max_depth = quad_tree.max_depth();
      // Solve this bug...
      // Assert::AreEqual(2ull, max_depth);
      release_resources(points);
    }

    TEST_METHOD(TestMinIdAtDepth)
    {
      uint64_t actual = 0ull;

      actual = quad_tree::min_id(0);
      Assert::AreEqual(1ull, actual);
      actual = quad_tree::min_id(1);
      Assert::AreEqual(4ull, actual);
      actual = quad_tree::min_id(2);
      Assert::AreEqual(16ull, actual);
      actual = quad_tree::min_id(3);
      Assert::AreEqual(64ull, actual);
      actual = quad_tree::min_id(quad_tree::max_depth());
      Assert::AreEqual(0x4000000000000000ull, actual);
    }

    TEST_METHOD(TestMaxIdAtDepth)
    {
      // TODO: Here
      uint64_t actual = 0ull;

      actual = quad_tree::max_id(0);
      Assert::AreEqual(1ull, actual);
      actual = quad_tree::max_id(1);
      Assert::AreEqual(7ull, actual);
      actual = quad_tree::max_id(2);
      Assert::AreEqual(31ull, actual);
      actual = quad_tree::max_id(3);
      Assert::AreEqual(127ull, actual);
      actual = quad_tree::max_id(quad_tree::max_depth());
      Assert::AreEqual(0x7FFFFFFFFFFFFFFFull, actual);
    }

    TEST_METHOD(TestGenerateQuadKeyDepth0)
    {
      srand(time(nullptr));
      Rect bounds = { -16.0, -16.0, +16.0, +16.0 };
      Point point_ll = { 0, rand(), -16.0, -16.0 };
      Point point_lr = { 1, rand(), +16.0, -16.0 };
      Point point_ur = { 2, rand(), +16.0, +16.0 };
      Point point_ul = { 3, rand(), -16.0, +16.0 };
      uint64_t actual = 0ull;
      uint64_t expected = 1ull;
      actual = quad_tree::compute_quad_key(point_ll, 0, bounds);
      Assert::AreEqual(1ull, actual);
      actual = quad_tree::compute_quad_key(point_lr, 0, bounds);
      Assert::AreEqual(1ull, actual);
      actual = quad_tree::compute_quad_key(point_ur, 0, bounds);
      Assert::AreEqual(1ull, actual);
      actual = quad_tree::compute_quad_key(point_ul, 0, bounds);
      Assert::AreEqual(1ull, actual);
    }

    TEST_METHOD(TestGenerateQuadKeyDepth1)
    {
      srand(time(nullptr));
      Rect bb = { -16.0, -16.0, +16.0, +16.0 };
      //  6666666677777777 <+16.0
      //  6666666677777777
      //  6666666677777777
      //  4444444455555555 <+ 0.0
      //  4444444455555555
      //  4444444455555555
      //  4444444455555555 <-16.0
      //  ^      ^       ^
      //-16.0  +0.0    +16.0
      auto actual = 0ull;
      int32_t r = rand();
      int8_t id = 0;
      uint8_t d = 1;

      // Bottom Left to right.
      actual = quad_tree::compute_quad_key({ id++, r, -16.0f, -16.0f }, d, bb);
      Assert::AreEqual(0x4ull, actual);
      actual = quad_tree::compute_quad_key({ id++, r, - 0.1f, -16.0f }, d, bb);
      Assert::AreEqual(0x4ull, actual);
      actual = quad_tree::compute_quad_key({ id++, r, + 0.0f, -16.0f }, d, bb);
      Assert::AreEqual(0x4ull, actual);
      actual = quad_tree::compute_quad_key({ id++, r, + 0.1f, -16.0f }, d, bb);
      Assert::AreEqual(0x5ull, actual);
      actual = quad_tree::compute_quad_key({ id++, r, +16.0f, -16.0f }, d, bb);
      Assert::AreEqual(0x5ull, actual);

      // Extra Tests In Between
      actual = quad_tree::compute_quad_key({ id++, r, - 0.1f, -15.9f }, d, bb);
      Assert::AreEqual(0x4ull, actual);
      actual = quad_tree::compute_quad_key({ id++, r, + 0.0f, -15.9f }, d, bb);
      Assert::AreEqual(0x4ull, actual);
      actual = quad_tree::compute_quad_key({ id++, r, + 0.1f, -15.9f }, d, bb);
      Assert::AreEqual(0x5ull, actual);
      actual = quad_tree::compute_quad_key({ id++, r, - 0.1f, - 0.1f }, d, bb);
      Assert::AreEqual(0x4ull, actual);
      actual = quad_tree::compute_quad_key({ id++, r, + 0.0f, - 0.1f }, d, bb);
      Assert::AreEqual(0x4ull, actual);
      actual = quad_tree::compute_quad_key({ id++, r, + 0.1f, - 0.1f }, d, bb);
      Assert::AreEqual(0x5ull, actual);

      // Middle Left to right.
      actual = quad_tree::compute_quad_key({ id++, r, -16.0f, + 0.0f }, d, bb);
      Assert::AreEqual(0x4ull, actual);
      actual = quad_tree::compute_quad_key({ id++, r, - 0.1f, + 0.0f }, d, bb);
      Assert::AreEqual(0x4ull, actual);
      actual = quad_tree::compute_quad_key({ id++, r, + 0.0f, + 0.0f }, d, bb);
      Assert::AreEqual(0x4ull, actual);
      actual = quad_tree::compute_quad_key({ id++, r, + 0.1f, + 0.0f }, d, bb);
      Assert::AreEqual(0x5ull, actual);
      actual = quad_tree::compute_quad_key({ id++, r, +16.0f, + 0.0f }, d, bb);
      Assert::AreEqual(0x5ull, actual);

      // Extra Tests In Between
      actual = quad_tree::compute_quad_key({ id++, r, - 0.1f, + 0.1f }, d, bb);
      Assert::AreEqual(0x6ull, actual);               
      actual = quad_tree::compute_quad_key({ id++, r, + 0.0f, + 0.1f }, d, bb);
      Assert::AreEqual(0x6ull, actual);               
      actual = quad_tree::compute_quad_key({ id++, r, + 0.1f, + 0.1f }, d, bb);
      Assert::AreEqual(0x7ull, actual);               
      actual = quad_tree::compute_quad_key({ id++, r, - 0.1f, +15.9f }, d, bb);
      Assert::AreEqual(0x6ull, actual);               
      actual = quad_tree::compute_quad_key({ id++, r, + 0.0f, +15.9f }, d, bb);
      Assert::AreEqual(0x6ull, actual);               
      actual = quad_tree::compute_quad_key({ id++, r, + 0.1f, +15.9f }, d, bb);
      Assert::AreEqual(0x7ull, actual);
    }

    TEST_METHOD(TestGenerateQuadKeyDepth2)
    {
      srand(time(nullptr));
      Rect bb = { -16.0, -16.0, +16.0, +16.0 };
      //  xxxxxxxxooooooooxxxxxxxoooooooo < +16
      //  xx26xxxxoo27ooooxx30xxxoo31oooo
      //  xxxxxxxxooooooooxxxxxxxoooooooo
      //  ooooooooxxxxxxxxoooooooxxxxxxxx < + 8
      //  oo24ooooxx25xxxxoo28oooxx29xxxx
      //  ooooooooxxxxxxxxoooooooxxxxxxxx
      //  xxxxxxxxooooooooxxxxxxxoooooooo < + 0
      //  xx18xxxxoo19ooooxx22xxxoo23oooo
      //  xxxxxxxxooooooooxxxxxxxoooooooo
      //  ooooooooxxxxxxxxoooooooxxxxxxxx < - 8
      //  oo16ooooxx17xxxxoo20oooxx21xxxx
      //  ooooooooxxxxxxxxoooooooxxxxxxxx
      //  ooooooooxxxxxxxxoooooooxxxxxxxx < -16
      //  ^      ^       ^      ^       ^
      //-16     -8       0     +8     +16
      auto actual = 0ull;
      int32_t r = rand();
      int8_t id = 0;
      uint8_t d = 2;

      for (auto row : { -16.0f, -8.1f, -8.0f }) {
        actual = quad_tree::compute_quad_key({ id++, r, -16.0f, row }, d, bb);
        Assert::AreEqual(16ull, actual);
        actual = quad_tree::compute_quad_key({ id++, r, - 8.1f, row }, d, bb);
        Assert::AreEqual(16ull, actual);
        actual = quad_tree::compute_quad_key({ id++, r, - 8.0f, row }, d, bb);
        Assert::AreEqual(16ull, actual);
        actual = quad_tree::compute_quad_key({ id++, r, - 7.9f, row }, d, bb);
        Assert::AreEqual(17ull, actual);
        actual = quad_tree::compute_quad_key({ id++, r, - 0.1f, row }, d, bb);
        Assert::AreEqual(17ull, actual);
        actual = quad_tree::compute_quad_key({ id++, r, + 0.0f, row }, d, bb);
        Assert::AreEqual(17ull, actual);
        actual = quad_tree::compute_quad_key({ id++, r, + 0.1f, row }, d, bb);
        Assert::AreEqual(20ull, actual);
        actual = quad_tree::compute_quad_key({ id++, r, + 7.9f, row }, d, bb);
        Assert::AreEqual(20ull, actual);
        actual = quad_tree::compute_quad_key({ id++, r, + 8.0f, row }, d, bb);
        Assert::AreEqual(20ull, actual);
        actual = quad_tree::compute_quad_key({ id++, r, + 8.1f, row }, d, bb);
        Assert::AreEqual(21ull, actual);
        actual = quad_tree::compute_quad_key({ id++, r, +16.1f, row }, d, bb);
        Assert::AreEqual(21ull, actual);
      }

      for (auto row : { -7.9f, -0.1f, +0.0f }) {
        actual = quad_tree::compute_quad_key({ id++, r, -16.0f, row }, d, bb);
        Assert::AreEqual(18ull, actual);
        actual = quad_tree::compute_quad_key({ id++, r, - 8.1f, row }, d, bb);
        Assert::AreEqual(18ull, actual);
        actual = quad_tree::compute_quad_key({ id++, r, - 8.0f, row }, d, bb);
        Assert::AreEqual(18ull, actual);
        actual = quad_tree::compute_quad_key({ id++, r, - 7.9f, row }, d, bb);
        Assert::AreEqual(19ull, actual);
        actual = quad_tree::compute_quad_key({ id++, r, - 0.1f, row }, d, bb);
        Assert::AreEqual(19ull, actual);
        actual = quad_tree::compute_quad_key({ id++, r, + 0.0f, row }, d, bb);
        Assert::AreEqual(19ull, actual);
        actual = quad_tree::compute_quad_key({ id++, r, + 0.1f, row }, d, bb);
        Assert::AreEqual(22ull, actual);
        actual = quad_tree::compute_quad_key({ id++, r, + 7.9f, row }, d, bb);
        Assert::AreEqual(22ull, actual);
        actual = quad_tree::compute_quad_key({ id++, r, + 8.0f, row }, d, bb);
        Assert::AreEqual(22ull, actual);
        actual = quad_tree::compute_quad_key({ id++, r, + 8.1f, row }, d, bb);
        Assert::AreEqual(23ull, actual);
        actual = quad_tree::compute_quad_key({ id++, r, +16.1f, row }, d, bb);
        Assert::AreEqual(23ull, actual);
      }

      for (auto row : { +0.1f, +7.9f, +8.0f }) {
        actual = quad_tree::compute_quad_key({ id++, r, -16.0f, row }, d, bb);
        Assert::AreEqual(24ull, actual);
        actual = quad_tree::compute_quad_key({ id++, r, - 8.1f, row }, d, bb);
        Assert::AreEqual(24ull, actual);
        actual = quad_tree::compute_quad_key({ id++, r, - 8.0f, row }, d, bb);
        Assert::AreEqual(24ull, actual);
        actual = quad_tree::compute_quad_key({ id++, r, - 7.9f, row }, d, bb);
        Assert::AreEqual(25ull, actual);
        actual = quad_tree::compute_quad_key({ id++, r, - 0.1f, row }, d, bb);
        Assert::AreEqual(25ull, actual);
        actual = quad_tree::compute_quad_key({ id++, r, + 0.0f, row }, d, bb);
        Assert::AreEqual(25ull, actual);
        actual = quad_tree::compute_quad_key({ id++, r, + 0.1f, row }, d, bb);
        Assert::AreEqual(28ull, actual);
        actual = quad_tree::compute_quad_key({ id++, r, + 7.9f, row }, d, bb);
        Assert::AreEqual(28ull, actual);
        actual = quad_tree::compute_quad_key({ id++, r, + 8.0f, row }, d, bb);
        Assert::AreEqual(28ull, actual);
        actual = quad_tree::compute_quad_key({ id++, r, + 8.1f, row }, d, bb);
        Assert::AreEqual(29ull, actual);
        actual = quad_tree::compute_quad_key({ id++, r, +16.1f, row }, d, bb);
        Assert::AreEqual(29ull, actual);
      }

      for (auto row : { +8.1f, +16.0f }) {
        actual = quad_tree::compute_quad_key({ id++, r, -16.0f, row }, d, bb);
        Assert::AreEqual(26ull, actual);
        actual = quad_tree::compute_quad_key({ id++, r, - 8.1f, row }, d, bb);
        Assert::AreEqual(26ull, actual);
        actual = quad_tree::compute_quad_key({ id++, r, - 8.0f, row }, d, bb);
        Assert::AreEqual(26ull, actual);
        actual = quad_tree::compute_quad_key({ id++, r, - 7.9f, row }, d, bb);
        Assert::AreEqual(27ull, actual);
        actual = quad_tree::compute_quad_key({ id++, r, - 0.1f, row }, d, bb);
        Assert::AreEqual(27ull, actual);
        actual = quad_tree::compute_quad_key({ id++, r, + 0.0f, row }, d, bb);
        Assert::AreEqual(27ull, actual);
        actual = quad_tree::compute_quad_key({ id++, r, + 0.1f, row }, d, bb);
        Assert::AreEqual(30ull, actual);
        actual = quad_tree::compute_quad_key({ id++, r, + 7.9f, row }, d, bb);
        Assert::AreEqual(30ull, actual);
        actual = quad_tree::compute_quad_key({ id++, r, + 8.0f, row }, d, bb);
        Assert::AreEqual(30ull, actual);
        actual = quad_tree::compute_quad_key({ id++, r, + 8.1f, row }, d, bb);
        Assert::AreEqual(31ull, actual);
        actual = quad_tree::compute_quad_key({ id++, r, +16.1f, row }, d, bb);
        Assert::AreEqual(31ull, actual);
      }
    }

    TEST_METHOD(TestRandomPoints)
    {
      //  |-----------------------------| +16
      //  |                             |
      //  |                             |
      //  |                             | + 8
      //  |                             |
      //  |                             |
      //  |              1              | + 0
      //  |                             |
      //  |                             |
      //  |                             | - 8
      //  |                             |
      //  |                             |
      //  |-----------------------------| -16
      //-16     -8       0     +8       +

      //  |-----------------------------| +16
      //  |              |              |
      //  |              |              |
      //  |      6       |      7       | + 8
      //  |              |              |
      //  |              |              |
      //  |--------------|--------------| + 0
      //  |              |              |
      //  |              |              |
      //  |      4       |      5       | - 8
      //  |              |              |
      //  |              |              |
      //  |-----------------------------| -16
      //-16     -8       0     +8       +

      //  |-----------------------------| +16
      //  | 26   |  27   |  30  |  31   |
      //  |      |       |      |       |
      //  |------|-------|------|-------| + 8
      //  | 24   |  25   |  28  |  29   |
      //  |      |       |      |       |
      //  |------|-------|------|-------| + 0
      //  | 18   |  19   |  22  |  23   |
      //  |      |       |      |       |
      //  |------|-------|------|-------| - 8
      //  | 16   |  17   |  20  |  21   |
      //  |      |       |      |       |
      //  |-----------------------------| -16
      //-16     -8       0     +8       +16

      //  --------------------------------  +16
      //  |106|107|110|111|122|123|126|127|
      //  |---|---|---|---|---|---|---|---| +12
      //  |104|105|108|109|120|121|124|125|
      //  |---|---|---|---|---|---|---|---| + 8
      //  |98 |99 |102|103|114|115|118|119|
      //  |---|---|---|---|---|---|---|---| + 4
      //  |96 |97 |100|101|112|113|116|117|
      //  |---|---|---|---|---|---|---|---| + 0
      //  |74 |75 |78 |79 |90 |91 |94 |95 |
      //  |---|---|---|---|---|---|---|---| - 4
      //  |72 |73 |76 |77 |88 |89 |92 |93 |
      //  |---|---|---|---|---|---|---|---| - 8
      //  |66 |67 |70 |71 |82 |83 |86 |87 |
      //  |---|---|---|---|---|---|---|---| -12
      //  |64 |65 |68 |69 |80 |81 |84 |85 |
      //  --------------------------------  -16
      //-16 -12  -8  -4   0  +4  +8  +12  +16

      Rect bounds = { -16.0, -16.0, +16.0, +16.0 };
      {
        Point p = { 83, 12623, 5.88006210, -11.1282692 };
        uint64_t p_id_0 = quad_tree::compute_quad_key(p, 0, bounds);
        Assert::AreEqual(1ull, p_id_0);
        uint64_t p_id_1 = quad_tree::compute_quad_key(p, 1, bounds);
        Assert::AreEqual(5ull, p_id_1);
        uint64_t p_id_2 = quad_tree::compute_quad_key(p, 2, bounds);
        Assert::AreEqual(20ull, p_id_2);
        uint64_t p_id_3 = quad_tree::compute_quad_key(p, 3, bounds);
        Assert::AreEqual(83ull, p_id_3);
      }

      {
        Point p = { 82, 24464, 2.80233169, -8.83230019 };
        uint64_t p_id_0 = quad_tree::compute_quad_key(p, 0, bounds);
        Assert::AreEqual(1ull, p_id_0);
        uint64_t p_id_1 = quad_tree::compute_quad_key(p, 1, bounds);
        Assert::AreEqual(5ull, p_id_1);
        uint64_t p_id_2 = quad_tree::compute_quad_key(p, 2, bounds);
        Assert::AreEqual(20ull, p_id_2);
        uint64_t p_id_3 = quad_tree::compute_quad_key(p, 3, bounds);
        Assert::AreEqual(82ull, p_id_3);
      }
    }

    TEST_METHOD(TestGenerateQuadKeyDepth2RectangleRegion)
    {
      srand(time(nullptr));
      Rect bb = { -16.0, -8.0, +16.0, +8.0 };
      //  xxxxxxxxooooooooxxxxxxxoooooooo < +8
      //  xx26xxxxoo27ooooxx30xxxoo31oooo
      //  xxxxxxxxooooooooxxxxxxxoooooooo
      //  ooooooooxxxxxxxxoooooooxxxxxxxx < +4
      //  oo24ooooxx25xxxxoo28oooxx29xxxx
      //  ooooooooxxxxxxxxoooooooxxxxxxxx
      //  xxxxxxxxooooooooxxxxxxxoooooooo < +0
      //  xx18xxxxoo19ooooxx22xxxoo23oooo
      //  xxxxxxxxooooooooxxxxxxxoooooooo
      //  ooooooooxxxxxxxxoooooooxxxxxxxx < -4
      //  oo16ooooxx17xxxxoo20oooxx21xxxx
      //  ooooooooxxxxxxxxoooooooxxxxxxxx
      //  ooooooooxxxxxxxxoooooooxxxxxxxx < -8
      //  ^      ^       ^      ^       ^
      //-16     -8       0     +8     +16

      auto actual = 0ull;
      int32_t r = rand();
      int8_t id = 0;
      uint8_t d = 2;

      for (auto row : { -8.0f, -4.1f, -4.0f }) {
        actual = quad_tree::compute_quad_key({ id++, r, -16.0f, row }, d, bb);
        Assert::AreEqual(16ull, actual);
        actual = quad_tree::compute_quad_key({ id++, r, - 8.1f, row }, d, bb);
        Assert::AreEqual(16ull, actual);
        actual = quad_tree::compute_quad_key({ id++, r, - 8.0f, row }, d, bb);
        Assert::AreEqual(16ull, actual);
        actual = quad_tree::compute_quad_key({ id++, r, - 7.9f, row }, d, bb);
        Assert::AreEqual(17ull, actual);
        actual = quad_tree::compute_quad_key({ id++, r, - 0.1f, row }, d, bb);
        Assert::AreEqual(17ull, actual);
        actual = quad_tree::compute_quad_key({ id++, r, + 0.0f, row }, d, bb);
        Assert::AreEqual(17ull, actual);
        actual = quad_tree::compute_quad_key({ id++, r, + 0.1f, row }, d, bb);
        Assert::AreEqual(20ull, actual);
        actual = quad_tree::compute_quad_key({ id++, r, + 7.9f, row }, d, bb);
        Assert::AreEqual(20ull, actual);
        actual = quad_tree::compute_quad_key({ id++, r, + 8.0f, row }, d, bb);
        Assert::AreEqual(20ull, actual);
        actual = quad_tree::compute_quad_key({ id++, r, + 8.1f, row }, d, bb);
        Assert::AreEqual(21ull, actual);
        actual = quad_tree::compute_quad_key({ id++, r, +16.1f, row }, d, bb);
        Assert::AreEqual(21ull, actual);
      }

      for (auto row : { -3.9f, -0.1f, +0.0f }) {
        actual = quad_tree::compute_quad_key({ id++, r, -16.0f, row }, d, bb);
        Assert::AreEqual(18ull, actual);
        actual = quad_tree::compute_quad_key({ id++, r, - 8.1f, row }, d, bb);
        Assert::AreEqual(18ull, actual);
        actual = quad_tree::compute_quad_key({ id++, r, - 8.0f, row }, d, bb);
        Assert::AreEqual(18ull, actual);
        actual = quad_tree::compute_quad_key({ id++, r, - 7.9f, row }, d, bb);
        Assert::AreEqual(19ull, actual);
        actual = quad_tree::compute_quad_key({ id++, r, - 0.1f, row }, d, bb);
        Assert::AreEqual(19ull, actual);
        actual = quad_tree::compute_quad_key({ id++, r, + 0.0f, row }, d, bb);
        Assert::AreEqual(19ull, actual);
        actual = quad_tree::compute_quad_key({ id++, r, + 0.1f, row }, d, bb);
        Assert::AreEqual(22ull, actual);
        actual = quad_tree::compute_quad_key({ id++, r, + 7.9f, row }, d, bb);
        Assert::AreEqual(22ull, actual);
        actual = quad_tree::compute_quad_key({ id++, r, + 8.0f, row }, d, bb);
        Assert::AreEqual(22ull, actual);
        actual = quad_tree::compute_quad_key({ id++, r, + 8.1f, row }, d, bb);
        Assert::AreEqual(23ull, actual);
        actual = quad_tree::compute_quad_key({ id++, r, +16.1f, row }, d, bb);
        Assert::AreEqual(23ull, actual);
      }

      for (auto row : { +0.1f, +3.9f, +4.0f }) {
        actual = quad_tree::compute_quad_key({ id++, r, -16.0f, row }, d, bb);
        Assert::AreEqual(24ull, actual);
        actual = quad_tree::compute_quad_key({ id++, r, - 8.1f, row }, d, bb);
        Assert::AreEqual(24ull, actual);
        actual = quad_tree::compute_quad_key({ id++, r, - 8.0f, row }, d, bb);
        Assert::AreEqual(24ull, actual);
        actual = quad_tree::compute_quad_key({ id++, r, - 7.9f, row }, d, bb);
        Assert::AreEqual(25ull, actual);
        actual = quad_tree::compute_quad_key({ id++, r, - 0.1f, row }, d, bb);
        Assert::AreEqual(25ull, actual);
        actual = quad_tree::compute_quad_key({ id++, r, + 0.0f, row }, d, bb);
        Assert::AreEqual(25ull, actual);
        actual = quad_tree::compute_quad_key({ id++, r, + 0.1f, row }, d, bb);
        Assert::AreEqual(28ull, actual);
        actual = quad_tree::compute_quad_key({ id++, r, + 7.9f, row }, d, bb);
        Assert::AreEqual(28ull, actual);
        actual = quad_tree::compute_quad_key({ id++, r, + 8.0f, row }, d, bb);
        Assert::AreEqual(28ull, actual);
        actual = quad_tree::compute_quad_key({ id++, r, + 8.1f, row }, d, bb);
        Assert::AreEqual(29ull, actual);
        actual = quad_tree::compute_quad_key({ id++, r, +16.1f, row }, d, bb);
        Assert::AreEqual(29ull, actual);
      }

      for (auto row : { +4.1f, +8.0f }) {
        actual = quad_tree::compute_quad_key({ id++, r, -16.0f, row }, d, bb);
        Assert::AreEqual(26ull, actual);
        actual = quad_tree::compute_quad_key({ id++, r, - 8.1f, row }, d, bb);
        Assert::AreEqual(26ull, actual);
        actual = quad_tree::compute_quad_key({ id++, r, - 8.0f, row }, d, bb);
        Assert::AreEqual(26ull, actual);
        actual = quad_tree::compute_quad_key({ id++, r, - 7.9f, row }, d, bb);
        Assert::AreEqual(27ull, actual);
        actual = quad_tree::compute_quad_key({ id++, r, - 0.1f, row }, d, bb);
        Assert::AreEqual(27ull, actual);
        actual = quad_tree::compute_quad_key({ id++, r, + 0.0f, row }, d, bb);
        Assert::AreEqual(27ull, actual);
        actual = quad_tree::compute_quad_key({ id++, r, + 0.1f, row }, d, bb);
        Assert::AreEqual(30ull, actual);
        actual = quad_tree::compute_quad_key({ id++, r, + 7.9f, row }, d, bb);
        Assert::AreEqual(30ull, actual);
        actual = quad_tree::compute_quad_key({ id++, r, + 8.0f, row }, d, bb);
        Assert::AreEqual(30ull, actual);
        actual = quad_tree::compute_quad_key({ id++, r, + 8.1f, row }, d, bb);
        Assert::AreEqual(31ull, actual);
        actual = quad_tree::compute_quad_key({ id++, r, +16.1f, row }, d, bb);
        Assert::AreEqual(31ull, actual);
      }
    }

    TEST_METHOD(ComputeChildrenAndParent)
    {
      //  |-----------------------------| +16
      //  |                             |
      //  |                             |
      //  |                             | + 8
      //  |                             |
      //  |                             |
      //  |              1              | + 0
      //  |                             |
      //  |                             |
      //  |                             | - 8
      //  |                             |
      //  |                             |
      //  |-----------------------------| -16
      //-16     -8       0     +8       +
      {
        quad_tree::Children_t depth0_children;
        quad_tree::compute_children(1ull, depth0_children);
        Assert::AreEqual(4ull, depth0_children[0]);
        Assert::AreEqual(5ull, depth0_children[1]);
        Assert::AreEqual(6ull, depth0_children[2]);
        Assert::AreEqual(7ull, depth0_children[3]);
      }

      //  |-----------------------------| +16
      //  |              |              |
      //  |              |              |
      //  |      6       |      7       | + 8
      //  |              |              |
      //  |              |              |
      //  |--------------|--------------| + 0
      //  |              |              |
      //  |              |              |
      //  |      4       |      5       | - 8
      //  |              |              |
      //  |              |              |
      //  |-----------------------------| -16
      //-16     -8       0     +8       +
      {
        {
          uint64_t parent = 0;
          parent = quad_tree::compute_parent(4ull);
          Assert::AreEqual(1ull, parent);
          parent = quad_tree::compute_parent(5ull);
          Assert::AreEqual(1ull, parent);
          parent = quad_tree::compute_parent(6ull);
          Assert::AreEqual(1ull, parent);
          parent = quad_tree::compute_parent(7ull);
          Assert::AreEqual(1ull, parent);
        }
        
        {
          quad_tree::Children_t depth1_children4;
          quad_tree::compute_children(4ull, depth1_children4);
          Assert::AreEqual(16ull, depth1_children4[0]);
          Assert::AreEqual(17ull, depth1_children4[1]);
          Assert::AreEqual(18ull, depth1_children4[2]);
          Assert::AreEqual(19ull, depth1_children4[3]);
        }

        {
          quad_tree::Children_t depth1_children5;
          quad_tree::compute_children(5ull, depth1_children5);
          Assert::AreEqual(20ull, depth1_children5[0]);
          Assert::AreEqual(21ull, depth1_children5[1]);
          Assert::AreEqual(22ull, depth1_children5[2]);
          Assert::AreEqual(23ull, depth1_children5[3]);
        }

        {
          quad_tree::Children_t depth1_children6;
          quad_tree::compute_children(6ull, depth1_children6);
          Assert::AreEqual(24ull, depth1_children6[0]);
          Assert::AreEqual(25ull, depth1_children6[1]);
          Assert::AreEqual(26ull, depth1_children6[2]);
          Assert::AreEqual(27ull, depth1_children6[3]);
        }

        {
          quad_tree::Children_t depth1_children7;
          quad_tree::compute_children(7ull, depth1_children7);
          Assert::AreEqual(28ull, depth1_children7[0]);
          Assert::AreEqual(29ull, depth1_children7[1]);
          Assert::AreEqual(30ull, depth1_children7[2]);
          Assert::AreEqual(31ull, depth1_children7[3]);
        }
      }

      //  |-----------------------------| +16
      //  | 26   |  27   |  30  |  31   |
      //  |      |       |      |       |
      //  |------|-------|------|-------| + 8
      //  | 24   |  25   |  28  |  29   |
      //  |      |       |      |       |
      //  |------|-------|------|-------| + 0
      //  | 18   |  19   |  22  |  23   |
      //  |      |       |      |       |
      //  |------|-------|------|-------| - 8
      //  | 16   |  17   |  20  |  21   |
      //  |      |       |      |       |
      //  |-----------------------------| -16
      //-16     -8       0     +8       +16
      {
        {
          {
            uint64_t parent = 0;
            parent = quad_tree::compute_parent(16ull);
            Assert::AreEqual(4ull, parent);
            parent = quad_tree::compute_parent(17ull);
            Assert::AreEqual(4ull, parent);
            parent = quad_tree::compute_parent(18ull);
            Assert::AreEqual(4ull, parent);
            parent = quad_tree::compute_parent(19ull);
            Assert::AreEqual(4ull, parent);
          }
          {
            uint64_t parent = 0;
            parent = quad_tree::compute_parent(20ull);
            Assert::AreEqual(5ull, parent);
            parent = quad_tree::compute_parent(21ull);
            Assert::AreEqual(5ull, parent);
            parent = quad_tree::compute_parent(22ull);
            Assert::AreEqual(5ull, parent);
            parent = quad_tree::compute_parent(23ull);
            Assert::AreEqual(5ull, parent);
          }
          {
            uint64_t parent = 0;
            parent = quad_tree::compute_parent(24ull);
            Assert::AreEqual(6ull, parent);
            parent = quad_tree::compute_parent(25ull);
            Assert::AreEqual(6ull, parent);
            parent = quad_tree::compute_parent(26ull);
            Assert::AreEqual(6ull, parent);
            parent = quad_tree::compute_parent(27ull);
            Assert::AreEqual(6ull, parent);
          }
          {
            uint64_t parent = 0;
            parent = quad_tree::compute_parent(28ull);
            Assert::AreEqual(7ull, parent);
            parent = quad_tree::compute_parent(29ull);
            Assert::AreEqual(7ull, parent);
            parent = quad_tree::compute_parent(30ull);
            Assert::AreEqual(7ull, parent);
            parent = quad_tree::compute_parent(31ull);
            Assert::AreEqual(7ull, parent);
          }
        }
        //  -------------------------------- +16
        //  |106|107|110|111|122|123|126|127|
        //  |---|---|---|---|---|---|---|---|
        //  |104|105|108|109|120|121|124|125|
        //  |---|---|---|---|---|---|---|---| + 8
        //  |98 |99 |102|103|114|115|118|119|
        //  |---|---|---|---|---|---|---|---|
        //  |96 |97 |100|101|112|113|116|117|
        //  |---|---|---|---|---|---|---|---| + 0
        //  |74 |75 |78 |79 |90 |91 |94 |95 |
        //  |---|---|---|---|---|---|---|---|
        //  |72 |73 |76 |77 |88 |89 |92 |93 |
        //  |---|---|---|---|---|---|---|---| - 8
        //  |66 |67 |70 |71 |82 |83 |86 |87 |
        //  |---|---|---|---|---|---|---|---|
        //  |64 |65 |68 |69 |80 |81 |84 |85 |
        //  -------------------------------- -16
        //-16     -8       0     +8       +16
        {
          std::vector<std::vector<uint64_t>> expected = {
            { 64ull, 65ull, 66ull, 67ull },
            { 68ull, 69ull, 70ull, 71ull },
            { 72ull, 73ull, 74ull, 75ull },
            { 76ull, 77ull, 78ull, 79ull }
          };
          std::size_t expected_index = 0;
          for (uint64_t parent : { 16ull, 17ull, 18ull, 19ull }) {
            quad_tree::Children_t children;
            quad_tree::compute_children(parent, children);
            Assert::AreEqual(expected[expected_index][0], children[0]);
            Assert::AreEqual(expected[expected_index][1], children[1]);
            Assert::AreEqual(expected[expected_index][2], children[2]);
            Assert::AreEqual(expected[expected_index][3], children[3]);
            ++expected_index;
          }
        }

        {
          std::vector<std::vector<uint64_t>> expected = {
            { 80ull, 81ull, 82ull, 83ull },
            { 84ull, 85ull, 86ull, 87ull },
            { 88ull, 89ull, 90ull, 91ull },
            { 92ull, 93ull, 94ull, 95ull }
          };
          std::size_t expected_index = 0;
          for (uint64_t parent : { 20ull, 21ull, 22ull, 23ull }) {
            quad_tree::Children_t children;
            quad_tree::compute_children(parent, children);
            Assert::AreEqual(expected[expected_index][0], children[0]);
            Assert::AreEqual(expected[expected_index][1], children[1]);
            Assert::AreEqual(expected[expected_index][2], children[2]);
            Assert::AreEqual(expected[expected_index][3], children[3]);
            ++expected_index;
          }
        }

        {
          std::vector<std::vector<uint64_t>> expected = {
            { 96ull, 97ull, 98ull, 99ull },
            { 100ull, 101ull, 102ull, 103ull },
            { 104ull, 105ull, 106ull, 107ull },
            { 108ull, 109ull, 110ull, 111ull },
          };
          std::size_t expected_index = 0;
          for (uint64_t parent : { 24ull, 25ull, 26ull, 27ull }) {
            quad_tree::Children_t children;
            quad_tree::compute_children(parent, children);
            Assert::AreEqual(expected[expected_index][0], children[0]);
            Assert::AreEqual(expected[expected_index][1], children[1]);
            Assert::AreEqual(expected[expected_index][2], children[2]);
            Assert::AreEqual(expected[expected_index][3], children[3]);
            ++expected_index;
          }
        }

        {
          std::vector<std::vector<uint64_t>> expected = {
            { 112ull, 113ull, 114ull, 115ull },
            { 116ull, 117ull, 118ull, 119ull },
            { 120ull, 121ull, 122ull, 123ull },
            { 124ull, 125ull, 126ull, 127ull }
          };
          std::size_t expected_index = 0;
          for (uint64_t parent : { 28ull, 29ull, 30ull, 31ull }) {
            quad_tree::Children_t children;
            quad_tree::compute_children(parent, children);
            Assert::AreEqual(expected[expected_index][0], children[0]);
            Assert::AreEqual(expected[expected_index][1], children[1]);
            Assert::AreEqual(expected[expected_index][2], children[2]);
            Assert::AreEqual(expected[expected_index][3], children[3]);
            ++expected_index;
          }
        }
      }
    }

    TEST_METHOD(TestMSB64)
    {
      {
        uint64_t test = 0x1;
        int8_t bit = quad_tree::msb64(test);
        Assert::AreEqual(static_cast<int8_t>(0), bit);
      }

      {
        uint64_t test = 0x10000000ull;
        int8_t bit = quad_tree::msb64(test);
        Assert::AreEqual(static_cast<int8_t>(28), bit);
      }

      {
        uint64_t test = 0x2000000000000000ull;
        int8_t bit = quad_tree::msb64(test);
        Assert::AreEqual(quad_tree::max_depth(), static_cast<uint8_t>(bit));
      }

      {
        uint64_t test = 0x20F000A0C0009000ull;
        int8_t bit = quad_tree::msb64(test);
        Assert::AreEqual(quad_tree::max_depth(), static_cast<uint8_t>(bit));
      }
    }

    TEST_METHOD(TestComputeStepSizeAtDepth)
    {
      Assert::AreEqual(0xFFFFFFFFu, quad_tree::step_size_at_depth(0));
      Assert::AreEqual(0x7FFFFFFFu, quad_tree::step_size_at_depth(1));
      Assert::AreEqual(0x3FFFFFFFu, quad_tree::step_size_at_depth(2));
      Assert::AreEqual(0x1FFFFFFFu, quad_tree::step_size_at_depth(3));
    }

    TEST_METHOD(TestComputeQuadKeyBoundsSquareDepth0)
    {
      Rect bounds = { -16.0f, -16.0f, +16.0f, +16.0f };
      //  |-----------------------------| +16
      //  |                             |
      //  |                             |
      //  |                             | + 8
      //  |                             |
      //  |                             |
      //  |              1              | + 0
      //  |                             |
      //  |                             |
      //  |                             | - 8
      //  |                             |
      //  |                             |
      //  |-----------------------------| -16
      //-16     -8       0     +8     +16
      {
        Rect level0_bounds;
        quad_tree::compute_bounds_for_quad_key(0x1ull, bounds, level0_bounds);
        Assert::AreEqual(-16.0f, level0_bounds.lx,
          std::numeric_limits<float>::epsilon());
        Assert::AreEqual(-16.0f, level0_bounds.ly,
          std::numeric_limits<float>::epsilon());
        Assert::AreEqual(+16.0f, level0_bounds.hx,
          std::numeric_limits<float>::epsilon());
        Assert::AreEqual(+16.0f, level0_bounds.hy,
          std::numeric_limits<float>::epsilon());
      }
    }

    TEST_METHOD(TestComputeQuadKeyBoundsSquareDepth1)
    {
      Rect bounds = { -16.0f, -16.0f, +16.0f, +16.0f };
      //  |-----------------------------| +16
      //  |              |              |
      //  |              |              |
      //  |      6       |      7       | + 8
      //  |              |              |
      //  |              |              |
      //  |--------------|--------------| + 0
      //  |              |              |
      //  |              |              |
      //  |      4       |      5       | - 8
      //  |              |              |
      //  |              |              |
      //  |-----------------------------| -16
      //-16     -8       0     +8     +16
      {
        Rect level1_bounds;

        quad_tree::compute_bounds_for_quad_key(0x4ull, bounds, level1_bounds);
        Assert::AreEqual(-16.0f, level1_bounds.lx,
          std::numeric_limits<float>::epsilon());
        Assert::AreEqual(-16.0f, level1_bounds.ly,
          std::numeric_limits<float>::epsilon());
        Assert::AreEqual(+ 0.0f, level1_bounds.hx,
          std::numeric_limits<float>::epsilon());
        Assert::AreEqual(+ 0.0f, level1_bounds.hy,
          std::numeric_limits<float>::epsilon());

        quad_tree::compute_bounds_for_quad_key(0x5ull, bounds, level1_bounds);
        Assert::AreEqual(+ 0.0f, level1_bounds.lx,
          std::numeric_limits<float>::epsilon());
        Assert::AreEqual(-16.0f, level1_bounds.ly,
          std::numeric_limits<float>::epsilon());
        Assert::AreEqual(+16.0f, level1_bounds.hx,
          std::numeric_limits<float>::epsilon());
        Assert::AreEqual(+ 0.0f, level1_bounds.hy,
          std::numeric_limits<float>::epsilon());

        quad_tree::compute_bounds_for_quad_key(0x6ull, bounds, level1_bounds);
        Assert::AreEqual(-16.0f, level1_bounds.lx,
          std::numeric_limits<float>::epsilon());
        Assert::AreEqual(+ 0.0f, level1_bounds.ly,
          std::numeric_limits<float>::epsilon());
        Assert::AreEqual(+ 0.0f, level1_bounds.hx,
          std::numeric_limits<float>::epsilon());
        Assert::AreEqual(+16.0f, level1_bounds.hy,
          std::numeric_limits<float>::epsilon());

        quad_tree::compute_bounds_for_quad_key(0x7ull, bounds, level1_bounds);
        Assert::AreEqual(+ 0.0f, level1_bounds.lx,
          std::numeric_limits<float>::epsilon());
        Assert::AreEqual(+ 0.0f, level1_bounds.ly,
          std::numeric_limits<float>::epsilon());
        Assert::AreEqual(+16.0f, level1_bounds.hx,
          std::numeric_limits<float>::epsilon());
        Assert::AreEqual(+16.0f, level1_bounds.hy,
          std::numeric_limits<float>::epsilon());
      }
    }


    TEST_METHOD(TestComputeQuadKeyBoundsSquareDepth2)
    {
      Rect bounds = { -16.0f, -16.0f, +16.0f, +16.0f };
      //  |-----------------------------| +16
      //  | 26   |  27   |  30  |  31   |
      //  |11010 |11011  |11110 |11111  |
      //  |------|-------|------|-------| + 8
      //  | 24   |  25   |  28  |  29   |
      //  |11000 |11001  |11100 |11101  |
      //  |------|-------|------|-------| + 0
      //  | 18   |  19   |  22  |  23   |
      //  |10010 |10011  |10110 |10111  |
      //  |------|-------|------|-------| - 8
      //  | 16   |  17   |  20  |  21   |
      //  |10000 |10001  |10100 |10101  |
      //  |-----------------------------| -16
      //-16     -8       0     +8       +16
      {
        Rect level2_bounds;

        quad_tree::compute_bounds_for_quad_key(16ull, bounds, level2_bounds);
        Assert::AreEqual(-16.0f, level2_bounds.lx,
          std::numeric_limits<float>::epsilon());
        Assert::AreEqual(-16.0f, level2_bounds.ly,
          std::numeric_limits<float>::epsilon());
        Assert::AreEqual(- 8.0f, level2_bounds.hx,
          std::numeric_limits<float>::epsilon());
        Assert::AreEqual(- 8.0f, level2_bounds.hy,
          std::numeric_limits<float>::epsilon());

        quad_tree::compute_bounds_for_quad_key(17ull, bounds, level2_bounds);
        Assert::AreEqual(- 8.0f, level2_bounds.lx,
          std::numeric_limits<float>::epsilon());
        Assert::AreEqual(-16.0f, level2_bounds.ly,
          std::numeric_limits<float>::epsilon());
        Assert::AreEqual(+ 0.0f, level2_bounds.hx,
          std::numeric_limits<float>::epsilon());
        Assert::AreEqual(- 8.0f, level2_bounds.hy,
          std::numeric_limits<float>::epsilon());

        quad_tree::compute_bounds_for_quad_key(20ull, bounds, level2_bounds);
        Assert::AreEqual(+ 0.0f, level2_bounds.lx,
          std::numeric_limits<float>::epsilon());
        Assert::AreEqual(-16.0f, level2_bounds.ly,
          std::numeric_limits<float>::epsilon());
        Assert::AreEqual(+ 8.0f, level2_bounds.hx,
          std::numeric_limits<float>::epsilon());
        Assert::AreEqual(- 8.0f, level2_bounds.hy,
          std::numeric_limits<float>::epsilon());

        quad_tree::compute_bounds_for_quad_key(21ull, bounds, level2_bounds);
        Assert::AreEqual(+ 8.0f, level2_bounds.lx,
          std::numeric_limits<float>::epsilon());
        Assert::AreEqual(-16.0f, level2_bounds.ly,
          std::numeric_limits<float>::epsilon());
        Assert::AreEqual(+16.0f, level2_bounds.hx,
          std::numeric_limits<float>::epsilon());
        Assert::AreEqual(- 8.0f, level2_bounds.hy,
          std::numeric_limits<float>::epsilon());

        quad_tree::compute_bounds_for_quad_key(18ull, bounds, level2_bounds);
        Assert::AreEqual(-16.0f, level2_bounds.lx,
          std::numeric_limits<float>::epsilon());
        Assert::AreEqual(- 8.0f, level2_bounds.ly,
          std::numeric_limits<float>::epsilon());
        Assert::AreEqual(- 8.0f, level2_bounds.hx,
          std::numeric_limits<float>::epsilon());
        Assert::AreEqual(+ 0.0f, level2_bounds.hy,
          std::numeric_limits<float>::epsilon());

        quad_tree::compute_bounds_for_quad_key(19ull, bounds, level2_bounds);
        Assert::AreEqual(- 8.0f, level2_bounds.lx,
          std::numeric_limits<float>::epsilon());
        Assert::AreEqual(- 8.0f, level2_bounds.ly,
          std::numeric_limits<float>::epsilon());
        Assert::AreEqual(+ 0.0f, level2_bounds.hx,
          std::numeric_limits<float>::epsilon());
        Assert::AreEqual(+ 0.0f, level2_bounds.hy,
          std::numeric_limits<float>::epsilon());

        quad_tree::compute_bounds_for_quad_key(22ull, bounds, level2_bounds);
        Assert::AreEqual(+ 0.0f, level2_bounds.lx,
          std::numeric_limits<float>::epsilon());
        Assert::AreEqual(- 8.0f, level2_bounds.ly,
          std::numeric_limits<float>::epsilon());
        Assert::AreEqual(+ 8.0f, level2_bounds.hx,
          std::numeric_limits<float>::epsilon());
        Assert::AreEqual(+ 0.0f, level2_bounds.hy,
          std::numeric_limits<float>::epsilon());

        quad_tree::compute_bounds_for_quad_key(23ull, bounds, level2_bounds);
        Assert::AreEqual(+ 8.0f, level2_bounds.lx,
          std::numeric_limits<float>::epsilon());
        Assert::AreEqual(- 8.0f, level2_bounds.ly,
          std::numeric_limits<float>::epsilon());
        Assert::AreEqual(+16.0f, level2_bounds.hx,
          std::numeric_limits<float>::epsilon());
        Assert::AreEqual(+ 0.0f, level2_bounds.hy,
          std::numeric_limits<float>::epsilon());

        quad_tree::compute_bounds_for_quad_key(24ull, bounds, level2_bounds);
        Assert::AreEqual(-16.0f, level2_bounds.lx,
          std::numeric_limits<float>::epsilon());
        Assert::AreEqual(+ 0.0f, level2_bounds.ly,
          std::numeric_limits<float>::epsilon());
        Assert::AreEqual(- 8.0f, level2_bounds.hx,
          std::numeric_limits<float>::epsilon());
        Assert::AreEqual(+ 8.0f, level2_bounds.hy,
          std::numeric_limits<float>::epsilon());

        quad_tree::compute_bounds_for_quad_key(25ull, bounds, level2_bounds);
        Assert::AreEqual(- 8.0f, level2_bounds.lx,
          std::numeric_limits<float>::epsilon());
        Assert::AreEqual(+ 0.0f, level2_bounds.ly,
          std::numeric_limits<float>::epsilon());
        Assert::AreEqual(+ 0.0f, level2_bounds.hx,
          std::numeric_limits<float>::epsilon());
        Assert::AreEqual(+ 8.0f, level2_bounds.hy,
          std::numeric_limits<float>::epsilon());

        quad_tree::compute_bounds_for_quad_key(28ull, bounds, level2_bounds);
        Assert::AreEqual(+ 0.0f, level2_bounds.lx,
          std::numeric_limits<float>::epsilon());
        Assert::AreEqual(+ 0.0f, level2_bounds.ly,
          std::numeric_limits<float>::epsilon());
        Assert::AreEqual(+ 8.0f, level2_bounds.hx,
          std::numeric_limits<float>::epsilon());
        Assert::AreEqual(+ 8.0f, level2_bounds.hy,
          std::numeric_limits<float>::epsilon());

        quad_tree::compute_bounds_for_quad_key(29ull, bounds, level2_bounds);
        Assert::AreEqual(+ 8.0f, level2_bounds.lx,
          std::numeric_limits<float>::epsilon());
        Assert::AreEqual(+ 0.0f, level2_bounds.ly,
          std::numeric_limits<float>::epsilon());
        Assert::AreEqual(+16.0f, level2_bounds.hx,
          std::numeric_limits<float>::epsilon());
        Assert::AreEqual(+ 8.0f, level2_bounds.hy,
          std::numeric_limits<float>::epsilon());

        quad_tree::compute_bounds_for_quad_key(26ull, bounds, level2_bounds);
        Assert::AreEqual(-16.0f, level2_bounds.lx,
          std::numeric_limits<float>::epsilon());
        Assert::AreEqual(+ 8.0f, level2_bounds.ly,
          std::numeric_limits<float>::epsilon());
        Assert::AreEqual(- 8.0f, level2_bounds.hx,
          std::numeric_limits<float>::epsilon());
        Assert::AreEqual(+16.0f, level2_bounds.hy,
          std::numeric_limits<float>::epsilon());

        quad_tree::compute_bounds_for_quad_key(27ull, bounds, level2_bounds);
        Assert::AreEqual(- 8.0f, level2_bounds.lx,
          std::numeric_limits<float>::epsilon());
        Assert::AreEqual(+ 8.0f, level2_bounds.ly,
          std::numeric_limits<float>::epsilon());
        Assert::AreEqual(+ 0.0f, level2_bounds.hx,
          std::numeric_limits<float>::epsilon());
        Assert::AreEqual(+16.0f, level2_bounds.hy,
          std::numeric_limits<float>::epsilon());

        quad_tree::compute_bounds_for_quad_key(30ull, bounds, level2_bounds);
        Assert::AreEqual(+ 0.0f, level2_bounds.lx,
          std::numeric_limits<float>::epsilon());
        Assert::AreEqual(+ 8.0f, level2_bounds.ly,
          std::numeric_limits<float>::epsilon());
        Assert::AreEqual(+ 8.0f, level2_bounds.hx,
          std::numeric_limits<float>::epsilon());
        Assert::AreEqual(+16.0f, level2_bounds.hy,
          std::numeric_limits<float>::epsilon());

        quad_tree::compute_bounds_for_quad_key(31ull, bounds, level2_bounds);
        Assert::AreEqual(+ 8.0f, level2_bounds.lx,
          std::numeric_limits<float>::epsilon());
        Assert::AreEqual(+ 8.0f, level2_bounds.ly,
          std::numeric_limits<float>::epsilon());
        Assert::AreEqual(+16.0f, level2_bounds.hx,
          std::numeric_limits<float>::epsilon());
        Assert::AreEqual(+16.0f, level2_bounds.hy,
          std::numeric_limits<float>::epsilon());
      }
    }

    TEST_METHOD(TestComputeQuadKeyBoundsSquareDepth3)
    {
      //  --------------------------------  +16
      //  |106|107|110|111|122|123|126|127|
      //  |---|---|---|---|---|---|---|---| +12
      //  |104|105|108|109|120|121|124|125|
      //  |---|---|---|---|---|---|---|---| + 8
      //  |98 |99 |102|103|114|115|118|119|
      //  |---|---|---|---|---|---|---|---| + 4
      //  |96 |97 |100|101|112|113|116|117|
      //  |---|---|---|---|---|---|---|---| + 0
      //  |74 |75 |78 |79 |90 |91 |94 |95 |
      //  |---|---|---|---|---|---|---|---| - 4
      //  |72 |73 |76 |77 |88 |89 |92 |93 |
      //  |---|---|---|---|---|---|---|---| - 8
      //  |66 |67 |70 |71 |82 |83 |86 |87 |
      //  |---|---|---|---|---|---|---|---| -12
      //  |64 |65 |68 |69 |80 |81 |84 |85 |
      //  --------------------------------  -16
      //-16 -12  -8  -4   0  +4  +8  +12  +16
      // TODO (mhoggan): Write all 64 unit tests.
    }

    TEST_METHOD(TestInArbitraryFloatingPointSpaceComputeBounds)
    {
      constexpr auto eps = std::numeric_limits<float>::epsilon();
      const Rect global_bounds = { -9683.0f, -9913.0f, 9916.0f, 9824.0f };
      {
        Rect bounds_4_depth1;
        quad_tree::compute_bounds_for_quad_key(
          4ull,
          global_bounds,
          bounds_4_depth1);
        Assert::AreEqual(-9683.0f, bounds_4_depth1.lx, eps);
        Assert::AreEqual(-9913.0f, bounds_4_depth1.ly, eps);
        Assert::AreEqual(+ 116.5f, bounds_4_depth1.hx, eps);
        Assert::AreEqual(-  44.5f, bounds_4_depth1.hy, eps);
      }
      {
        Rect bounds_5_depth1;
        quad_tree::compute_bounds_for_quad_key(
          5ull,
          global_bounds,
          bounds_5_depth1);
        Assert::AreEqual(+ 116.5f, bounds_5_depth1.lx, eps);
        Assert::AreEqual(-9913.0f, bounds_5_depth1.ly, eps);
        Assert::AreEqual(+9916.0f, bounds_5_depth1.hx, eps);
        Assert::AreEqual(-  44.5f, bounds_5_depth1.hy, eps);
      }
      {
        Rect bounds_6_depth1;
        quad_tree::compute_bounds_for_quad_key(
          6ull,
          global_bounds,
          bounds_6_depth1);
        Assert::AreEqual(-9683.0f, bounds_6_depth1.lx, eps);
        Assert::AreEqual(-  44.5f, bounds_6_depth1.ly, eps);
        Assert::AreEqual(+ 116.5f, bounds_6_depth1.hx, eps);
        Assert::AreEqual(+9824.0f, bounds_6_depth1.hy, eps);
      }
      {
        Rect bounds_7_depth1;
        quad_tree::compute_bounds_for_quad_key(
          7ull,
          global_bounds,
          bounds_7_depth1);
        Assert::AreEqual(+ 116.5f, bounds_7_depth1.lx, eps);
        Assert::AreEqual(-  44.5f, bounds_7_depth1.ly, eps);
        Assert::AreEqual(+9916.0f, bounds_7_depth1.hx, eps);
        Assert::AreEqual(+9824.0f, bounds_7_depth1.hy, eps);
      }
    }

    TEST_METHOD(TestInAllFloatingPointSpaceComputeBounds)
    {
      constexpr auto eps = std::numeric_limits<float>::epsilon();
      constexpr auto maxf = std::numeric_limits<float>::max();
      const Rect global_bounds = { -maxf, -maxf, +maxf, +maxf };
      {
        Rect bounds_4_depth1;
        quad_tree::compute_bounds_for_quad_key(
          4ull,
          global_bounds,
          bounds_4_depth1);
        Assert::AreEqual(-maxf, bounds_4_depth1.lx, eps);
        Assert::AreEqual(-maxf, bounds_4_depth1.ly, eps);
        Assert::AreEqual(+0.0f, bounds_4_depth1.hx, eps);
        Assert::AreEqual(-0.0f, bounds_4_depth1.hy, eps);
      }
      {
        Rect bounds_5_depth1;
        quad_tree::compute_bounds_for_quad_key(
          5ull,
          global_bounds,
          bounds_5_depth1);
        Assert::AreEqual(+0.0f, bounds_5_depth1.lx, eps);
        Assert::AreEqual(-maxf, bounds_5_depth1.ly, eps);
        Assert::AreEqual(+maxf, bounds_5_depth1.hx, eps);
        Assert::AreEqual(+0.0f, bounds_5_depth1.hy, eps);
      }
      {
        Rect bounds_6_depth1;
        quad_tree::compute_bounds_for_quad_key(
          6ull,
          global_bounds,
          bounds_6_depth1);
        Assert::AreEqual(-maxf, bounds_6_depth1.lx, eps);
        Assert::AreEqual(+0.0f, bounds_6_depth1.ly, eps);
        Assert::AreEqual(+0.0f, bounds_6_depth1.hx, eps);
        Assert::AreEqual(+maxf, bounds_6_depth1.hy, eps);
      }
      {
        Rect bounds_7_depth1;
        quad_tree::compute_bounds_for_quad_key(
          7ull,
          global_bounds,
          bounds_7_depth1);
        Assert::AreEqual(+0.0f, bounds_7_depth1.lx, eps);
        Assert::AreEqual(+0.0f, bounds_7_depth1.ly, eps);
        Assert::AreEqual(+maxf, bounds_7_depth1.hx, eps);
        Assert::AreEqual(+maxf, bounds_7_depth1.hy, eps);
      }
    }
	};
}
