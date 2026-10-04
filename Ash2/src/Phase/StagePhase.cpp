#include "Phase/StagePhase.hpp"

#include "Component/Dead.hpp"
#include "Component/Hierarchy.hpp"
#include "Factory/EnemyFactory.hpp"
#include "Factory/PlayerFactory.hpp"
#include "Factory/StageData.hpp"
#include "FatalError.hpp"
#include "FrameData.hpp"
#include "Phase/StageClearPhase.hpp"
#include "Phase/StageGameOverPhase.hpp"
#include "System/BattleSystem.hpp"

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
