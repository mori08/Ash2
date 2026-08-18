#include "EnemyConfig.hpp"

#include "Config/TomlFields.hpp"

std::expected<EnemyConfig, String> EnemyConfig::FromToml(
    const TOMLValue& toml
) {
  TomlFields f{toml, U"EnemyConfig::FromToml"};
  return f.wrap(
      EnemyConfig{
          .maxHp = f.get<int32>(U"max_hp"),
          .size = {f.get<double>(U"size_w"), f.get<double>(U"size_h")},
          .capsuleRadius = f.get<double>(U"capsule_radius"),
          .capsuleHeight = f.get<double>(U"capsule_height"),
          .spawnW = f.get<double>(U"spawn_w"),
          .staggerSec = f.get<double>(U"stagger_sec"),
          .repelSpeed = f.get<double>(U"repel_speed"),
          .repelSec = f.get<double>(U"repel_sec"),
          .blowSpeedW = f.get<double>(U"blow_speed_w"),
          .blowSpeedH = f.get<double>(U"blow_speed_h"),
          .knockbackSec = f.get<double>(U"knockback_sec"),
          .defeatedSec = f.get<double>(U"defeated_sec"),
          .respawnSec = f.get<double>(U"respawn_sec"),
      }
  );
}
