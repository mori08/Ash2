#include "EnemyConfig.hpp"

EnemyConfig EnemyConfig::FromToml(const TOMLValue& toml) {
  return {
      .maxHp = toml[U"max_hp"].get<int32>(),
      .size = {toml[U"size_w"].get<double>(), toml[U"size_h"].get<double>()},
      .capsuleRadius = toml[U"capsule_radius"].get<double>(),
      .capsuleHeight = toml[U"capsule_height"].get<double>(),
      .spawnW = toml[U"spawn_w"].get<double>(),
      .staggerSec = toml[U"stagger_sec"].get<double>(),
      .repelSpeed = toml[U"repel_speed"].get<double>(),
      .repelSec = toml[U"repel_sec"].get<double>(),
      .blowSpeedW = toml[U"blow_speed_w"].get<double>(),
      .blowSpeedH = toml[U"blow_speed_h"].get<double>(),
      .knockbackSec = toml[U"knockback_sec"].get<double>(),
      .defeatedSec = toml[U"defeated_sec"].get<double>(),
      .respawnSec = toml[U"respawn_sec"].get<double>(),
  };
}
