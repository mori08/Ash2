#include "EnemyConfig.hpp"

#include "Config/TomlFields.hpp"

std::expected<EnemyConfig, String> EnemyConfig::FromToml(
    const TOMLValue& toml
) {
  TomlFields f{toml, U"EnemyConfig::FromToml"};
  const auto maxHp = f.get<int32>(U"max_hp");
  const auto capsuleRadius = f.get<double>(U"capsule_radius");
  const auto capsuleHeight = f.get<double>(U"capsule_height");
  const auto spawnW = f.get<double>(U"spawn_w");
  const auto staggerSec = f.get<double>(U"stagger_sec");
  const auto repelSpeed = f.get<double>(U"repel_speed");
  const auto repelSec = f.get<double>(U"repel_sec");
  const auto blowSpeedW = f.get<double>(U"blow_speed_w");
  const auto blowSpeedH = f.get<double>(U"blow_speed_h");
  const auto knockbackSec = f.get<double>(U"knockback_sec");
  const auto defeatedSec = f.get<double>(U"defeated_sec");
  const auto respawnSec = f.get<double>(U"respawn_sec");
  const auto moveSpeed = f.get<double>(U"move_speed");
  const auto aggroRange = f.get<double>(U"aggro_range");
  const auto leapRange = f.get<double>(U"leap_range");
  const auto windupSec = f.get<double>(U"windup_sec");
  const auto leapSpeedW = f.get<double>(U"leap_speed_w");
  const auto leapSpeedH = f.get<double>(U"leap_speed_h");
  const auto landingSec = f.get<double>(U"landing_sec");
  const auto attackDamage = f.get<int32>(U"attack_damage");
  const auto attackHitstopSec = f.get<double>(U"attack_hitstop_sec");
  const auto attackReactionStr = f.get<String>(U"attack_reaction");
  // Why not: reaction の変換前に check() で欠落を確定させる。変換を先に
  // 行うと、欠落時の既定値 String{}（空文字列）が「不明な reaction」と
  // 誤報されてしまうため。
  if (auto result = f.check(); !result) {
    return std::unexpected{std::move(result).error()};
  }

  auto attackReaction = ParseReactionLevel(attackReactionStr);
  if (!attackReaction) {
    return std::unexpected{std::move(attackReaction).error()};
  }

  return EnemyConfig{
      .maxHp = maxHp,
      .capsuleRadius = capsuleRadius,
      .capsuleHeight = capsuleHeight,
      .spawnW = spawnW,
      .staggerSec = staggerSec,
      .repelSpeed = repelSpeed,
      .repelSec = repelSec,
      .blowSpeedW = blowSpeedW,
      .blowSpeedH = blowSpeedH,
      .knockbackSec = knockbackSec,
      .defeatedSec = defeatedSec,
      .respawnSec = respawnSec,
      .moveSpeed = moveSpeed,
      .aggroRange = aggroRange,
      .leapRange = leapRange,
      .windupSec = windupSec,
      .leapSpeedW = leapSpeedW,
      .leapSpeedH = leapSpeedH,
      .landingSec = landingSec,
      .attackDamage = attackDamage,
      .attackHitstopSec = attackHitstopSec,
      .attackReaction = *attackReaction,
  };
}
