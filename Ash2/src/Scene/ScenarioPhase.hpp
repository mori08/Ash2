#pragma once
#include <Siv3D.hpp>

#include "IPhase.hpp"

/// @brief TOML シナリオに従ってエンティティ生成・フェーズ遷移を進めるフェーズ
class ScenarioPhase : public IPhase {
 public:
  /// @brief コンストラクタ
  /// @param sectionName 処理するシナリオセクション名
  explicit ScenarioPhase(const s3d::String& sectionName);

  /// @brief currentStep_ を初期化する
  /// @param registry ECS レジストリ
  void onAfterPush(entt::registry& registry) override;

  /// @brief 毎フレーム 1 ステップを処理する
  /// @param registry ECS レジストリ
  /// @return フェーズスタックへの操作
  [[nodiscard]] PhaseCommand update(entt::registry& registry) override;

  /// @brief 生成したエンティティと NameLookup エントリを削除する
  /// @param registry ECS レジストリ
  void onBeforePop(entt::registry& registry) override;

 private:
  /// シナリオセクション名
  s3d::String m_sectionName;
  /// 現在のステップインデックス
  size_t m_currentStep = 0;
  /// このフェーズで生成したエンティティ一覧
  s3d::Array<entt::entity> m_createdEntities;
};
