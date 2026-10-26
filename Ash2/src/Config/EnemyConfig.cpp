#include "EnemyConfig.hpp"

#include "Config/TomlTable.hpp"

EnemyConfig EnemyConfig::FromToml(const TOMLValue& toml) {
  const TomlTable t{toml};
  return EnemyConfig{
      .maxHp = t.get<int32>(U"max_hp"),
      .capsuleRadius = t.get<double>(U"capsule_radius"),
      .capsuleHeight = t.get<double>(U"capsule_height"),
      .staggerSec = t.get<double>(U"stagger_sec"),
      .repelSpeed = t.get<double>(U"repel_speed"),
      .repelSec = t.get<double>(U"repel_sec"),
      .blowSpeedW = t.get<double>(U"blow_speed_w"),
      .blowSpeedH = t.get<double>(U"blow_speed_h"),
      .knockbackSec = t.get<double>(U"knockback_sec"),
      .defeatedSec = t.get<double>(U"defeated_sec"),
      .moveSpeed = t.get<double>(U"move_speed"),
      .aggroRange = t.get<double>(U"aggro_range"),
      .leapRange = t.get<double>(U"leap_range"),
      .windupSec = t.get<double>(U"windup_sec"),
      .leapSpeedW = t.get<double>(U"leap_speed_w"),
      .leapSpeedH = t.get<double>(U"leap_speed_h"),
      .landingSec = t.get<double>(U"landing_sec"),
      .attackDamage = t.get<int32>(U"attack_damage"),
      .attackHitstopSec = t.get<double>(U"attack_hitstop_sec"),
      .attackReaction = ParseReactionLevel(t.get<String>(U"attack_reaction")),
  };
}
