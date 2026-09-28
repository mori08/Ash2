#include "Phase/StageData.hpp"

std::expected<StageData, String> StageData::FromToml(const TOMLValue& table) {
  Array<EnemyFactory::Param> enemies;

  // Why not: table[U"enemies"] がテーブル配列として存在しない場合に
  // tableArrayView() を呼ぶと不正アクセスになるため、
  // 事前に isTableArray() で存在確認する。
  if (const auto& enemiesValue = table[U"enemies"];
      enemiesValue.isTableArray()) {
    size_t index = 0;
    for (const auto& enemyToml : enemiesValue.tableArrayView()) {
      auto enemy = EnemyFactory::Param::FromToml(enemyToml);
      if (!enemy) {
        return std::unexpected{
            U"enemies[{}]: {}"_fmt(index, std::move(enemy).error())
        };
      }
      enemies.push_back(*std::move(enemy));
      ++index;
    }
  }

  // 敵が0体だと開始直後にクリア扱いになるため許容しない
  if (enemies.isEmpty()) {
    return std::unexpected{U"StageData::FromToml: enemies がありません"};
  }

  return StageData{.enemies = std::move(enemies)};
}
