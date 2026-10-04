#ifdef _DEBUG
#include <ThirdParty/Catch2/catch.hpp>

#include "Factory/StageData.hpp"

TEST_CASE("StageData::FromToml - parses stages and enemy positions") {
  constexpr std::string_view kToml =
      "[[stage1.enemies]]\n"
      "w = 200.0\n"
      "d = 80.0\n"
      "[[stage1.enemies]]\n"
      "w = -220.0\n"
      "d = -60.0\n"
      "[[stage2.enemies]]\n"
      "w = 0.0\n"
      "d = 10.0\n";
  const TOMLReader reader{MemoryViewReader{kToml.data(), kToml.size()}};
  const auto data = StageData::FromToml(reader);
  REQUIRE(data.has_value());
  REQUIRE(data->stages.size() == 2);

  const auto& enemies = data->stages.at(U"stage1").enemies;
  REQUIRE(enemies.size() == 2);
  REQUIRE(enemies[0].pos.w == 200.0);
  REQUIRE(enemies[0].pos.d == 80.0);
  REQUIRE(enemies[0].pos.h == 0.0);
  REQUIRE(enemies[1].pos.w == -220.0);
  REQUIRE(enemies[1].pos.d == -60.0);
  REQUIRE(data->stages.at(U"stage2").enemies.size() == 1);
}

TEST_CASE("StageData::FromToml - missing w in enemy returns unexpected") {
  constexpr std::string_view kToml =
      "[[stage1.enemies]]\n"
      "d = 80.0\n";
  const TOMLReader reader{MemoryViewReader{kToml.data(), kToml.size()}};
  REQUIRE_FALSE(StageData::FromToml(reader).has_value());
}

TEST_CASE("StageData::FromToml - missing enemies returns unexpected") {
  constexpr std::string_view kToml =
      "[stage1]\n"
      "name = \"x\"\n";
  const TOMLReader reader{MemoryViewReader{kToml.data(), kToml.size()}};
  REQUIRE_FALSE(StageData::FromToml(reader).has_value());
}

TEST_CASE("StageData::FromToml - empty enemies returns unexpected") {
  constexpr std::string_view kToml = "[stage1]\nenemies = []\n";
  const TOMLReader reader{MemoryViewReader{kToml.data(), kToml.size()}};
  REQUIRE_FALSE(StageData::FromToml(reader).has_value());
}

#endif
