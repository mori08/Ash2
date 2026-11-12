#ifdef _DEBUG
#include <ThirdParty/Catch2/catch.hpp>

#include "System/DrawSystem.hpp"

TEST_CASE("DrawOrderLess - far objects come first") {
  // 奥のオブジェクトが先に来る
  DrawOrderKey near{.depth = 100.0, .entity = entt::entity{1}};
  DrawOrderKey far{.depth = 500.0, .entity = entt::entity{2}};
  REQUIRE(DrawOrderLess(far, near));
  REQUIRE_FALSE(DrawOrderLess(near, far));
}

TEST_CASE("DrawOrderLess - same depth breaks tie by entity ascending") {
  // depthが等しい場合はentityの昇順になる
  DrawOrderKey lower{.depth = 100.0, .entity = entt::entity{1}};
  DrawOrderKey higher{.depth = 100.0, .entity = entt::entity{2}};
  REQUIRE(DrawOrderLess(lower, higher));
  REQUIRE_FALSE(DrawOrderLess(higher, lower));
}

TEST_CASE("DrawOrderLess - lower layer comes first regardless of depth") {
  // layerが優先され、depthは同じlayer内でのみ効く
  DrawOrderKey back{.layer = 0, .depth = 0.0, .entity = entt::entity{2}};
  DrawOrderKey front{.layer = 100, .depth = 900.0, .entity = entt::entity{1}};
  REQUIRE(DrawOrderLess(back, front));
  REQUIRE_FALSE(DrawOrderLess(front, back));
}

#endif
