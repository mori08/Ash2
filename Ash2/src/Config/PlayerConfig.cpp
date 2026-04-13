#include "PlayerConfig.hpp"

PlayerConfig PlayerConfig::FromToml(const s3d::TOMLValue& toml) {
  return PlayerConfig{
      .speed = toml[U"speed"].get<double>(),
      .jumpSpeed = toml[U"jump_speed"].get<double>(),
      .gravity = toml[U"gravity"].get<double>(),
      .spriteWidth = toml[U"sprite_width"].get<double>(),
      .spriteHeight = toml[U"sprite_height"].get<double>(),
      .spriteColorR = toml[U"sprite_color"][U"r"].get<double>(),
      .spriteColorG = toml[U"sprite_color"][U"g"].get<double>(),
      .spriteColorB = toml[U"sprite_color"][U"b"].get<double>(),
  };
}
