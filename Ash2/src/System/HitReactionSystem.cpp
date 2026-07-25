#include <Siv3D.hpp>

#include "System/HitReactionSystem.hpp"

#include "Component/Attack.hpp"
#include "Component/Collider.hpp"
#include "Component/Drawable.hpp"
#include "Component/Enemy.hpp"
#include "Component/EnemyMotion.hpp"
#include "Component/Hierarchy.hpp"
#include "Component/Hitstop.hpp"
#include "Component/Hp.hpp"
#include "Component/Motion.hpp"
#include "Component/Velocity.hpp"
#include "Component/WorldPos.hpp"
#include "Config/EnemyConfig.hpp"

void HitReactionSystem::Apply(entt::registry& registry,
                              const Array<HitPair>& hits) {
  const auto& cfg = registry.ctx().get<EnemyConfig>();

  for (const auto& hit : hits) {
    const auto& attack = registry.get<Attack>(hit.attacker);

    // ヒットボックスの親（攻撃側本体）を解決する。ヒットストップ付与と、
    // 後段の弾き・吹っ飛び方向の判定の両方で使う
    auto attackerOwner = hit.attacker;
    if (const auto* hierarchy = registry.try_get<Hierarchy>(hit.attacker);
        hierarchy != nullptr && hierarchy->parent() != entt::null) {
      attackerOwner = hierarchy->parent();
    }

    // hitstopSec 0 の攻撃（弾など）はヒットストップ付与のみ省く。
    // 撃破判定・リアクション適用まで飛ばすと弾で敵が倒れなくなる
    if (attack.hitstopSec > 0.0) {
      // 長い停止中に別ヒットの短い停止で上書きされないよう、
      // 既に付与済みなら長い方を残す
      const auto grantHitstop = [&registry,
                                 sec = attack.hitstopSec](entt::entity e) {
        auto& hitstop = registry.get_or_emplace<Hitstop>(e);
        hitstop.remaining = Max(hitstop.remaining, sec);
      };
      grantHitstop(attackerOwner);
      grantHitstop(hit.target);
    }

    // リアクションは Enemy を持つ被弾側にのみ適用する（プレイヤーの被弾は
    // #166）
    if (!registry.all_of<Enemy>(hit.target)) continue;

    // 同一フレームの多重ヒットで、先のヒットにより Hp
    // が外れた（Defeated 済みの）被弾側は再処理しない
    auto* hp = registry.try_get<Hp>(hit.target);
    if (hp == nullptr) continue;

    // 弾き・吹っ飛びの向きは攻撃側本体と被弾側の w を比較して決める
    // （珠自身は突き軌道で敵の中心を越えうるため使わない）
    const double ownerW = registry.get<WorldPos>(attackerOwner).w;
    const double targetW = registry.get<WorldPos>(hit.target).w;
    const double sign = (targetW < ownerW) ? -1.0 : 1.0;

    // Motion を差し替えない経路では後始末もしない。後始末だけが走ると
    // 前の状態が続いたまま Velocity と size を失う
    if (attack.reaction == ReactionLevel::None && hp->current > 0) continue;

    // Motion は Tick() の返り値を経由せずここで直接 replace する（被弾は
    // Tick() 側が知らない外部要因による強制遷移のため）。前の状態が
    // 遺した Velocity・縮んだ size は Tick() 満了時にしか戻らないので、
    // 上書きする前にここで後始末しておく
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
      registry.replace<Motion>(
          hit.target, EnemyMotion::Defeated{.remaining = cfg.defeatedSec});
      continue;
    }

    switch (attack.reaction) {
      case ReactionLevel::None:
        break;
      case ReactionLevel::Stagger:
        registry.replace<Motion>(
            hit.target, EnemyMotion::Stagger{.remaining = cfg.staggerSec});
        break;
      case ReactionLevel::Repel:
        registry.get<Velocity>(hit.target).w = sign * cfg.repelSpeed;
        registry.replace<Motion>(hit.target,
                                 EnemyMotion::Repel{.remaining = cfg.repelSec});
        break;
      case ReactionLevel::Blow:
        registry.get<Velocity>(hit.target).w = sign * cfg.blowSpeedW;
        registry.get<Velocity>(hit.target).h = cfg.blowSpeedH;
        registry.replace<Motion>(
            hit.target, EnemyMotion::Knockback{.remaining = cfg.knockbackSec});
        break;
    }
  }
}
