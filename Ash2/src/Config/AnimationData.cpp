#include "Config/AnimationData.hpp"

#include "Config/TomlTable.hpp"

namespace {

/// @brief TOML から1クリップ分の AnimationClip を生成する
[[nodiscard]] AnimationClip ParseClip(const TOMLValue& clip) {
  const TomlTable t{clip};
  AnimationClip result{
      .row = t.get<int32>(U"row"),
      .count = t.get<int32>(U"count"),
      .speed = t.get<double>(U"speed"),
      .loop = t.getOr<bool>(U"loop", false),
  };
  if (result.row < 0) {
    throw Error{U"アニメーションのクリップの row が負"};
  }
  if (result.count <= 0) {
    throw Error{U"アニメーションのクリップの count が正でない"};
  }
  if (result.speed <= 0.0) {
    throw Error{U"アニメーションのクリップの speed が正でない"};
  }
  return result;
}

}  // namespace

AnimationData AnimationData::FromToml(const TOMLValue& toml) {
  const TomlTable t{toml};
  const TomlTable offset{TomlMember(toml, U"draw_offset")};
  AnimationData data{
      .textureKey = t.get<String>(U"texture"),
      .size = {t.get<int32>(U"width"), t.get<int32>(U"height")},
      .drawOffset = {offset.get<double>(U"x"), offset.get<double>(U"y")},
  };

  if (data.size.x <= 0 || data.size.y <= 0) {
    throw Error{U"アニメーションの width または height が正でない"};
  }

  for (const auto& member : toml.tableView()) {
    if (member.name == U"texture" || member.name == U"width" ||
        member.name == U"height" || member.name == U"draw_offset") {
      continue;
    }
    data.clips[member.name] = ParseClip(member.value);
  }
  if (data.clips.empty()) {
    throw Error{U"アニメーションのクリップが1件もない"};
  }
  return data;
}
