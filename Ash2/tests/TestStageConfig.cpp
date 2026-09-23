#ifdef _DEBUG
#include <ThirdParty/Catch2/catch.hpp>

#include "Config/StageConfig.hpp"

namespace {
constexpr std::string_view kValidToml =
    "half_w = 380.0\n"
    "half_d = 280.0\n";
}  // namespace

TEST_CASE("StageConfig::FromToml - parses all fields correctly") {
  const TOMLReader reader{
      MemoryViewReader{kValidToml.data(), kValidToml.size()}
  };
  const auto cfg = StageConfig::FromToml(reader);
  REQUIRE(cfg.has_value());
  REQUIRE(cfg->halfW == 380.0);
  REQUIRE(cfg->halfD == 280.0);
}

TEST_CASE("StageConfig::FromToml - missing half_w returns unexpected") {
  constexpr std::string_view kToml = "half_d = 280.0\n";
  const TOMLReader reader{MemoryViewReader{kToml.data(), kToml.size()}};
  REQUIRE_FALSE(StageConfig::FromToml(reader).has_value());
}

TEST_CASE("StageConfig::FromToml - non-positive half_w returns unexpected") {
  constexpr std::string_view kToml =
      "half_w = 0.0\n"
      "half_d = 280.0\n";
  const TOMLReader reader{MemoryViewReader{kToml.data(), kToml.size()}};
  REQUIRE_FALSE(StageConfig::FromToml(reader).has_value());
}

TEST_CASE("StageConfig::FromToml - non-positive half_d returns unexpected") {
  constexpr std::string_view kToml =
      "half_w = 380.0\n"
      "half_d = -1.0\n";
  const TOMLReader reader{MemoryViewReader{kToml.data(), kToml.size()}};
  REQUIRE_FALSE(StageConfig::FromToml(reader).has_value());
}

#endif
