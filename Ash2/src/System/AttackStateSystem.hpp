#pragma once
#include <entt/entt.hpp>

struct FrameData;

/// @brief プレイヤーの攻撃状態（AttackState）の終了処理を行うシステム
class AttackStateSystem {
 public:
  /// @brief AttackState を持つエンティティのタイマーを減算し、終了処理を行う
  ///
  /// `timer` を `frameData.dt` だけ減算し、0以下になったら `AttackState`
  /// を `erase` して `NeutralState` を `emplace` する（Attack → Neutral
  /// 遷移）。`AttackState.entity` が `entt::null` でない場合は
  /// `Hierarchy::DestroyWithChildren` で攻撃判定エンティティを破棄する。
  static void Update(entt::registry& registry, const FrameData& frameData);
};
