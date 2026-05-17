#include "Component/AnimationData.hpp"

AnimationData AnimationData::FromToml(const s3d::TOMLValue& toml) {
  const auto& off = toml[U"draw_offset"];
  AnimationData data{
      .textureKey = toml[U"texture"].getString(),
      .size = {toml[U"width"].get<int>(), toml[U"height"].get<int>()},
      .drawOffset = {off[U"x"].get<double>(), off[U"y"].get<double>()},
  };

  for (const auto& member : toml.tableView()) {
    if (member.name == U"texture" || member.name == U"width" ||
        member.name == U"height" || member.name == U"draw_offset") {
      continue;
    }
    const auto& clip = member.value;
    data.clips[member.name] = AnimationClip{
        .row = clip[U"row"].get<int>(),
        .count = clip[U"count"].get<int>(),
        .speed = clip[U"speed"].get<double>(),
    };
  }
  return data;
}
