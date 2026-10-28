#ifdef _DEBUG
#include <ThirdParty/Catch2/catch.hpp>
#include <string>

#include "Config/TomlTable.hpp"

namespace {

constexpr std::string_view kToml =
    "speed = 150.0\n"
    "name = \"abc\"\n";

}  // namespace

TEST_CASE("TomlTable::get - returns the value of an existing key") {
  const TOMLReader reader{MemoryViewReader{kToml.data(), kToml.size()}};
  const TomlTable t{reader};
  REQUIRE(t.get<double>(U"speed") == 150.0);
}

TEST_CASE("TomlTable::get - throws when the key is missing") {
  const TOMLReader reader{MemoryViewReader{kToml.data(), kToml.size()}};
  const TomlTable t{reader};
  REQUIRE_THROWS_AS(t.get<double>(U"missing"), Error);
}

TEST_CASE("TomlTable::get - throws when the type differs") {
  const TOMLReader reader{MemoryViewReader{kToml.data(), kToml.size()}};
  const TomlTable t{reader};
  REQUIRE_THROWS_AS(t.get<double>(U"name"), Error);
}

TEST_CASE("TomlTable::getOr - returns the default when the key is missing") {
  const TOMLReader reader{MemoryViewReader{kToml.data(), kToml.size()}};
  const TomlTable t{reader};
  REQUIRE(t.getOr<double>(U"missing", 7.0) == 7.0);
  REQUIRE(t.getOr<double>(U"speed", 7.0) == 150.0);
}

TEST_CASE("TomlTable::getOr - throws when the type differs") {
  const TOMLReader reader{MemoryViewReader{kToml.data(), kToml.size()}};
  const TomlTable t{reader};
  REQUIRE_THROWS_AS(t.getOr<double>(U"name", 7.0), Error);
}

#endif
