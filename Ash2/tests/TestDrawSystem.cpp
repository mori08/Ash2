#ifdef _DEBUG
#include <ThirdParty/Catch2/catch.hpp>

#include "System/DrawSystem.hpp"

TEST_CASE("DrawOrderLess - far objects come first") {
  // 奥のオブジェクトが先に来る
  DrawOrderKey near{.d = 100.0, .entity = entt::entity{1}};
  DrawOrderKey far{.d = 500.0, .entity = entt::entity{2}};
  REQUIRE(DrawOrderLess(far, near));
  REQUIRE_FALSE(DrawOrderLess(near, far));
}

TEST_CASE("DrawOrderLess - same depth breaks tie by entity ascending") {
  // dが等しい場合はentityの昇順になる
  DrawOrderKey lower{.d = 100.0, .entity = entt::entity{1}};
  DrawOrderKey higher{.d = 100.0, .entity = entt::entity{2}};
  REQUIRE(DrawOrderLess(lower, higher));
  REQUIRE_FALSE(DrawOrderLess(higher, lower));
}

#endif
