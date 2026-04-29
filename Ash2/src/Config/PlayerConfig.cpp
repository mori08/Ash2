#include "PlayerConfig.hpp"

namespace {
WorldPos ParseOffset(const s3d::TOMLValue& v) {
  return {.w = v[U"w"].get<double>(),
          .h = v[U"h"].get<double>(),
          .d = v[U"d"].get<double>()};
}

s3d::ColorF ParseColor(const s3d::TOMLValue& v) {
  return {v[U"r"].get<double>(), v[U"g"].get<double>(), v[U"b"].get<double>()};
}

s3d::Optional<BorderStyle> ParseBorder(const s3d::TOMLValue& v) {
  const auto& border = v[U"border"];
  if (border.getType() != s3d::TOMLValueType::Table) {
    return s3d::none;
  }
  return BorderStyle{.color = ParseColor(border[U"color"]),
                     .thickness = border[U"thickness"].get<double>()};
}

PlayerPiePartConfig ParsePiePart(const s3d::TOMLValue& v) {
  return {.offset = ParseOffset(v[U"offset"]),
          .radius = v[U"radius"].get<double>(),
          .startAngle = Math::ToRadians(v[U"start_angle"].get<double>()),
          .angle = Math::ToRadians(v[U"angle"].get<double>()),
          .color = ParseColor(v[U"color"]),
          .border = ParseBorder(v)};
}

PlayerCirclePartConfig ParseCirclePart(const s3d::TOMLValue& v) {
  return {.offset = ParseOffset(v[U"offset"]),
          .radius = v[U"radius"].get<double>(),
          .color = ParseColor(v[U"color"]),
          .border = ParseBorder(v)};
}
}  // namespace

PlayerConfig PlayerConfig::FromToml(const s3d::TOMLValue& toml) {
  return {.speed = toml[U"speed"].get<double>(),
          .jumpSpeed = toml[U"jump_speed"].get<double>(),
          .gravity = toml[U"gravity"].get<double>(),
          .body = ParsePiePart(toml[U"body"]),
          .head = ParseCirclePart(toml[U"head"]),
          .handFront = ParseCirclePart(toml[U"hand_front"]),
          .handBack = ParseCirclePart(toml[U"hand_back"]),
          .footLeft = ParseCirclePart(toml[U"foot_left"]),
          .footRight = ParseCirclePart(toml[U"foot_right"])};
}
