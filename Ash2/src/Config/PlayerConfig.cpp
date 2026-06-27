#include "PlayerConfig.hpp"

PlayerConfig PlayerConfig::FromToml(const s3d::TOMLValue& toml) {
  const auto& m = toml[U"melee"];
  const auto& r = toml[U"ranged"];
  const auto& d = toml[U"dash"];
  const auto& s = toml[U"stamina"];
  return {
      .speed = toml[U"speed"].get<double>(),
      .jumpSpeed = toml[U"jump_speed"].get<double>(),
      .gravity = toml[U"gravity"].get<double>(),
      .melee =
          {
              .capMidH = m[U"cap_mid_h"].get<double>(),
              .reach = m[U"reach"].get<double>(),
              .radius = m[U"radius"].get<double>(),
              .damage = m[U"damage"].get<int>(),
              .windupSec = m[U"windup_sec"].get<double>(),
              .activeSec = m[U"active_sec"].get<double>(),
              .recoverySec = m[U"recovery_sec"].get<double>(),
              .active2Sec = m[U"active2_sec"].get<double>(),
              .slashRiseHeight = m[U"slash_rise_height"].get<double>(),
              .windup3Sec = m[U"windup3_sec"].get<double>(),
              .active3Sec = m[U"active3_sec"].get<double>(),
              .recovery3Sec = m[U"recovery3_sec"].get<double>(),
              .radius3 = m[U"radius3"].get<double>(),
          },
      .ranged =
          {
              .reach = r[U"reach"].get<double>(),
              .radius = r[U"radius"].get<double>(),
              .damage = r[U"damage"].get<int>(),
              .bulletSpeed = r[U"bullet_speed"].get<double>(),
              .spawnHeight = r[U"spawn_height"].get<double>(),
              .staminaCost = r[U"stamina_cost"].get<int>(),
          },
      .dash =
          {
              .speed = d[U"speed"].get<double>(),
              .windupSec = d[U"windup_sec"].get<double>(),
              .dashSec = d[U"dash_sec"].get<double>(),
              .recoveryASec = d[U"recovery_a_sec"].get<double>(),
              .recoveryBSec = d[U"recovery_b_sec"].get<double>(),
              .staminaCost = d[U"stamina_cost"].get<int>(),
          },
      .stamina =
          {
              .recoveryDelay = s[U"recovery_delay"].get<double>(),
              .recoveryRate = s[U"recovery_rate"].get<double>(),
          },
  };
}
