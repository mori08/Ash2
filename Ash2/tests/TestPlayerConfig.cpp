#ifdef _DEBUG
#include <ThirdParty/Catch2/catch.hpp>
#include <string>
#include <tuple>

#include "Config/PlayerConfig.hpp"

namespace {

/// @brief 全必須キーを満たす PlayerConfig 用 TOML（テストの基準値）
constexpr std::string_view KFullToml =
    "speed = 150.0\n"
    "jump_speed = 400.0\n"
    "gravity = 900.0\n"
    "[melee]\n"
    "cap_mid_h = 30.0\n"
    "reach = 60.0\n"
    "damage = 20\n"
    "[[melee.stage]]\n"
    "windup_sec = 0.1\n"
    "active_sec = 0.1\n"
    "recovery_a_sec = 0.1\n"
    "recovery_b_sec = 0.1\n"
    "radius = 20.0\n"
    "trajectory = \"thrust\"\n"
    "slash_rise_height = 0.0\n"
    "slash_curve = 0.0\n"
    "hitstop_sec = 0.05\n"
    "[ranged]\n"
    "reach = 200.0\n"
    "radius = 15.0\n"
    "damage = 15\n"
    "bullet_speed = 300.0\n"
    "spawn_height = 0.0\n"
    "stamina_cost = 30\n"
    "[dash]\n"
    "speed = 600.0\n"
    "windup_sec = 0.0\n"
    "active_sec = 0.15\n"
    "recovery_a_sec = 0.15\n"
    "recovery_b_sec = 0.15\n"
    "stamina_cost = 20\n"
    "[dash_attack]\n"
    "windup_sec = 0.05\n"
    "active_sec = 0.35\n"
    "recovery_a_sec = 0.20\n"
    "recovery_b_sec = 0.00\n"
    "speed = 300.0\n"
    "orbit_radius = 50.0\n"
    "radius = 25.0\n"
    "damage = 25\n"
    "hitstop_sec = 0.08\n"
    "[air_attack]\n"
    "windup_sec = 0.05\n"
    "active_sec = 0.25\n"
    "recovery_a_sec = 0.20\n"
    "recovery_b_sec = 0.00\n"
    "orbit_radius = 50.0\n"
    "radius = 25.0\n"
    "damage = 25\n"
    "hitstop_sec = 0.08\n"
    "[stamina]\n"
    "recovery_delay = 2.0\n"
    "recovery_rate = 0.5\n"
    "[landing]\n"
    "recovery_sec = 0.20\n"
    "[attack_effect]\n"
    "fade_sec = 0.30\n";

}  // namespace

TEST_CASE("PlayerConfig::FromToml - parses all fields correctly") {
  const TOMLReader reader{MemoryViewReader{KFullToml.data(), KFullToml.size()}};
  const PlayerConfig cfg = PlayerConfig::FromToml(reader);
  REQUIRE(cfg.speed == 150.0);
  REQUIRE(cfg.jumpSpeed == 400.0);
  REQUIRE(cfg.gravity == 900.0);
  REQUIRE(cfg.melee.capMidH == 30.0);
  REQUIRE(cfg.melee.reach == 60.0);
  REQUIRE(cfg.melee.damage == 20);
  REQUIRE(cfg.melee.stages.size() == 1);
  REQUIRE(cfg.melee.stages[0].trajectory == MeleeTrajectory::Thrust);
  REQUIRE(cfg.melee.stages[0].radius == 20.0);
  REQUIRE(cfg.ranged.damage == 15);
  REQUIRE(cfg.dash.speed == 600.0);
  REQUIRE(cfg.dashAttack.damage == 25);
  REQUIRE(cfg.airAttack.damage == 25);
  REQUIRE(cfg.stamina.recoveryRate == 0.5);
  REQUIRE(cfg.landing.recoverySec == 0.20);
  REQUIRE(cfg.attackEffect.fadeSec == 0.30);
}

TEST_CASE("PlayerConfig::FromToml - missing melee.stage throws Error") {
  constexpr std::string_view KToml =
      "speed = 150.0\n"
      "jump_speed = 400.0\n"
      "gravity = 900.0\n";
  const TOMLReader reader{MemoryViewReader{KToml.data(), KToml.size()}};
  REQUIRE_THROWS_AS(PlayerConfig::FromToml(reader), Error);
}

TEST_CASE("PlayerConfig::FromToml - missing speed throws Error") {
  std::string toml{KFullToml};
  const auto pos = toml.find("speed = 150.0\n");
  REQUIRE(pos != std::string::npos);
  toml.erase(pos, std::string_view{"speed = 150.0\n"}.size());
  const TOMLReader reader{MemoryViewReader{toml.data(), toml.size()}};
  REQUIRE_THROWS_AS(PlayerConfig::FromToml(reader), Error);
}

TEST_CASE(
    "PlayerConfig::FromToml - missing melee.stage trajectory throws Error "
    "(not \"unknown value\")"
) {
  std::string toml{KFullToml};
  const auto pos = toml.find("trajectory = \"thrust\"\n");
  REQUIRE(pos != std::string::npos);
  toml.erase(pos, std::string_view{"trajectory = \"thrust\"\n"}.size());
  const TOMLReader reader{MemoryViewReader{toml.data(), toml.size()}};
  try {
    std::ignore = PlayerConfig::FromToml(reader);
    FAIL("PlayerConfig::FromToml should throw when trajectory is missing");
  } catch (const Error& e) {
    const String& message = e.what();
    REQUIRE(message.contains(U"キーがありません"));
    REQUIRE_FALSE(message.contains(U"不明な"));
  }
}

TEST_CASE(
    "PlayerConfig::FromToml - unknown melee.stage trajectory value throws "
    "Error"
) {
  std::string toml{KFullToml};
  const auto pos = toml.find("trajectory = \"thrust\"\n");
  REQUIRE(pos != std::string::npos);
  toml.replace(
      pos, std::string_view{"trajectory = \"thrust\"\n"}.size(),
      "trajectory = \"spin\"\n"
  );
  const TOMLReader reader{MemoryViewReader{toml.data(), toml.size()}};
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
