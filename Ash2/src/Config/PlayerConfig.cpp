#include "PlayerConfig.hpp"

#include "Config/TomlFields.hpp"

namespace {

/// @brief TOML 値から MotionTimeline を生成する
/// @param section 欠落キーのメッセージに前置するテーブル名
/// @note windup_sec/active_sec/recovery_a_sec/recovery_b_sec
/// の4キーを持つテーブルであればよく、melee の段テーブルのように
/// 他のキーを併せ持っていても構わない
[[nodiscard]] std::expected<MotionTimeline, String> ParseTimeline(
    const TOMLValue& toml, StringView section
) {
  TomlFields f{toml, U"PlayerConfig::ParseTimeline", String{section}};
  return f.wrap(
      MotionTimeline{
          .windupSec = f.get<double>(U"windup_sec"),
          .activeSec = f.get<double>(U"active_sec"),
          .recoveryASec = f.get<double>(U"recovery_a_sec"),
          .recoveryBSec = f.get<double>(U"recovery_b_sec"),
      }
  );
}

/// @brief TOML の trajectory 文字列を MeleeTrajectory へ変換する
[[nodiscard]] std::expected<MeleeTrajectory, String> ParseMeleeTrajectory(
    const String& value
) {
  if (value == U"thrust") return MeleeTrajectory::Thrust;
  if (value == U"slash") return MeleeTrajectory::Slash;
  return std::unexpected{
      U"PlayerConfig::ParseMeleeTrajectory: 不明な melee "
      U"trajectory \"" +
      value + U"\""
  };
}

/// @brief TOML から近接1振り分の MeleeSwingConfig を生成する
/// @param section 欠落キーのメッセージに前置するテーブル名
[[nodiscard]] std::expected<MeleeSwingConfig, String> ParseMeleeSwing(
    const TOMLValue& swingToml, StringView section
) {
  auto timeline = ParseTimeline(swingToml, section);
  if (!timeline) {
    return std::unexpected{std::move(timeline).error()};
  }

  TomlFields f{swingToml, U"PlayerConfig::ParseMeleeSwing", String{section}};
  const auto radius = f.get<double>(U"radius");
  const auto trajectoryStr = f.get<String>(U"trajectory");
  const auto slashRiseHeight = f.get<double>(U"slash_rise_height");
  const auto slashCurve = f.get<double>(U"slash_curve");
  const auto hitstopSec = f.get<double>(U"hitstop_sec");
  // Why not: trajectory の変換前に check() で欠落を確定させる。変換を先に
  // 行うと、欠落時の既定値 String{}（空文字列）が「不明な trajectory」と
  // 誤報されてしまうため。
  if (auto result = f.check(); !result) {
    return std::unexpected{std::move(result).error()};
  }

  auto trajectory = ParseMeleeTrajectory(trajectoryStr);
  if (!trajectory) {
    return std::unexpected{std::move(trajectory).error()};
  }

  return MeleeSwingConfig{
      .timeline = *std::move(timeline),
      .radius = radius,
      .trajectory = *trajectory,
      .slashRiseHeight = slashRiseHeight,
      .slashCurve = slashCurve,
      .hitstopSec = hitstopSec,
  };
}

/// @brief TOML から締め段の MeleeFinisherConfig を生成する
[[nodiscard]] std::expected<MeleeFinisherConfig, String> ParseMeleeFinisher(
    const TOMLValue& finisherToml
) {
  auto swing = ParseMeleeSwing(finisherToml, U"melee.finisher");
  if (!swing) {
    return std::unexpected{std::move(swing).error()};
  }

  TomlFields f{
      finisherToml, U"PlayerConfig::ParseMeleeFinisher", U"melee.finisher"
  };
  auto finisher = f.wrap(
      MeleeFinisherConfig{
          .swing = *std::move(swing),
          .lightCount = f.get<int32>(U"light_count"),
          .lightGap = f.get<double>(U"light_gap"),
      }
  );
  if (!finisher) {
    return std::unexpected{std::move(finisher).error()};
  }

  // parse 時に不変条件（光は2つ以上）を確定させ、MeleeLightSpread
  // 側では assert のみで済むようにする
  if (finisher->lightCount < 2) {
    return std::unexpected{
        U"PlayerConfig::ParseMeleeFinisher: melee.finisher.light_count は "
        U"2以上でなければなりません"
    };
  }

  return finisher;
}

/// @brief TOML から近接攻撃の設定値を生成する
[[nodiscard]] std::expected<MeleeConfig, String> ParseMelee(
    const TOMLValue& m
) {
  Array<MeleeSwingConfig> chain;
  // Why not: m[U"chain"] がテーブル配列として存在しない場合に
  // tableArrayView() を呼ぶと不正アクセスになるため、
  // 事前に isTableArray() で存在確認する。
  if (const auto& chainValue = m[U"chain"]; chainValue.isTableArray()) {
    size_t index = 0;
    for (const auto& chainToml : chainValue.tableArrayView()) {
      auto swing =
          ParseMeleeSwing(chainToml, U"melee.chain[" + Format(index) + U"]");
      if (!swing) {
        return std::unexpected{std::move(swing).error()};
      }
      chain.push_back(*std::move(swing));
      ++index;
    }
  }
  // chain が空だと Tick(MeleeChain&, ...) の chain[state.stage] アクセスが
  // 不正になるため許容しない
  if (chain.isEmpty()) {
    return std::unexpected{
        U"PlayerConfig::ParseMelee: melee.chain がありません"
    };
  }

  auto finisher = ParseMeleeFinisher(m[U"finisher"]);
  if (!finisher) {
    return std::unexpected{std::move(finisher).error()};
  }

  TomlFields f{m, U"PlayerConfig::ParseMelee", U"melee"};
  return f.wrap(
      MeleeConfig{
          .capMidH = f.get<double>(U"cap_mid_h"),
          .reach = f.get<double>(U"reach"),
          .damage = f.get<int32>(U"damage"),
          .chain = std::move(chain),
          .finisher = *std::move(finisher),
      }
  );
}

/// @brief TOML から遠距離攻撃の設定値を生成する
[[nodiscard]] std::expected<RangedConfig, String> ParseRanged(
    const TOMLValue& r
) {
  TomlFields f{r, U"PlayerConfig::ParseRanged", U"ranged"};
  return f.wrap(
      RangedConfig{
          .reach = f.get<double>(U"reach"),
          .radius = f.get<double>(U"radius"),
          .damage = f.get<int32>(U"damage"),
          .bulletSpeed = f.get<double>(U"bullet_speed"),
          .spawnHeight = f.get<double>(U"spawn_height"),
          .staminaCost = f.get<int32>(U"stamina_cost"),
      }
  );
}

/// @brief TOML からダッシュの設定値を生成する
[[nodiscard]] std::expected<DashConfig, String> ParseDash(const TOMLValue& d) {
  auto timeline = ParseTimeline(d, U"dash");
  if (!timeline) {
    return std::unexpected{std::move(timeline).error()};
  }

  TomlFields f{d, U"PlayerConfig::ParseDash", U"dash"};
  return f.wrap(
      DashConfig{
          .speed = f.get<double>(U"speed"),
          .timeline = *std::move(timeline),
          .staminaCost = f.get<int32>(U"stamina_cost"),
      }
  );
}

/// @brief TOML からダッシュ攻撃の設定値を生成する
[[nodiscard]] std::expected<DashAttackConfig, String> ParseDashAttack(
    const TOMLValue& da
) {
  auto timeline = ParseTimeline(da, U"dash_attack");
  if (!timeline) {
    return std::unexpected{std::move(timeline).error()};
  }

  TomlFields f{da, U"PlayerConfig::ParseDashAttack", U"dash_attack"};
  return f.wrap(
      DashAttackConfig{
          .timeline = *std::move(timeline),
          .speed = f.get<double>(U"speed"),
          .orbitRadius = f.get<double>(U"orbit_radius"),
          .radius = f.get<double>(U"radius"),
          .damage = f.get<int32>(U"damage"),
          .hitstopSec = f.get<double>(U"hitstop_sec"),
      }
  );
}

/// @brief TOML から空中攻撃の設定値を生成する
[[nodiscard]] std::expected<AirAttackConfig, String> ParseAirAttack(
    const TOMLValue& aa
) {
  auto timeline = ParseTimeline(aa, U"air_attack");
  if (!timeline) {
    return std::unexpected{std::move(timeline).error()};
  }

  TomlFields f{aa, U"PlayerConfig::ParseAirAttack", U"air_attack"};
  return f.wrap(
      AirAttackConfig{
          .timeline = *std::move(timeline),
          .driftRatio = f.get<double>(U"drift_ratio"),
          .orbitRadius = f.get<double>(U"orbit_radius"),
          .orbitStartDeg = f.get<double>(U"orbit_start_deg"),
          .orbitEndDeg = f.get<double>(U"orbit_end_deg"),
          .radius = f.get<double>(U"radius"),
          .damage = f.get<int32>(U"damage"),
          .hitstopSec = f.get<double>(U"hitstop_sec"),
      }
  );
}

/// @brief TOML からスタミナ回復の設定値を生成する
[[nodiscard]] std::expected<StaminaConfig, String> ParseStamina(
    const TOMLValue& s
) {
  TomlFields f{s, U"PlayerConfig::ParseStamina", U"stamina"};
  return f.wrap(
      StaminaConfig{
          .recoveryDelay = f.get<double>(U"recovery_delay"),
          .recoveryRate = f.get<double>(U"recovery_rate"),
      }
  );
}

/// @brief TOML から着地硬直の設定値を生成する
[[nodiscard]] std::expected<LandingConfig, String> ParseLanding(
    const TOMLValue& l
) {
  TomlFields f{l, U"PlayerConfig::ParseLanding", U"landing"};
  return f.wrap(
      LandingConfig{
          .recoverySec = f.get<double>(U"recovery_sec"),
      }
  );
}

/// @brief TOML から攻撃演出共通の設定値を生成する
[[nodiscard]] std::expected<AttackEffectConfig, String> ParseAttackEffect(
    const TOMLValue& ae
) {
  TomlFields f{ae, U"PlayerConfig::ParseAttackEffect", U"attack_effect"};
  return f.wrap(
      AttackEffectConfig{
          .fadeSec = f.get<double>(U"fade_sec"),
      }
  );
}

}  // namespace

