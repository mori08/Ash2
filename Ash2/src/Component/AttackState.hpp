#pragma once
#include <Siv3D.hpp>

#include <entt/entt.hpp>

/// @brief プレイヤーが攻撃状態であることを示すコンポーネント
struct AttackState {
  /// 現在再生中の攻撃クリップ名
  s3d::String clip;
  /// 攻撃モーション残り時間（秒）
  double timer = 0.0;
  /// 攻撃判定の子エンティティ（遠距離攻撃時は entt::null）
  entt::entity entity = entt::null;
};
