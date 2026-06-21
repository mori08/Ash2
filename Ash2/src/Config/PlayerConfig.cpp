#include "PlayerConfig.hpp"

PlayerConfig PlayerConfig::FromToml(const s3d::TOMLValue& toml) {
  const auto& m = toml[U"melee"];
  const auto& r = toml[U"ranged"];
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
          },
      .ranged =
          {
              .reach = r[U"reach"].get<double>(),
              .radius = r[U"radius"].get<double>(),
              .damage = r[U"damage"].get<int>(),
              .bulletSpeed = r[U"bullet_speed"].get<double>(),
              .spawnHeight = r[U"spawn_height"].get<double>(),
          },
  };
}
