#pragma once
#include <Siv3D.hpp>

#include "IPhase.hpp"

/// @brief TOML シナリオに従ってエンティティ生成・フェーズ遷移を進めるフェーズ
class ScenarioPhase : public IPhase {
 public:
  /// @brief ScenarioPhase の生成パラメータ
  struct Param {
    /// 処理するシナリオセクション名
    s3d::String sectionName;
  };

  /// @brief コンストラクタ（Param 受け取り版）
  /// @param param 生成パラメータ
  explicit ScenarioPhase(const Param& param);

  /// @brief currentStep_ を初期化する
  /// @param registry ECS レジストリ
  void onAfterPush(entt::registry& registry) override;

  /// @brief 毎フレーム 1 ステップを処理する
  /// @param registry ECS レジストリ
  /// @param frameData フレームごとの更新データ
  /// @return フェーズスタックへの操作
  [[nodiscard]] PhaseCommand update(entt::registry& registry,
                                    const FrameData& frameData) override;

 private:
  /// シナリオセクション名
  s3d::String m_sectionName;
  /// 現在のステップインデックス
  size_t m_currentStep = 0;
};
