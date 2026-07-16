#include "Config/AnimationData.hpp"

AnimationData AnimationData::FromToml(const TOMLValue& toml) {
  const auto& off = toml[U"draw_offset"];
  AnimationData data{
      .textureKey = toml[U"texture"].getString(),
      .size = {toml[U"width"].get<int32>(), toml[U"height"].get<int32>()},
      .drawOffset = {off[U"x"].get<double>(), off[U"y"].get<double>()},
  };

  for (const auto& member : toml.tableView()) {
    if (member.name == U"texture" || member.name == U"width" ||
        member.name == U"height" || member.name == U"draw_offset") {
      continue;
    }
    const auto& clip = member.value;
    data.clips[member.name] = AnimationClip{
        .row = clip[U"row"].get<int32>(),
        .count = clip[U"count"].get<int32>(),
        .speed = clip[U"speed"].get<double>(),
    };
  }
  return data;
}
