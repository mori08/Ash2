#include "Phase/StagePhase.hpp"

#include "Component/Dead.hpp"
#include "Component/DrawColor.hpp"
#include "Component/Drawable.hpp"
#include "Component/Gauge.hpp"
#include "Component/Hierarchy.hpp"
#include "Component/ScreenPos.hpp"
#include "Factory/EnemyFactory.hpp"
#include "Factory/PlayerFactory.hpp"
#include "Factory/StageData.hpp"
#include "FatalError.hpp"
#include "FrameData.hpp"
#include "Phase/StageClearPhase.hpp"
#include "Phase/StageGameOverPhase.hpp"
#include "System/BattleSystem.hpp"

namespace {

constexpr double kGaugeX = 16.0;
constexpr double kHpGaugeY = 16.0;
constexpr double kStaminaGaugeY = 40.0;
constexpr double kGaugeWidth = 200.0;
constexpr double kGaugeHeight = 18.0;
constexpr ColorF kGaugeBgColor{0.2, 0.2, 0.2, 0.7};
constexpr ColorF kHpColor{0.2, 0.8, 0.2};
constexpr ColorF kStaminaColor{0.9, 0.8, 0.1};

/// @brief 背景と fill の2体1組のゲージを生成し、gauges に積む
/// @param source fill が値を読む先のエンティティ
template <typename GaugeT>
void AppendGauge(
    entt::registry& registry, entt::entity source, double y,
    const ColorF& fillColor, Array<entt::entity>& gauges
) {
  const Vec2 pos{kGaugeX, y};

  const auto bg = registry.create();
  registry.emplace<ScreenPos>(bg, ScreenPos{.pos = pos, .layer = 0});
  registry.emplace<Drawable>(
      bg,
      RectDrawable{
          .size = SizeF{kGaugeWidth, kGaugeHeight},
          .anchor = DrawAnchor::TopLeft
      }
  );
  registry.emplace<DrawColor>(bg, DrawColor{.color = kGaugeBgColor});

  const auto fill = registry.create();
  registry.emplace<ScreenPos>(fill, ScreenPos{.pos = pos, .layer = 1});
  registry.emplace<Drawable>(
      fill,
      RectDrawable{
          .size = SizeF{0.0, kGaugeHeight}, .anchor = DrawAnchor::TopLeft
      }
  );
  registry.emplace<DrawColor>(fill, DrawColor{.color = fillColor});
  registry.emplace<GaugeT>(fill, GaugeT{.fullWidth = kGaugeWidth});
  registry.emplace<GaugeSource>(fill, GaugeSource{.entity = source});

  gauges.push_back(bg);
  gauges.push_back(fill);
}

}  // namespace

StagePhase::StagePhase(const Param& param) : m_stageName(param.stageName) {}

void StagePhase::onAfterPush(entt::registry& registry) {
  const auto& stageData = registry.ctx().get<StageData>();
  const auto it = stageData.stages.find(m_stageName);
  if (it == stageData.stages.end()) {
    throw FatalError{
        .reason = FatalReason::ConfigInvalid,
        .detail =
            U"StagePhase::onAfterPush: stageName '{}' が StageData "
            U"にありません"_fmt(m_stageName),
    };
  }

  m_playerRoot = PlayerFactory::Create(registry, {});
  m_gauges.clear();
  AppendGauge<HpGauge>(registry, m_playerRoot, kHpGaugeY, kHpColor, m_gauges);
  AppendGauge<StaminaGauge>(
      registry, m_playerRoot, kStaminaGaugeY, kStaminaColor, m_gauges
  );
  m_enemies.clear();
  for (const auto& param : it->second.enemies) {
    m_enemies.push_back(EnemyFactory::Create(registry, param));
  }
}

PhaseCommand StagePhase::update(
    entt::registry& registry, const FrameData& frameData
) {
  BattleSystem::Update(registry, frameData);

  // 最後の敵の消滅フェード中にプレイヤーが倒れた場合を GAME OVER にするため、
  // 撃破判定をクリア判定より先に評価する
  if (m_playerRoot != entt::null && registry.all_of<Dead>(m_playerRoot)) {
    return PhaseCommand::Push{
        .nextPhase = std::make_unique<StageGameOverPhase>()
    };
  }

  // 敵は Defeated のフェード満了後に EnemySystem が破棄するので、
  // 消滅を待ってクリアになる
  const bool allDefeated = m_enemies.all([&registry](entt::entity enemy) {
    return !registry.valid(enemy);
  });
  if (allDefeated) {
    return PhaseCommand::Push{.nextPhase = std::make_unique<StageClearPhase>()};
  }

  return PhaseCommand::None{};
}

void StagePhase::onBeforePop(entt::registry& registry) {
  // GaugeSource が参照先を失った状態を残さないため、プレイヤーより先に破棄する
  for (const auto gauge : m_gauges) {
    registry.destroy(gauge);
  }
  m_gauges.clear();

  if (m_playerRoot != entt::null) {
    Hierarchy::DestroyWithChildren(registry, m_playerRoot);
    m_playerRoot = entt::null;
  }
  for (const auto enemy : m_enemies) {
    if (registry.valid(enemy)) {
      // 敵に付いたレティクル（LockOn.target のアタッチ先）も連動して破棄する
      Hierarchy::DestroyWithChildren(registry, enemy);
    }
  }
  m_enemies.clear();

  BattleSystem::Cleanup(registry);
}
