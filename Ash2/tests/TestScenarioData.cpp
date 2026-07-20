#ifdef _DEBUG
#include <ThirdParty/Catch2/catch.hpp>

#include "Config/ScenarioData.hpp"
#include "Phase/PhaseLoaders.hpp"
#include "Phase/WaitPhase.hpp"

TEST_CASE("ScenarioData::FromToml - push action creates StepPush") {
  constexpr std::string_view Toml =
      "[[intro]]\n"
      "action = \"push\"\n"
      "phase = \"wait\"\n"
      "duration = 1.5\n";
  const TOMLReader reader{MemoryViewReader{Toml.data(), Toml.size()}};
  const ScenarioData data = ScenarioData::FromToml(reader, GetPhaseLoaders());
  REQUIRE(data.sections.contains(U"intro"));
  REQUIRE(data.sections.at(U"intro").size() == 1);
  const auto& step = data.sections.at(U"intro")[0];
  REQUIRE(std::holds_alternative<StepPush>(step));
  const auto& maker = std::get<StepPush>(step).maker;
  REQUIRE(maker != nullptr);
  const auto phase = maker->make();
  REQUIRE(dynamic_cast<WaitPhase*>(phase.get()) != nullptr);
}

TEST_CASE("ScenarioData::FromToml - reset action creates StepReset") {
  constexpr std::string_view Toml =
      "[[intro]]\n"
      "action = \"reset\"\n"
      "phase = \"wait\"\n"
      "duration = 2.0\n";
  const TOMLReader reader{MemoryViewReader{Toml.data(), Toml.size()}};
  const ScenarioData data = ScenarioData::FromToml(reader, GetPhaseLoaders());
  REQUIRE(data.sections.contains(U"intro"));
  REQUIRE(data.sections.at(U"intro").size() == 1);
  const auto& step = data.sections.at(U"intro")[0];
  REQUIRE(std::holds_alternative<StepReset>(step));
  const auto& maker = std::get<StepReset>(step).maker;
  REQUIRE(maker != nullptr);
  const auto phase = maker->make();
  REQUIRE(dynamic_cast<WaitPhase*>(phase.get()) != nullptr);
}

TEST_CASE("ScenarioData::FromToml - unknown phase name throws Error") {
  constexpr std::string_view Toml =
      "[[intro]]\n"
      "action = \"push\"\n"
      "phase = \"nonexistent\"\n";
  const TOMLReader reader{MemoryViewReader{Toml.data(), Toml.size()}};
  REQUIRE_THROWS_AS(ScenarioData::FromToml(reader, GetPhaseLoaders()), Error);
}

TEST_CASE("ScenarioData::FromToml - unknown action throws Error") {
  constexpr std::string_view Toml =
      "[[intro]]\n"
      "action = \"fly\"\n"
      "phase = \"wait\"\n";
  const TOMLReader reader{MemoryViewReader{Toml.data(), Toml.size()}};
  REQUIRE_THROWS_AS(ScenarioData::FromToml(reader, GetPhaseLoaders()), Error);
}

TEST_CASE("ScenarioData::FromToml - missing duration throws Error") {
  constexpr std::string_view Toml =
      "[[intro]]\n"
      "action = \"push\"\n"
      "phase = \"wait\"\n";
  const TOMLReader reader{MemoryViewReader{Toml.data(), Toml.size()}};
  REQUIRE_THROWS_AS(ScenarioData::FromToml(reader, GetPhaseLoaders()), Error);
}

TEST_CASE("ScenarioData::FromToml - missing param throws Error") {
  constexpr std::string_view Toml =
      "[[intro]]\n"
      "action = \"push\"\n"
      "phase = \"scenario\"\n";
  const TOMLReader reader{MemoryViewReader{Toml.data(), Toml.size()}};
  REQUIRE_THROWS_AS(ScenarioData::FromToml(reader, GetPhaseLoaders()), Error);
}

#endif
