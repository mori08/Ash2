#include "Config/AnimationData.hpp"

#include "Config/TomlFields.hpp"

namespace {

/// @brief TOML から1クリップ分の AnimationClip を生成する
/// @param name クリップ名（欠落キーのメッセージにテーブル名として前置する）
[[nodiscard]] std::expected<AnimationClip, String> ParseClip(
    const TOMLValue& clip, StringView name) {
  TomlFields f{clip, U"AnimationData::ParseClip", String{name}};
  return f.wrap(AnimationClip{
      .row = f.get<int32>(U"row"),
      .count = f.get<int32>(U"count"),
      .speed = f.get<double>(U"speed"),
  });
}

/// @brief TOML からアニメーション共有データを生成する
[[nodiscard]] std::expected<AnimationData, String> Parse(
    const TOMLValue& toml) {
  TomlFields f{toml, U"AnimationData::Parse"};
  const auto textureKey = f.get<String>(U"texture");
  const Size size = {f.get<int32>(U"width"), f.get<int32>(U"height")};

  // draw_offset は別テーブルのため、prefix 付きの別インスタンスで読む。
  // offset.check() が先に欠落を報告し、f 側の欠落チェックは後続の
  // f.wrap() 呼び出しで別途行われる。
  TomlFields offset{toml[U"draw_offset"], U"AnimationData::Parse",
                    U"draw_offset"};
  const Vec2 drawOffset = {offset.get<double>(U"x"), offset.get<double>(U"y")};
  if (auto result = offset.check(); !result) {
    return std::unexpected{std::move(result).error()};
  }

  auto wrapped = f.wrap(AnimationData{
      .textureKey = textureKey,
      .size = size,
      .drawOffset = drawOffset,
  });
  if (!wrapped) {
    return std::unexpected{std::move(wrapped).error()};
  }
  AnimationData data = *std::move(wrapped);

  for (const auto& member : toml.tableView()) {
    if (member.name == U"texture" || member.name == U"width" ||
        member.name == U"height" || member.name == U"draw_offset") {
      continue;
    }
    auto clip = ParseClip(member.value, member.name);
    if (!clip) {
      return std::unexpected{std::move(clip).error()};
    }
    data.clips[member.name] = *std::move(clip);
  }
  return data;
}

}  // namespace

AnimationData AnimationData::FromToml(const TOMLValue& toml) {
  auto parsed = Parse(toml);
  if (!parsed) {
    // Why not: Parse からの失敗は expected で伝播してくるが、ここは設定
    // 読み込みの最上位境界であり、呼び出し元の GameSetup は expected を
    // 扱わない設計のため throw で致命として確定させる。
    throw Error{std::move(parsed).error()};
  }
  return *std::move(parsed);
}
