#ifdef _DEBUG
#include <ThirdParty/Catch2/catch.hpp>

#include "Config/ScenarioData.hpp"
#include "Phase/PhaseLoaders.hpp"
#include "Phase/WaitPhase.hpp"

TEST_CASE("ScenarioData::FromToml - push action creates StepPush") {
  constexpr std::string_view kToml =
      "[[intro]]\n"
      "action = \"push\"\n"
      "phase = \"wait\"\n"
      "duration = 1.5\n";
  const TOMLReader reader{MemoryViewReader{kToml.data(), kToml.size()}};
  const auto data = ScenarioData::FromToml(reader, GetPhaseLoaders());
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
  constexpr std::string_view kToml =
      "[[intro]]\n"
      "action = \"reset\"\n"
      "phase = \"wait\"\n"
      "duration = 2.0\n";
  const TOMLReader reader{MemoryViewReader{kToml.data(), kToml.size()}};
  const auto data = ScenarioData::FromToml(reader, GetPhaseLoaders());
  REQUIRE(data.sections.contains(U"intro"));
  REQUIRE(data.sections.at(U"intro").size() == 1);
  const auto& step = data.sections.at(U"intro")[0];
  REQUIRE(std::holds_alternative<StepReset>(step));
  const auto& maker = std::get<StepReset>(step).maker;
  REQUIRE(maker != nullptr);
  const auto phase = maker->make();
  REQUIRE(dynamic_cast<WaitPhase*>(phase.get()) != nullptr);
}

TEST_CASE("ScenarioData::FromToml - unknown phase name throws") {
  constexpr std::string_view kToml =
      "[[intro]]\n"
      "action = \"push\"\n"
      "phase = \"nonexistent\"\n";
  const TOMLReader reader{MemoryViewReader{kToml.data(), kToml.size()}};
  REQUIRE_THROWS_AS(
      (void)ScenarioData::FromToml(reader, GetPhaseLoaders()), Error
  );
}

TEST_CASE("ScenarioData::FromToml - unknown action throws") {
  constexpr std::string_view kToml =
      "[[intro]]\n"
      "action = \"fly\"\n"
      "phase = \"wait\"\n";
  const TOMLReader reader{MemoryViewReader{kToml.data(), kToml.size()}};
  REQUIRE_THROWS_AS(
      (void)ScenarioData::FromToml(reader, GetPhaseLoaders()), Error
  );
}

TEST_CASE("ScenarioData::FromToml - missing duration throws") {
  constexpr std::string_view kToml =
      "[[intro]]\n"
      "action = \"push\"\n"
      "phase = \"wait\"\n";
  const TOMLReader reader{MemoryViewReader{kToml.data(), kToml.size()}};
  REQUIRE_THROWS_AS(
      (void)ScenarioData::FromToml(reader, GetPhaseLoaders()), Error
  );
}

TEST_CASE("ScenarioData::FromToml - missing param throws") {
  constexpr std::string_view kToml =
      "[[intro]]\n"
      "action = \"push\"\n"
      "phase = \"scenario\"\n";
  const TOMLReader reader{MemoryViewReader{kToml.data(), kToml.size()}};
  REQUIRE_THROWS_AS(
      (void)ScenarioData::FromToml(reader, GetPhaseLoaders()), Error
  );
}

TEST_CASE("ScenarioData::FromToml - stage without param throws") {
  constexpr std::string_view kToml =
      "[[intro]]\n"
      "action = \"push\"\n"
      "phase = \"stage\"\n";
  const TOMLReader reader{MemoryViewReader{kToml.data(), kToml.size()}};
  REQUIRE_THROWS_AS(
      (void)ScenarioData::FromToml(reader, GetPhaseLoaders()), Error
  );
}

TEST_CASE(
    "ScenarioData::FromToml - reference to nonexistent section "
    "throws"
) {
  constexpr std::string_view kToml =
      "[[intro]]\n"
      "action = \"push\"\n"
      "phase = \"scenario\"\n"
      "param = \"nonexistent\"\n";
  const TOMLReader reader{MemoryViewReader{kToml.data(), kToml.size()}};
  REQUIRE_THROWS_AS(
      (void)ScenarioData::FromToml(reader, GetPhaseLoaders()), Error
  );
}

TEST_CASE(
    "ScenarioData::FromToml - forward reference to later section succeeds"
) {
  constexpr std::string_view kToml =
      "[[intro]]\n"
      "action = \"push\"\n"
      "phase = \"scenario\"\n"
      "param = \"later\"\n"
      "[[later]]\n"
      "action = \"push\"\n"
      "phase = \"wait\"\n"
      "duration = 1.0\n";
  const TOMLReader reader{MemoryViewReader{kToml.data(), kToml.size()}};
  REQUIRE_NOTHROW((void)ScenarioData::FromToml(reader, GetPhaseLoaders()));
}

#endif
