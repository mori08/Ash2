#ifdef _DEBUG
#include <ThirdParty/Catch2/catch.hpp>

#include "Config/AnimationData.hpp"

TEST_CASE("AnimationData::FromToml - parses top-level fields correctly") {
  constexpr std::string_view kToml =
      "texture = \"assets/images/player.png\"\n"
      "width = 32\n"
      "height = 64\n"
      "\n"
      "[draw_offset]\n"
      "x = -8.0\n"
      "y = -32.0\n";
  const TOMLReader reader{MemoryViewReader{kToml.data(), kToml.size()}};
  const auto data = AnimationData::FromToml(reader);
  REQUIRE(data.has_value());
  REQUIRE(data->textureKey == U"assets/images/player.png");
  REQUIRE(data->size.x == 32);
  REQUIRE(data->size.y == 64);
  REQUIRE(data->drawOffset.x == -8.0);
  REQUIRE(data->drawOffset.y == -32.0);
}

TEST_CASE("AnimationData::FromToml - parses clip entries correctly") {
  constexpr std::string_view kToml =
      "texture = \"assets/images/enemy.png\"\n"
      "width = 48\n"
      "height = 48\n"
      "\n"
      "[draw_offset]\n"
      "x = 0.0\n"
      "y = -24.0\n"
      "\n"
      "[idle]\n"
      "row = 0\n"
      "count = 4\n"
      "speed = 8.0\n"
      "\n"
      "[walk]\n"
      "row = 1\n"
      "count = 6\n"
      "speed = 12.0\n";
  const TOMLReader reader{MemoryViewReader{kToml.data(), kToml.size()}};
  const auto data = AnimationData::FromToml(reader);
  REQUIRE(data.has_value());
  REQUIRE(data->clips.contains(U"idle"));
  REQUIRE(data->clips.at(U"idle").row == 0);
  REQUIRE(data->clips.at(U"idle").count == 4);
  REQUIRE(data->clips.at(U"idle").speed == 8.0);
  REQUIRE(data->clips.contains(U"walk"));
  REQUIRE(data->clips.at(U"walk").row == 1);
  REQUIRE(data->clips.at(U"walk").count == 6);
  REQUIRE(data->clips.at(U"walk").speed == 12.0);
}

TEST_CASE("AnimationData::FromToml - missing width returns unexpected") {
  constexpr std::string_view kToml =
      "texture = \"assets/images/player.png\"\n"
      "height = 64\n"
      "\n"
      "[draw_offset]\n"
      "x = -8.0\n"
      "y = -32.0\n";
  const TOMLReader reader{MemoryViewReader{kToml.data(), kToml.size()}};
  REQUIRE_FALSE(AnimationData::FromToml(reader).has_value());
}

TEST_CASE("AnimationData::FromToml - missing clip row returns unexpected") {
  constexpr std::string_view kToml =
      "texture = \"assets/images/player.png\"\n"
      "width = 32\n"
      "height = 64\n"
      "\n"
      "[draw_offset]\n"
      "x = -8.0\n"
      "y = -32.0\n"
      "\n"
      "[idle]\n"
      "count = 4\n"
      "speed = 8.0\n";
  const TOMLReader reader{MemoryViewReader{kToml.data(), kToml.size()}};
  REQUIRE_FALSE(AnimationData::FromToml(reader).has_value());
}

TEST_CASE("AnimationData::FromToml - loop key omitted defaults to false") {
  constexpr std::string_view kToml =
      "texture = \"assets/images/player.png\"\n"
      "width = 32\n"
      "height = 64\n"
      "\n"
      "[draw_offset]\n"
      "x = 0.0\n"
      "y = 0.0\n"
      "\n"
      "[idle]\n"
      "row = 0\n"
      "count = 4\n"
      "speed = 8.0\n";
  const TOMLReader reader{MemoryViewReader{kToml.data(), kToml.size()}};
  const auto data = AnimationData::FromToml(reader);
  REQUIRE(data.has_value());
  REQUIRE_FALSE(data->clips.at(U"idle").loop);
}

TEST_CASE("AnimationData::FromToml - loop = true is parsed") {
  constexpr std::string_view kToml =
      "texture = \"assets/images/player.png\"\n"
      "width = 32\n"
      "height = 64\n"
      "\n"
      "[draw_offset]\n"
      "x = 0.0\n"
      "y = 0.0\n"
      "\n"
      "[idle]\n"
      "row = 0\n"
      "count = 4\n"
      "speed = 8.0\n"
      "loop = true\n";
  const TOMLReader reader{MemoryViewReader{kToml.data(), kToml.size()}};
  const auto data = AnimationData::FromToml(reader);
  REQUIRE(data.has_value());
  REQUIRE(data->clips.at(U"idle").loop);
}

TEST_CASE(
    "AnimationClip::advance - loop wraps past the end back to the start"
) {
  const AnimationClip clip{.row = 0, .count = 4, .speed = 2.0, .loop = true};
  // cycleDuration() == 2.0
  const double elapsed = clip.advance(1.9, 0.2);
  REQUIRE(elapsed == Approx(0.1));
}

TEST_CASE(
    "AnimationClip::advance - non-loop stops at the end without wrapping"
) {
  const AnimationClip clip{.row = 0, .count = 4, .speed = 2.0, .loop = false};
  // cycleDuration() == 2.0。超過分を足しても cycleDuration() で止まる
  const double elapsed = clip.advance(1.9, 0.5);
  REQUIRE(elapsed == Approx(2.0));
}

TEST_CASE("AnimationClip::columnAt - clamps to [0, count - 1]") {
  const AnimationClip clip{.row = 0, .count = 4, .speed = 2.0, .loop = false};
  REQUIRE(clip.columnAt(0.0) == 0);
  // 最終コマ（列3）の開始位相
  REQUIRE(clip.columnAt(1.5) == 3);
  // cycleDuration() 到達時も最終コマに留まり、列0へは戻らない
  REQUIRE(clip.columnAt(clip.cycleDuration()) == 3);
}

TEST_CASE("AnimationData::FromToml - reserved keys are not treated as clips") {
  // texture / width / height / draw_offset はクリップとして誤認されてはならない
  constexpr std::string_view kToml =
      "texture = \"assets/images/player.png\"\n"
      "width = 32\n"
      "height = 64\n"
      "\n"
      "[draw_offset]\n"
      "x = 0.0\n"
      "y = 0.0\n";
  const TOMLReader reader{MemoryViewReader{kToml.data(), kToml.size()}};
  const auto data = AnimationData::FromToml(reader);
  REQUIRE(data.has_value());
  REQUIRE_FALSE(data->clips.contains(U"texture"));
  REQUIRE_FALSE(data->clips.contains(U"width"));
  REQUIRE_FALSE(data->clips.contains(U"height"));
  REQUIRE_FALSE(data->clips.contains(U"draw_offset"));
}

#endif
