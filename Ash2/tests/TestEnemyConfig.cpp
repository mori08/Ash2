#ifdef _DEBUG
#include <ThirdParty/Catch2/catch.hpp>

#include "Config/EnemyConfig.hpp"

namespace {
constexpr std::string_view kValidToml =
    "max_hp = 100\n"
    "capsule_radius = 24.0\n"
    "capsule_height = 56.0\n"
    "spawn_w = 150.0\n"
    "stagger_sec = 0.15\n"
    "repel_speed = 250.0\n"
    "repel_sec = 0.20\n"
    "blow_speed_w = 300.0\n"
    "blow_speed_h = 300.0\n"
    "knockback_sec = 1.00\n"
    "defeated_sec = 0.50\n"
    "respawn_sec = 1.00\n"
    "move_speed = 90.0\n"
    "aggro_range = 400.0\n"
    "leap_range = 180.0\n"
    "windup_sec = 0.50\n"
    "leap_speed_w = 260.0\n"
    "leap_speed_h = 350.0\n"
    "landing_sec = 0.60\n"
    "attack_damage = 10\n"
    "attack_hitstop_sec = 0.05\n"
    "attack_reaction = \"stagger\"\n";
}  // namespace

TEST_CASE("EnemyConfig::FromToml - parses all fields correctly") {
  const TOMLReader reader{
      MemoryViewReader{kValidToml.data(), kValidToml.size()}
  };
  const auto cfg = EnemyConfig::FromToml(reader);
  REQUIRE(cfg.has_value());
  REQUIRE(cfg->maxHp == 100);
  REQUIRE(cfg->capsuleRadius == 24.0);
  REQUIRE(cfg->capsuleHeight == 56.0);
  REQUIRE(cfg->spawnW == 150.0);
  REQUIRE(cfg->staggerSec == 0.15);
  REQUIRE(cfg->repelSpeed == 250.0);
  REQUIRE(cfg->repelSec == 0.20);
  REQUIRE(cfg->blowSpeedW == 300.0);
  REQUIRE(cfg->blowSpeedH == 300.0);
  REQUIRE(cfg->knockbackSec == 1.00);
  REQUIRE(cfg->defeatedSec == 0.50);
  REQUIRE(cfg->respawnSec == 1.00);
  REQUIRE(cfg->moveSpeed == 90.0);
  REQUIRE(cfg->aggroRange == 400.0);
  REQUIRE(cfg->leapRange == 180.0);
  REQUIRE(cfg->windupSec == 0.50);
  REQUIRE(cfg->leapSpeedW == 260.0);
  REQUIRE(cfg->leapSpeedH == 350.0);
  REQUIRE(cfg->landingSec == 0.60);
  REQUIRE(cfg->attackDamage == 10);
  REQUIRE(cfg->attackHitstopSec == 0.05);
  REQUIRE(cfg->attackReaction == ReactionLevel::Stagger);
}

TEST_CASE("EnemyConfig::FromToml - missing max_hp returns unexpected") {
  constexpr std::string_view kToml =
      "capsule_radius = 24.0\n"
      "capsule_height = 56.0\n"
      "spawn_w = 150.0\n"
      "stagger_sec = 0.15\n"
      "repel_speed = 250.0\n"
      "repel_sec = 0.20\n"
      "blow_speed_w = 300.0\n"
      "blow_speed_h = 300.0\n"
      "knockback_sec = 1.00\n"
      "defeated_sec = 0.50\n"
      "respawn_sec = 1.00\n"
      "move_speed = 90.0\n"
      "aggro_range = 400.0\n"
      "leap_range = 180.0\n"
      "windup_sec = 0.50\n"
      "leap_speed_w = 260.0\n"
      "leap_speed_h = 350.0\n"
      "landing_sec = 0.60\n"
      "attack_damage = 10\n"
      "attack_hitstop_sec = 0.05\n"
      "attack_reaction = \"stagger\"\n";
  const TOMLReader reader{MemoryViewReader{kToml.data(), kToml.size()}};
  REQUIRE_FALSE(EnemyConfig::FromToml(reader).has_value());
}

TEST_CASE(
    "EnemyConfig::FromToml - unknown attack_reaction returns unexpected"
) {
  constexpr std::string_view kToml =
      "max_hp = 100\n"
      "capsule_radius = 24.0\n"
      "capsule_height = 56.0\n"
      "spawn_w = 150.0\n"
      "stagger_sec = 0.15\n"
      "repel_speed = 250.0\n"
      "repel_sec = 0.20\n"
      "blow_speed_w = 300.0\n"
      "blow_speed_h = 300.0\n"
      "knockback_sec = 1.00\n"
      "defeated_sec = 0.50\n"
      "respawn_sec = 1.00\n"
      "move_speed = 90.0\n"
      "aggro_range = 400.0\n"
      "leap_range = 180.0\n"
      "windup_sec = 0.50\n"
      "leap_speed_w = 260.0\n"
      "leap_speed_h = 350.0\n"
      "landing_sec = 0.60\n"
      "attack_damage = 10\n"
      "attack_hitstop_sec = 0.05\n"
      "attack_reaction = \"unknown\"\n";
  const TOMLReader reader{MemoryViewReader{kToml.data(), kToml.size()}};
  REQUIRE_FALSE(EnemyConfig::FromToml(reader).has_value());
}

#endif
