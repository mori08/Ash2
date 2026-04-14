#include "PlayerConfig.hpp"

PlayerConfig PlayerConfig::FromToml(const s3d::TOMLValue& toml) {
  return PlayerConfig{
      .speed = toml[U"speed"].get<double>(),
      .jumpSpeed = toml[U"jump_speed"].get<double>(),
      .gravity = toml[U"gravity"].get<double>(),
      .spriteSize = SizeF{toml[U"sprite_width"].get<double>(),
                          toml[U"sprite_height"].get<double>()},
      .spriteColor = ColorF{toml[U"sprite_color"][U"r"].get<double>(),
                            toml[U"sprite_color"][U"g"].get<double>(),
                            toml[U"sprite_color"][U"b"].get<double>()},
  };
}
