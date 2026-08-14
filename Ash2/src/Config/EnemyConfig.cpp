#include "EnemyConfig.hpp"

#include "Config/TomlFields.hpp"

namespace {

/// @brief TOML から EnemyConfig を生成する
[[nodiscard]] std::expected<EnemyConfig, String> Parse(const TOMLValue& toml) {
  TomlFields f{toml, U"EnemyConfig::Parse"};
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

}  // namespace

EnemyConfig EnemyConfig::FromToml(const TOMLValue& toml) {
  auto parsed = Parse(toml);
  if (!parsed) {
    // Why not: Parse からの失敗は expected で伝播してくるが、ここは設定
    // 読み込みの最上位境界であり、呼び出し元の GameSetup は expected を
    // 扱わない設計のため throw で致命として確定させる。
    throw Error{std::move(parsed).error()};
  }
  return *std::move(parsed);
}
