#pragma once

#include <memory>

#include "IPhase.hpp"

namespace s3d {
class TOMLValue;
}

/// @brief プレイヤー操作デモシーン
class DemoPhase : public IPhase {
 public:
  /// @brief TOML ステップからインスタンスを生成する
  /// @param step TOML ステップ（パラメータ不要）
  /// @return DemoPhase インスタンス
  [[nodiscard]] static std::unique_ptr<IPhase> FromToml(
      const s3d::TOMLValue& step);

  /// @brief プレイヤーエンティティを生成する
  /// @param registry ECS レジストリ
  void onAfterPush(entt::registry& registry) override;

  /// @brief 毎フレームの更新処理
  /// @param registry ECS レジストリ
  /// @param dt 経過時間（秒）
  /// @return フェーズスタックへの操作
  [[nodiscard]] PhaseCommand update(entt::registry& registry,
                                    double dt) override;
};
