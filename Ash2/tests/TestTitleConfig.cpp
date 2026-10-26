#ifdef _DEBUG
#include <ThirdParty/Catch2/catch.hpp>

#include "Config/TitleConfig.hpp"

TEST_CASE("TitleConfig::FromToml - parses all fields correctly") {
  constexpr std::string_view kToml = "logo_y = 160.0\n";
  const TOMLReader reader{MemoryViewReader{kToml.data(), kToml.size()}};
  const auto cfg = TitleConfig::FromToml(reader);
  REQUIRE(cfg.logoY == 160.0);
}

TEST_CASE("TitleConfig::FromToml - missing logo_y throws") {
  constexpr std::string_view kToml = "\n";
  const TOMLReader reader{MemoryViewReader{kToml.data(), kToml.size()}};
  REQUIRE_THROWS_AS((void)TitleConfig::FromToml(reader), Error);
}

#endif
