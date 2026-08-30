#include <Siv3D.hpp>

#include "System/HitReactionSystem.hpp"

#include "Component/Collider.hpp"
#include "Component/Drawable.hpp"
#include "Component/EnemyMotion.hpp"
#include "Component/Hitstop.hpp"
#include "Component/Hp.hpp"
#include "Component/PlayerMotion.hpp"
#include "Component/ReactionLevel.hpp"
#include "Component/SpriteAnimation.hpp"
#include "Component/Velocity.hpp"
#include "Component/WorldPos.hpp"
#include "Config/EnemyConfig.hpp"
#include "Config/PlayerConfig.hpp"
#include "System/PlayerMotion/Transition.hpp"

namespace {

/// @brief Enemy 被弾側へリアクションを適用する
///
/// `Hp` が枯渇していれば `reaction` によらず `Defeated` へ強制遷移し
/// （`Collider`/`Hp` を外す）、それ以外は `reaction` に応じた
/// `EnemyMotion::Variant` 遷移と、攻撃側本体との位置関係から決めた向きの
/// `Velocity` を適用する。
void ApplyEnemyReaction(entt::registry& registry, const HitEvent& hit) {
  const auto& cfg = registry.ctx().get<EnemyConfig>();

  // 同一フレームの多重ヒットで、先のヒットにより Hp
  // が外れた（Defeated 済みの）被弾側は再処理しない
  auto* hp = registry.try_get<Hp>(hit.target);
  if (hp == nullptr) return;

  // 弾き・吹っ飛びの向きは攻撃側本体と被弾側の w を比較して決める
  // （珠自身は突き軌道で敵の中心を越えうるため使わない）
  const double ownerW = registry.get<WorldPos>(hit.attackerOwner).w;
  const double targetW = registry.get<WorldPos>(hit.target).w;
  const double sign = (targetW < ownerW) ? -1.0 : 1.0;

  // EnemyMotion::Variant を差し替えない経路では後始末もしない。後始末だけが
  // 走ると前の状態が続いたまま Velocity と size を失う
  if (hit.reaction == ReactionLevel::None && hp->current > 0) return;

  // EnemyMotion::Variant は Tick() の返り値を経由せずここで直接 replace
  // する（被弾は Tick() 側が知らない外部要因による強制遷移のため）。
  // 前の状態が遺した Velocity・縮んだ size は Tick() 満了時にしか戻らない
  // ので、上書きする前にここで後始末しておく
  registry.get<Velocity>(hit.target) = Velocity{};
  if (auto* drawable = registry.try_get<Drawable>(hit.target);
      drawable != nullptr) {
    if (auto* rect = std::get_if<RectDrawable>(drawable); rect != nullptr) {
      rect->size = cfg.size;
    }
  }

  if (hp->current <= 0) {
    registry.remove<Collider>(hit.target);
    registry.remove<Hp>(hit.target);
    registry.replace<EnemyMotion::Variant>(
        hit.target, EnemyMotion::Defeated{.remaining = cfg.defeatedSec}
    );
    return;
  }

  switch (hit.reaction) {
    case ReactionLevel::None:
      break;
    case ReactionLevel::Stagger:
      registry.replace<EnemyMotion::Variant>(
          hit.target, EnemyMotion::Stagger{.remaining = cfg.staggerSec}
      );
      break;
    case ReactionLevel::Repel:
      registry.get<Velocity>(hit.target).w = sign * cfg.repelSpeed;
      registry.replace<EnemyMotion::Variant>(
          hit.target, EnemyMotion::Repel{.remaining = cfg.repelSec}
      );
      break;
    case ReactionLevel::Blow:
      registry.get<Velocity>(hit.target).w = sign * cfg.blowSpeedW;
      registry.get<Velocity>(hit.target).h = cfg.blowSpeedH;
      registry.replace<EnemyMotion::Variant>(
          hit.target, EnemyMotion::Knockback{.remaining = cfg.knockbackSec}
      );
      break;
  }
}

/// @brief Player 被弾側へリアクションを適用する
///
/// `reaction` が `None` の場合は遷移させない（ダメージのみ）。それ以外は
/// `PlayerMotion::MakeDamaged` が Stagger/Knockback を決める。
void ApplyPlayerReaction(entt::registry& registry, const HitEvent& hit) {
  if (hit.reaction == ReactionLevel::None) return;

  const auto& cfg = registry.ctx().get<PlayerConfig>();

  // 吹き飛ばし方向は攻撃側本体と被弾側の w を比較して決める
  const double ownerW = registry.get<WorldPos>(hit.attackerOwner).w;
  const double targetW = registry.get<WorldPos>(hit.target).w;
  const double sign = (targetW < ownerW) ? -1.0 : 1.0;

  auto& anim = registry.get<SpriteAnimation>(hit.target);
  const auto next = PlayerMotion::MakeDamaged(
      registry, hit.target, cfg, anim, hit.reaction, sign
  );
  registry.replace<PlayerMotion::Variant>(hit.target, next);
}

}  // namespace

void HitReactionSystem::Apply(
    entt::registry& registry, const Array<HitEvent>& hits
) {
  for (const auto& hit : hits) {
    // hitstopSec 0 の攻撃（弾など）はヒットストップ付与のみ省く。
    // 撃破判定・リアクション適用まで飛ばすと弾で敵が倒れなくなる
    if (hit.hitstopSec > 0.0) {
      // 長い停止中に別ヒットの短い停止で上書きされないよう、
      // 既に付与済みなら長い方を残す
      const auto grantHitstop =
          [&registry, sec = hit.hitstopSec](entt::entity e) {
            auto& hitstop = registry.get_or_emplace<Hitstop>(e);
            hitstop.remaining = Max(hitstop.remaining, sec);
          };
      grantHitstop(hit.attackerOwner);
      grantHitstop(hit.target);
    }

    // 被弾側がどちらのモーション表現を持つかで分岐する
    // （Apply*Reaction はいずれも対応する ...Motion::Variant を replace
    // する）。
    // 走査するのはエンティティのビューではなくヒットイベントの列なので、
    // MotionSystem のように型ごとのビューへ分ける形は取れない
    if (registry.all_of<EnemyMotion::Variant>(hit.target)) {
      ApplyEnemyReaction(registry, hit);
    } else if (registry.all_of<PlayerMotion::Variant>(hit.target)) {
      ApplyPlayerReaction(registry, hit);
    }
  }
}