std::expected<PlayerConfig, String> PlayerConfig::FromToml(
    const TOMLValue& toml
) {
  auto melee = ParseMelee(toml[U"melee"]);
  if (!melee) {
    return std::unexpected{std::move(melee).error()};
  }
  auto ranged = ParseRanged(toml[U"ranged"]);
  if (!ranged) {
    return std::unexpected{std::move(ranged).error()};
  }
  auto dash = ParseDash(toml[U"dash"]);
  if (!dash) {
    return std::unexpected{std::move(dash).error()};
  }
  auto dashAttack = ParseDashAttack(toml[U"dash_attack"]);
  if (!dashAttack) {
    return std::unexpected{std::move(dashAttack).error()};
  }
  auto airAttack = ParseAirAttack(toml[U"air_attack"]);
  if (!airAttack) {
    return std::unexpected{std::move(airAttack).error()};
  }
  auto stamina = ParseStamina(toml[U"stamina"]);
  if (!stamina) {
    return std::unexpected{std::move(stamina).error()};
  }
  auto landing = ParseLanding(toml[U"landing"]);
  if (!landing) {
    return std::unexpected{std::move(landing).error()};
  }
  auto attackEffect = ParseAttackEffect(toml[U"attack_effect"]);
  if (!attackEffect) {
    return std::unexpected{std::move(attackEffect).error()};
  }

  TomlFields f{toml, U"PlayerConfig::FromToml"};
  return f.wrap(
      PlayerConfig{
          .speed = f.get<double>(U"speed"),
          .jumpSpeed = f.get<double>(U"jump_speed"),
          .gravity = f.get<double>(U"gravity"),
          .melee = *std::move(melee),
          .ranged = *std::move(ranged),
          .dash = *std::move(dash),
          .dashAttack = *std::move(dashAttack),
          .airAttack = *std::move(airAttack),
          .stamina = *std::move(stamina),
          .landing = *std::move(landing),
          .attackEffect = *std::move(attackEffect),
      }
  );
}
