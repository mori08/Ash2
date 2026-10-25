#include "PlayerConfig.hpp"

#include "Config/TomlTable.hpp"

namespace {

/// @brief TOML 値から MotionTimeline を生成する
/// @note windup_sec/active_sec/recovery_a_sec/recovery_b_sec
/// の4キーを持つテーブルであればよく、melee の段テーブルのように
/// 他のキーを併せ持っていても構わない
[[nodiscard]] MotionTimeline ParseTimeline(const TOMLValue& toml) {
  const TomlTable t{toml};
  return MotionTimeline{
      .windupSec = t.get<double>(U"windup_sec"),
      .activeSec = t.get<double>(U"active_sec"),
      .recoveryASec = t.get<double>(U"recovery_a_sec"),
      .recoveryBSec = t.get<double>(U"recovery_b_sec"),
  };
}

/// @brief TOML の trajectory 文字列を MeleeTrajectory へ変換する
[[nodiscard]] MeleeTrajectory ParseMeleeTrajectory(const String& value) {
  if (value == U"thrust") return MeleeTrajectory::Thrust;
  if (value == U"slash") return MeleeTrajectory::Slash;
  throw Error{U"melee の trajectory が不明な値"};
}

/// @brief TOML から近接1振り分の MeleeSwingConfig を生成する
[[nodiscard]] MeleeSwingConfig ParseMeleeSwing(const TOMLValue& swingToml) {
  const TomlTable t{swingToml};
  return MeleeSwingConfig{
      .timeline = ParseTimeline(swingToml),
      .radius = t.get<double>(U"radius"),
      .trajectory = ParseMeleeTrajectory(t.get<String>(U"trajectory")),
      .slashRiseHeight = t.get<double>(U"slash_rise_height"),
      .slashCurve = t.get<double>(U"slash_curve"),
      .hitstopSec = t.get<double>(U"hitstop_sec"),
      .reaction = ParseReactionLevel(t.get<String>(U"reaction")),
      .staminaCost = t.get<int32>(U"stamina_cost"),
  };
}

/// @brief TOML から締め段の MeleeFinisherConfig を生成する
[[nodiscard]] MeleeFinisherConfig ParseMeleeFinisher(
    const TOMLValue& finisherToml
) {
  const TomlTable t{finisherToml};
  MeleeFinisherConfig finisher{
      .swing = ParseMeleeSwing(finisherToml),
      .lightCount = t.get<int32>(U"light_count"),
      .lightGap = t.get<double>(U"light_gap"),
  };

  // parse 時に不変条件（光は2つ以上）を確定させ、MeleeLightSpread
  // 側では throw のみで済むようにする
  if (finisher.lightCount < 2) {
    throw Error{U"締め段の光が2つ未満"};
  }

  return finisher;
}

/// @brief TOML から近接攻撃の設定値を生成する
[[nodiscard]] MeleeConfig ParseMelee(const TOMLValue& m) {
  Array<MeleeSwingConfig> chain;
  // Why not: TomlMember(m, U"chain") がテーブル配列として存在しない場合に
  // tableArrayView() を呼ぶと不正アクセスになるため、
  // 事前に isTableArray() で存在確認する。
  if (const auto& chainValue = TomlMember(m, U"chain");
      chainValue.isTableArray()) {
    for (const auto& chainToml : chainValue.tableArrayView()) {
      chain.push_back(ParseMeleeSwing(chainToml));
    }
  }
  // chain が空だと Tick(MeleeChain&, ...) の chain[state.stage] アクセスが
  // 不正になるため許容しない
  if (chain.isEmpty()) {
    throw Error{U"melee.chain が空"};
  }

  const TomlTable t{m};
  return MeleeConfig{
      .capMidH = t.get<double>(U"cap_mid_h"),
      .reach = t.get<double>(U"reach"),
      .damage = t.get<int32>(U"damage"),
      .chain = std::move(chain),
      .finisher = ParseMeleeFinisher(TomlMember(m, U"finisher")),
  };
}

/// @brief TOML から遠距離攻撃の設定値を生成する
[[nodiscard]] RangedConfig ParseRanged(const TOMLValue& r) {
  const TomlTable t{r};
  return RangedConfig{
      .reach = t.get<double>(U"reach"),
      .radius = t.get<double>(U"radius"),
      .damage = t.get<int32>(U"damage"),
      .bulletSpeed = t.get<double>(U"bullet_speed"),
      .spawnHeight = t.get<double>(U"spawn_height"),
      .staminaCost = t.get<int32>(U"stamina_cost"),
      .recoverySec = t.get<double>(U"recovery_sec"),
  };
}

/// @brief TOML からダッシュの設定値を生成する
[[nodiscard]] DashConfig ParseDash(const TOMLValue& d) {
  const TomlTable t{d};
  return DashConfig{
      .speed = t.get<double>(U"speed"),
      .timeline = ParseTimeline(d),
      .staminaCost = t.get<int32>(U"stamina_cost"),
  };
}

/// @brief TOML からダッシュ攻撃の設定値を生成する
[[nodiscard]] DashAttackConfig ParseDashAttack(const TOMLValue& da) {
  const TomlTable t{da};
  return DashAttackConfig{
      .timeline = ParseTimeline(da),
      .speed = t.get<double>(U"speed"),
      .orbitRadius = t.get<double>(U"orbit_radius"),
      .radius = t.get<double>(U"radius"),
      .damage = t.get<int32>(U"damage"),
      .hitstopSec = t.get<double>(U"hitstop_sec"),
      .reaction = ParseReactionLevel(t.get<String>(U"reaction")),
      .staminaCost = t.get<int32>(U"stamina_cost"),
  };
}

/// @brief TOML から空中攻撃の設定値を生成する
[[nodiscard]] AirAttackConfig ParseAirAttack(const TOMLValue& aa) {
  const TomlTable t{aa};
  return AirAttackConfig{
      .timeline = ParseTimeline(aa),
      .driftRatio = t.get<double>(U"drift_ratio"),
      .orbitRadius = t.get<double>(U"orbit_radius"),
      .orbitStartDeg = t.get<double>(U"orbit_start_deg"),
      .orbitEndDeg = t.get<double>(U"orbit_end_deg"),
      .radius = t.get<double>(U"radius"),
      .damage = t.get<int32>(U"damage"),
      .hitstopSec = t.get<double>(U"hitstop_sec"),
      .reaction = ParseReactionLevel(t.get<String>(U"reaction")),
      .staminaCost = t.get<int32>(U"stamina_cost"),
  };
}

/// @brief TOML からスタミナ回復の設定値を生成する
[[nodiscard]] StaminaConfig ParseStamina(const TOMLValue& s) {
  const TomlTable t{s};
  return StaminaConfig{
      .max = t.get<int32>(U"max"),
      .recoveryDelay = t.get<double>(U"recovery_delay"),
      .recoveryRate = t.get<double>(U"recovery_rate"),
  };
}

/// @brief TOML から着地硬直の設定値を生成する
[[nodiscard]] LandingConfig ParseLanding(const TOMLValue& l) {
  const TomlTable t{l};
  return LandingConfig{
      .recoverySec = t.get<double>(U"recovery_sec"),
  };
}

/// @brief TOML から攻撃演出共通の設定値を生成する
[[nodiscard]] AttackEffectConfig ParseAttackEffect(const TOMLValue& ae) {
  const TomlTable t{ae};
  return AttackEffectConfig{
      .fadeSec = t.get<double>(U"fade_sec"),
  };
}

/// @brief TOML から被弾リアクションの設定値を生成する
[[nodiscard]] DamageConfig ParseDamage(const TOMLValue& d) {
  const TomlTable t{d};
  return DamageConfig{
      .staggerSec = t.get<double>(U"stagger_sec"),
      .knockbackSpeedW = t.get<double>(U"knockback_speed_w"),
      .knockbackSpeedH = t.get<double>(U"knockback_speed_h"),
      .downSec = t.get<double>(U"down_sec"),
      .getUpSec = t.get<double>(U"get_up_sec"),
  };
}

/// @brief TOML からロック（遠距離照準）の設定値を生成する
[[nodiscard]] LockConfig ParseLock(const TOMLValue& l) {
  const TomlTable t{l};
  return LockConfig{
      .capsuleScale = t.get<double>(U"capsule_scale"),
      .angleLimitDeg = t.get<double>(U"angle_limit_deg"),
      .angleWeight = t.get<double>(U"angle_weight"),
      .stickTilt = t.get<double>(U"stick_tilt"),
      .stickRelease = t.get<double>(U"stick_release"),
      .lockedColor = t.get<ColorF>(U"locked_color"),
      .lockedAlpha = t.get<double>(U"locked_alpha"),
      .warningAlpha = t.get<double>(U"warning_alpha"),
      .halfColor = t.get<ColorF>(U"half_color"),
      .halfAlpha = t.get<double>(U"half_alpha"),
  };
}

}  // namespace

PlayerConfig PlayerConfig::FromToml(const TOMLValue& toml) {
  const TomlTable t{toml};
  return PlayerConfig{
      .speed = t.get<double>(U"speed"),
      .jumpSpeed = t.get<double>(U"jump_speed"),
      .gravity = t.get<double>(U"gravity"),
      .capsuleRadius = t.get<double>(U"capsule_radius"),
      .capsuleHeight = t.get<double>(U"capsule_height"),
      .melee = ParseMelee(TomlMember(toml, U"melee")),
      .ranged = ParseRanged(TomlMember(toml, U"ranged")),
      .dash = ParseDash(TomlMember(toml, U"dash")),
      .dashAttack = ParseDashAttack(TomlMember(toml, U"dash_attack")),
      .airAttack = ParseAirAttack(TomlMember(toml, U"air_attack")),
      .stamina = ParseStamina(TomlMember(toml, U"stamina")),
      .landing = ParseLanding(TomlMember(toml, U"landing")),
      .attackEffect = ParseAttackEffect(TomlMember(toml, U"attack_effect")),
      .damage = ParseDamage(TomlMember(toml, U"damage")),
      .lock = ParseLock(TomlMember(toml, U"lock")),
  };
}
