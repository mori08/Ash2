#ifdef _DEBUG
#include <ThirdParty/Catch2/catch.hpp>

#include "Config/ArenaConfig.hpp"

namespace {
constexpr std::string_view kValidToml =
    "half_w = 380.0\n"
    "half_d = 280.0\n";
}  // namespace

TEST_CASE("ArenaConfig::FromToml - parses all fields correctly") {
  const TOMLReader reader{
      MemoryViewReader{kValidToml.data(), kValidToml.size()}
  };
  const auto cfg = ArenaConfig::FromToml(reader);
  REQUIRE(cfg.has_value());
  REQUIRE(cfg->halfW == 380.0);
  REQUIRE(cfg->halfD == 280.0);
}

TEST_CASE("ArenaConfig::FromToml - missing half_w returns unexpected") {
  constexpr std::string_view kToml = "half_d = 280.0\n";
  const TOMLReader reader{MemoryViewReader{kToml.data(), kToml.size()}};
  REQUIRE_FALSE(ArenaConfig::FromToml(reader).has_value());
}

TEST_CASE("ArenaConfig::FromToml - non-positive half_w returns unexpected") {
  constexpr std::string_view kToml =
      "half_w = 0.0\n"
      "half_d = 280.0\n";
  const TOMLReader reader{MemoryViewReader{kToml.data(), kToml.size()}};
  REQUIRE_FALSE(ArenaConfig::FromToml(reader).has_value());
}

TEST_CASE("ArenaConfig::FromToml - non-positive half_d returns unexpected") {
  constexpr std::string_view kToml =
      "half_w = 380.0\n"
      "half_d = -1.0\n";
  const TOMLReader reader{MemoryViewReader{kToml.data(), kToml.size()}};
  REQUIRE_FALSE(ArenaConfig::FromToml(reader).has_value());
}

#endif
