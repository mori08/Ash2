#pragma once
#include <entt/entt.hpp>

struct FrameData;

/// @brief プレイヤーの通常状態（NeutralState）での攻撃入力処理を行うシステム
class NeutralStateSystem {
 public:
  /// @brief NeutralState
  /// を持つエンティティの攻撃入力をチェックし、開始処理を行う
  ///
  /// 地上にいる状態で近距離・遠距離攻撃の入力があった場合、`NeutralState`
  /// を `erase` して `AttackState` を `emplace` する（Neutral → Attack
  /// 遷移）。近距離攻撃時は攻撃判定の子エンティティを、遠距離攻撃時は弾エンティティを生成する。
  static void Update(entt::registry& registry, const FrameData& frameData);
};
