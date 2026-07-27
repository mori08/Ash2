#ifdef _DEBUG
#include <ThirdParty/Catch2/catch.hpp>

#include "Config/EnemyConfig.hpp"

TEST_CASE("EnemyConfig::FromToml - parses all fields correctly") {
  constexpr std::string_view Toml =
      "max_hp = 100\n"
      "size_w = 60.0\n"
      "size_h = 80.0\n"
      "capsule_radius = 30.0\n"
      "capsule_height = 80.0\n"
      "spawn_w = 150.0\n"
      "stagger_sec = 0.15\n"
      "repel_speed = 250.0\n"
      "repel_sec = 0.20\n"
      "blow_speed_w = 300.0\n"
      "blow_speed_h = 300.0\n"
      "knockback_sec = 1.00\n"
      "defeated_sec = 0.50\n"
      "respawn_sec = 1.00\n";
  const TOMLReader reader{MemoryViewReader{Toml.data(), Toml.size()}};
  const EnemyConfig cfg = EnemyConfig::FromToml(reader);
  REQUIRE(cfg.maxHp == 100);
  REQUIRE(cfg.size.x == 60.0);
  REQUIRE(cfg.size.y == 80.0);
  REQUIRE(cfg.capsuleRadius == 30.0);
  REQUIRE(cfg.capsuleHeight == 80.0);
  REQUIRE(cfg.spawnW == 150.0);
  REQUIRE(cfg.staggerSec == 0.15);
  REQUIRE(cfg.repelSpeed == 250.0);
  REQUIRE(cfg.repelSec == 0.20);
  REQUIRE(cfg.blowSpeedW == 300.0);
  REQUIRE(cfg.blowSpeedH == 300.0);
  REQUIRE(cfg.knockbackSec == 1.00);
  REQUIRE(cfg.defeatedSec == 0.50);
  REQUIRE(cfg.respawnSec == 1.00);
}

TEST_CASE("EnemyConfig::FromToml - missing max_hp throws Error") {
  constexpr std::string_view Toml =
      "size_w = 60.0\n"
      "size_h = 80.0\n"
      "capsule_radius = 30.0\n"
      "capsule_height = 80.0\n"
      "spawn_w = 150.0\n"
      "stagger_sec = 0.15\n"
      "repel_speed = 250.0\n"
      "repel_sec = 0.20\n"
      "blow_speed_w = 300.0\n"
      "blow_speed_h = 300.0\n"
      "knockback_sec = 1.00\n"
      "defeated_sec = 0.50\n"
      "respawn_sec = 1.00\n";
  const TOMLReader reader{MemoryViewReader{Toml.data(), Toml.size()}};
  REQUIRE_THROWS_AS(EnemyConfig::FromToml(reader), Error);
}

#endif
