#ifdef _DEBUG
#include <ThirdParty/Catch2/catch.hpp>

#include "Config/ScenarioData.hpp"
#include "Phase/PhaseRegistry.hpp"

TEST_CASE("ScenarioData::FromToml - push action creates StepPush") {
  constexpr std::string_view Toml =
      "[[intro]]\n"
      "action = \"push\"\n"
      "phase = \"wait\"\n"
      "duration = 1.5\n";
  const TOMLReader reader{MemoryViewReader{Toml.data(), Toml.size()}};
  const PhaseRegistry registry = MakeDefaultPhaseRegistry();
  const ScenarioData data = ScenarioData::FromToml(reader, registry);
  REQUIRE(data.sections.contains(U"intro"));
  REQUIRE(data.sections.at(U"intro").size() == 1);
  REQUIRE(std::holds_alternative<StepPush>(data.sections.at(U"intro")[0]));
  REQUIRE(std::get<StepPush>(data.sections.at(U"intro")[0]).phaseName ==
          U"wait");
}

TEST_CASE("ScenarioData::FromToml - reset action creates StepReset") {
  constexpr std::string_view Toml =
      "[[intro]]\n"
      "action = \"reset\"\n"
      "phase = \"wait\"\n"
      "duration = 2.0\n";
  const TOMLReader reader{MemoryViewReader{Toml.data(), Toml.size()}};
  const PhaseRegistry registry = MakeDefaultPhaseRegistry();
  const ScenarioData data = ScenarioData::FromToml(reader, registry);
  REQUIRE(data.sections.contains(U"intro"));
  REQUIRE(data.sections.at(U"intro").size() == 1);
  REQUIRE(std::holds_alternative<StepReset>(data.sections.at(U"intro")[0]));
  REQUIRE(std::get<StepReset>(data.sections.at(U"intro")[0]).phaseName ==
          U"wait");
}

TEST_CASE("ScenarioData::FromToml - unknown phase name throws Error") {
  constexpr std::string_view Toml =
      "[[intro]]\n"
      "action = \"push\"\n"
      "phase = \"nonexistent\"\n";
  const TOMLReader reader{MemoryViewReader{Toml.data(), Toml.size()}};
  const PhaseRegistry registry = MakeDefaultPhaseRegistry();
  REQUIRE_THROWS_AS(ScenarioData::FromToml(reader, registry), Error);
}

TEST_CASE("ScenarioData::FromToml - unknown action throws Error") {
  constexpr std::string_view Toml =
      "[[intro]]\n"
      "action = \"fly\"\n"
      "phase = \"wait\"\n";
  const TOMLReader reader{MemoryViewReader{Toml.data(), Toml.size()}};
  const PhaseRegistry registry = MakeDefaultPhaseRegistry();
  REQUIRE_THROWS_AS(ScenarioData::FromToml(reader, registry), Error);
}

#endif
