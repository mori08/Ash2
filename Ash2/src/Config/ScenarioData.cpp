#include "Config/ScenarioData.hpp"

ScenarioData ScenarioData::FromToml(const s3d::TOMLValue& toml) {
  ScenarioData data;
  for (const auto& member : toml.tableView()) {
    s3d::Array<s3d::TOMLValue> steps;
    for (const auto& step : member.value.tableArrayView()) {
      steps.push_back(step);
    }
    data.sections[member.name] = std::move(steps);
  }
  return data;
}
