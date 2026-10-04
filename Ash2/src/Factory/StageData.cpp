#include "Factory/StageData.hpp"

std::expected<StageData, String> StageData::FromToml(const TOMLValue& toml) {
  StageData data;

  for (const auto& member : toml.tableView()) {
    const auto& enemiesValue = member.value[U"enemies"];
    // 配列でない値に tableArrayView() を呼ぶと例外になりうるため先に確かめる
    if (!enemiesValue.isTableArray()) {
      return std::unexpected{
          U"StageData::FromToml: ステージ \"{}\" に enemies がありません"_fmt(
              member.name
          )
      };
    }

    StageDefinition stage;
    size_t index = 0;
    for (const auto& enemyToml : enemiesValue.tableArrayView()) {
      auto enemy = EnemyFactory::Param::FromToml(enemyToml);
      if (!enemy) {
        return std::unexpected{
            U"StageData::FromToml: ステージ \"{}\" の enemies[{}]: {}"_fmt(
                member.name, index, std::move(enemy).error()
            )
        };
      }
      stage.enemies.push_back(*std::move(enemy));
      ++index;
    }

    if (stage.enemies.isEmpty()) {
      return std::unexpected{
          U"StageData::FromToml: ステージ \"{}\" の enemies が空です"_fmt(
              member.name
          )
      };
    }
    data.stages[member.name] = std::move(stage);
  }

  return data;
}
