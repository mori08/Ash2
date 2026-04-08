#include "PlayerConfig.hpp"

PlayerConfig PlayerConfig::fromTOML(const s3d::TOMLValue& toml) {
  return PlayerConfig{
      .speed = toml[U"speed"].get<double>(),
      .jumpSpeed = toml[U"jump_speed"].get<double>(),
      .gravity = toml[U"gravity"].get<double>(),
  };
}
