#if USE_TEST
#include <ThirdParty/Catch2/catch.hpp>

#include "Config/PlayerConfig.hpp"

TEST_CASE("PlayerConfig - can set speed directly") {
  // 直接値をセットできる（TOML なしでテスト可能）
  const PlayerConfig cfg{.speed = 100.0, .jumpSpeed = 300.0, .gravity = 800.0};
  REQUIRE(cfg.speed == 100.0);
  REQUIRE(cfg.jumpSpeed == 300.0);
  REQUIRE(cfg.gravity == 800.0);
}

TEST_CASE("PlayerConfig - speed is positive") {
  // 速度は正の値であること
  const PlayerConfig cfg{.speed = 100.0, .jumpSpeed = 300.0, .gravity = 800.0};
  REQUIRE(cfg.speed > 0.0);
}

TEST_CASE("PlayerConfig - gravity is positive") {
  // 重力加速度は正の値であること
  const PlayerConfig cfg{.speed = 100.0, .jumpSpeed = 300.0, .gravity = 800.0};
  REQUIRE(cfg.gravity > 0.0);
}

#endif
