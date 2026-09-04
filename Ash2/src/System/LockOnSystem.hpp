#pragma once
#include <Siv3D.hpp>

#include <entt/entt.hpp>

#include "Component/Collider.hpp"
#include "Component/WorldPos.hpp"
#include "Config/PlayerConfig.hpp"

struct FrameData;

/// @brief 画面へ投影したカプセル（カメラオフセットは含まない）
struct ScreenCapsule {
  Vec2 start;
  Vec2 end;
  double radius = 0.0;
};

/// @brief 遠距離照準のロック対象・レティクルを更新するシステム
class LockOnSystem {
 public:
  /// @brief LockOn を持つエンティティ（プレイヤー）のロック対象を更新し、
  /// 追従するレティクルを同期する
  /// @note MotionSystem より前に呼ぶこと（同フレームの Ranged
  /// 発射がその場のロック対象を見るため）。AttachmentSystem・AnimationSystem
  /// より前でよい（レティクルの WorldPos と TextureDrawable::region は
  /// 同フレーム内でそれぞれが埋める）
  static void Update(entt::registry& registry, const FrameData& frameData);

  /// @brief Collider 線分の中点（狙点）のワールド座標
  [[nodiscard]] static WorldPos AimPoint(
      const WorldPos& pos, const Collider& col
  );

  /// @brief Collider を画面へ投影し、半径を scale 倍したカプセルを返す
  [[nodiscard]] static ScreenCapsule Project(
      const WorldPos& pos, const Collider& col, double scale
  );

  /// @brief 点がカプセルの内側か
  /// @return 境界線上を含めて true
  [[nodiscard]] static bool Contains(const ScreenCapsule& cap, Vec2 point);

  /// @brief 軸から dir 方向にある敵を選ぶ
  /// @return 候補がなければ entt::null
  [[nodiscard]] static entt::entity SelectByDirection(
      const entt::registry& registry, entt::entity axis, Vec2 dir,
      const LockConfig& cfg
  );
};
