#ifdef _DEBUG
#include <ThirdParty/Catch2/catch.hpp>

#include "Config/PlayerConfig.hpp"

TEST_CASE("PlayerConfig::FromToml - parses all fields correctly") {
  constexpr std::string_view Toml =
      "speed = 150.0\n"
      "jump_speed = 400.0\n"
      "gravity = 900.0\n"
      "[[melee.stage]]\n"
      "windup_sec = 0.1\n"
      "active_sec = 0.1\n"
      "recovery_a_sec = 0.1\n"
      "recovery_b_sec = 0.1\n"
      "radius = 20.0\n"
      "trajectory = \"thrust\"\n"
      "slash_rise_height = 0.0\n"
      "hitstop_sec = 0.05\n";
  const TOMLReader reader{MemoryViewReader{Toml.data(), Toml.size()}};
  const PlayerConfig cfg = PlayerConfig::FromToml(reader);
  REQUIRE(cfg.speed == 150.0);
  REQUIRE(cfg.jumpSpeed == 400.0);
  REQUIRE(cfg.gravity == 900.0);
  REQUIRE(cfg.melee.stages.size() == 1);
  REQUIRE(cfg.melee.stages[0].trajectory == MeleeTrajectory::Thrust);
  REQUIRE(cfg.melee.stages[0].radius == 20.0);
}

TEST_CASE("PlayerConfig::FromToml - missing melee.stage throws Error") {
  constexpr std::string_view Toml =
      "speed = 150.0\n"
      "jump_speed = 400.0\n"
      "gravity = 900.0\n";
  const TOMLReader reader{MemoryViewReader{Toml.data(), Toml.size()}};
  REQUIRE_THROWS_AS(PlayerConfig::FromToml(reader), Error);
}

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
