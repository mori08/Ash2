#include "PlayerConfig.hpp"

namespace {

/// @brief TOML 値から MotionTimeline を生成する
/// @note windup_sec/active_sec/recovery_a_sec/recovery_b_sec
/// の4キーを持つテーブルであればよく、melee の段テーブルのように
/// 他のキーを併せ持っていても構わない
MotionTimeline ParseTimeline(const TOMLValue& toml) {
  return MotionTimeline{
      .windupSec = toml[U"windup_sec"].get<double>(),
      .activeSec = toml[U"active_sec"].get<double>(),
      .recoveryASec = toml[U"recovery_a_sec"].get<double>(),
      .recoveryBSec = toml[U"recovery_b_sec"].get<double>(),
  };
}

/// @brief TOML の trajectory 文字列を MeleeTrajectory へ変換する
// 初期化経路の設定読み込みであり、expected を返せない FromToml
// の下請けのため、不明な値は throw で致命として確定させる
MeleeTrajectory ParseMeleeTrajectory(const String& value) {
  if (value == U"thrust") return MeleeTrajectory::Thrust;
  if (value == U"slash") return MeleeTrajectory::Slash;
  throw Error{U"PlayerConfig::FromToml: 不明な melee trajectory \"" + value +
              U"\""};
}

}  // namespace

PlayerConfig PlayerConfig::FromToml(const TOMLValue& toml) {
  const auto& m = toml[U"melee"];
  const auto& r = toml[U"ranged"];
  const auto& d = toml[U"dash"];
  const auto& da = toml[U"dash_attack"];
  const auto& aa = toml[U"air_attack"];
  const auto& s = toml[U"stamina"];
  const auto& l = toml[U"landing"];

  Array<MeleeStageConfig> stages;
  // Why not: m[U"stage"] がテーブル配列として存在しない場合に
  // tableArrayView() を呼ぶと不正アクセスになるため、
  // 事前に isTableArray() で存在確認する。
  // キー欠落時は空の stages のままとし、旧 get<T>() のデフォルト値
  // 挙動に倣って寛容に扱う。
  if (const auto& stageValue = m[U"stage"]; stageValue.isTableArray()) {
    for (const auto& stageToml : stageValue.tableArrayView()) {
      stages.push_back(MeleeStageConfig{
          .timeline = ParseTimeline(stageToml),
          .radius = stageToml[U"radius"].get<double>(),
          .trajectory =
              ParseMeleeTrajectory(stageToml[U"trajectory"].get<String>()),
          .slashRiseHeight = stageToml[U"slash_rise_height"].get<double>(),
          .hitstopSec = stageToml[U"hitstop_sec"].get<double>(),
      });
    }
  }

  return {
      .speed = toml[U"speed"].get<double>(),
      .jumpSpeed = toml[U"jump_speed"].get<double>(),
      .gravity = toml[U"gravity"].get<double>(),
      .melee =
          {
              .capMidH = m[U"cap_mid_h"].get<double>(),
              .reach = m[U"reach"].get<double>(),
              .damage = m[U"damage"].get<int32>(),
              .stages = std::move(stages),
          },
      .ranged =
          {
              .reach = r[U"reach"].get<double>(),
              .radius = r[U"radius"].get<double>(),
              .damage = r[U"damage"].get<int32>(),
              .bulletSpeed = r[U"bullet_speed"].get<double>(),
              .spawnHeight = r[U"spawn_height"].get<double>(),
              .staminaCost = r[U"stamina_cost"].get<int32>(),
          },
      .dash =
          {
              .speed = d[U"speed"].get<double>(),
              .timeline = ParseTimeline(d),
              .staminaCost = d[U"stamina_cost"].get<int32>(),
          },
      .dashAttack =
          {
              .timeline = ParseTimeline(da),
              .speed = da[U"speed"].get<double>(),
              .orbitRadius = da[U"orbit_radius"].get<double>(),
              .radius = da[U"radius"].get<double>(),
              .damage = da[U"damage"].get<int32>(),
              .hitstopSec = da[U"hitstop_sec"].get<double>(),
          },
      .airAttack =
          {
              .timeline = ParseTimeline(aa),
              .orbitRadius = aa[U"orbit_radius"].get<double>(),
              .radius = aa[U"radius"].get<double>(),
              .damage = aa[U"damage"].get<int32>(),
              .hitstopSec = aa[U"hitstop_sec"].get<double>(),
          },
      .stamina =
          {
              .recoveryDelay = s[U"recovery_delay"].get<double>(),
              .recoveryRate = s[U"recovery_rate"].get<double>(),
          },
      .landing =
          {
              .recoverySec = l[U"recovery_sec"].get<double>(),
          },
  };
}
