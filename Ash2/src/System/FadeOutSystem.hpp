#pragma once
#include <entt/entt.hpp>

/// @brief 消滅演出中のエンティティを透過させ、満了したら破棄するシステム
class FadeOutSystem {
 public:
  /// @brief FadeOut の残り時間を減算して DrawColor::color.a
  /// に反映し、尽きたエンティティを破棄する
  static void Update(entt::registry& registry, double dt);
};
