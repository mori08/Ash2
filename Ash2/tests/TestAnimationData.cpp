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
