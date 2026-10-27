#include "Factory/StageData.hpp"

#include "Config/TomlTable.hpp"

StageData StageData::FromToml(const TOMLValue& toml) {
  StageData data;

  for (const auto& member : toml.tableView()) {
    const auto& enemiesValue = TomlMember(member.value, U"enemies");
    // 配列でない値に tableArrayView() を呼ぶと例外になりうるため先に確かめる
    if (!enemiesValue.isTableArray()) {
      throw Error{U"ステージの enemies がないか、配列でない"};
    }

    StageDefinition stage;
    for (const auto& enemyToml : enemiesValue.tableArrayView()) {
      stage.enemies.push_back(EnemyFactory::Param::FromToml(enemyToml));
    }

    if (stage.enemies.isEmpty()) {
      throw Error{U"ステージの enemies が空"};
    }
    data.stages[member.name] = std::move(stage);
  }

  return data;
}
