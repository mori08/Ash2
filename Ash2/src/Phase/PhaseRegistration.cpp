#include <Siv3D.hpp>

#include "Phase/DemoPhase.hpp"
#include "Phase/ScenarioPhase.hpp"
#include "Phase/WaitPhase.hpp"

std::unique_ptr<IPhase> DemoPhase::FromToml(const s3d::TOMLValue&) {
  return std::make_unique<DemoPhase>();
}

std::unique_ptr<IPhase> ScenarioPhase::FromToml(const s3d::TOMLValue& step) {
  return std::make_unique<ScenarioPhase>(step[U"param"].get<String>());
}

std::unique_ptr<IPhase> WaitPhase::FromToml(const s3d::TOMLValue& step) {
  return std::make_unique<WaitPhase>(step[U"duration"].get<double>());
}
