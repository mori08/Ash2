#pragma once
#include <entt/entt.hpp>

/// @brief 遠距離照準のロック状態（プレイヤーへ付与する）
struct LockOn {
  /// ロック確定中の敵
  entt::entity target = entt::null;
  /// 半ロック中の敵
  entt::entity halfTarget = entt::null;
  /// target に追従するレティクル
  entt::entity targetReticle = entt::null;
  /// halfTarget に追従するレティクル
  entt::entity halfReticle = entt::null;
  /// 右スティックを倒していたか（ヒステリシス用）
  bool stickTilted = false;
};
