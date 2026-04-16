#if USE_TEST
#include <ThirdParty/Catch2/catch.hpp>

#include "Component/WorldPos.hpp"

TEST_CASE("WorldPos::ToScreen - far objects have smaller y") {
  // 奥にあるものほどy座標が小さい
  WorldPos near{.w = 0.0, .h = 0.0, .d = 100.0};
  WorldPos far{.w = 0.0, .h = 0.0, .d = 500.0};
  REQUIRE(near.toScreen().y > far.toScreen().y);
}

TEST_CASE("WorldPos::ToScreen - higher objects have smaller y") {
  // 高いものほどy座標が小さい
  WorldPos ground{.w = 0.0, .h = 0.0, .d = 300.0};
  WorldPos jump{.w = 0.0, .h = 60.0, .d = 300.0};
  REQUIRE(jump.toScreen().y < ground.toScreen().y);
}

TEST_CASE("WorldPos::ToScreen - x maps to horizontal position") {
  // x座標は横位置に対応する
  WorldPos left{.w = 100.0, .h = 0.0, .d = 300.0};
  WorldPos right{.w = 500.0, .h = 0.0, .d = 300.0};
  REQUIRE(left.toScreen().x < right.toScreen().x);
}

TEST_CASE("WorldPos::isOnGround - on ground when h is 0") {
  // h=0 は地面上
  WorldPos pos{.w = 0.0, .h = 0.0, .d = 0.0};
  REQUIRE(pos.isOnGround());
}

TEST_CASE("WorldPos::isOnGround - not on ground when h is positive") {
  // h>0 は地面上ではない
  WorldPos pos{.w = 0.0, .h = 1.0, .d = 0.0};
  REQUIRE_FALSE(pos.isOnGround());
}

TEST_CASE("DrawOrderLess - far objects come first") {
  // 奥のオブジェクトが先に来る
  WorldPos near{.w = 0.0, .h = 0.0, .d = 100.0};
  WorldPos far{.w = 0.0, .h = 0.0, .d = 500.0};
  REQUIRE(DrawOrderLess(far, near));
  REQUIRE_FALSE(DrawOrderLess(near, far));
}

#endif
