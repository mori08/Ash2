#include "PlayerConfig.hpp"

PlayerConfig PlayerConfig::FromToml(const s3d::TOMLValue& toml) {
  const auto& tex = toml[U"texture"];
  const auto& off = tex[U"draw_offset"];
  return {
      .speed = toml[U"speed"].get<double>(),
      .jumpSpeed = toml[U"jump_speed"].get<double>(),
      .gravity = toml[U"gravity"].get<double>(),
      .texture =
          {
              .frameWidth = tex[U"frame_width"].get<int>(),
              .frameHeight = tex[U"frame_height"].get<int>(),
              .frameIndex = tex[U"frame_index"].get<int>(),
              .drawOffset = {off[U"x"].get<double>(), off[U"y"].get<double>()},
          },
  };
}
