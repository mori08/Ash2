#include "Config/AnimationData.hpp"

#include "Config/TomlTable.hpp"

namespace {

/// @brief TOML から1クリップ分の AnimationClip を生成する
[[nodiscard]] AnimationClip ParseClip(const TOMLValue& clip) {
  const TomlTable t{clip};
  return AnimationClip{
      .row = t.get<int32>(U"row"),
      .count = t.get<int32>(U"count"),
      .speed = t.get<double>(U"speed"),
      .loop = t.getOr<bool>(U"loop", false),
  };
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

  for (const auto& member : toml.tableView()) {
    if (member.name == U"texture" || member.name == U"width" ||
        member.name == U"height" || member.name == U"draw_offset") {
      continue;
    }
    data.clips[member.name] = ParseClip(member.value);
  }
  return data;
}
