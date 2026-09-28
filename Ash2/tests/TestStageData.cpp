#ifdef _DEBUG
#include <ThirdParty/Catch2/catch.hpp>

#include "Phase/StageData.hpp"

namespace {
constexpr std::string_view kValidToml =
    "[[enemies]]\n"
    "w = 150.0\n"
    "d = 0.0\n"
    "[[enemies]]\n"
    "w = -150.0\n"
    "d = 80.0\n";
}  // namespace

TEST_CASE("StageData::FromToml - parses all enemies correctly") {
  const TOMLReader reader{
      MemoryViewReader{kValidToml.data(), kValidToml.size()}
  };
  const auto data = StageData::FromToml(reader);
  REQUIRE(data.has_value());
  REQUIRE(data->enemies.size() == 2);
  REQUIRE(data->enemies[0].pos.w == 150.0);
  REQUIRE(data->enemies[0].pos.d == 0.0);
  REQUIRE(data->enemies[1].pos.w == -150.0);
  REQUIRE(data->enemies[1].pos.d == 80.0);
}

TEST_CASE("StageData::FromToml - missing w in an enemy returns unexpected") {
  constexpr std::string_view kToml =
      "[[enemies]]\n"
      "d = 0.0\n";
  const TOMLReader reader{MemoryViewReader{kToml.data(), kToml.size()}};
  REQUIRE_FALSE(StageData::FromToml(reader).has_value());
}

TEST_CASE("StageData::FromToml - empty enemies returns unexpected") {
  constexpr std::string_view kToml = "enemies = []\n";
  const TOMLReader reader{MemoryViewReader{kToml.data(), kToml.size()}};
  REQUIRE_FALSE(StageData::FromToml(reader).has_value());
}

TEST_CASE("StageData::FromToml - missing enemies returns unexpected") {
  constexpr std::string_view kToml = "";
  const TOMLReader reader{MemoryViewReader{kToml.data(), kToml.size()}};
  REQUIRE_FALSE(StageData::FromToml(reader).has_value());
}

#endif
