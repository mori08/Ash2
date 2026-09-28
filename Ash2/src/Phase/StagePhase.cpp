#include "Phase/StagePhase.hpp"

#include "Component/Dead.hpp"
#include "Component/Hierarchy.hpp"
#include "Factory/EnemyFactory.hpp"
#include "Factory/PlayerFactory.hpp"
#include "FatalError.hpp"
#include "Phase/StageClearPhase.hpp"
#include "Phase/StageData.hpp"
#include "Phase/StageGameOverPhase.hpp"
#include "System/BattleSystem.hpp"

StagePhase::StagePhase(const Param& param) : m_stageName(param.stageName) {}

void StagePhase::onAfterPush(entt::registry& registry) {
  const auto& stages = registry.ctx().get<StageDataRegistry>();
  const auto it = stages.find(m_stageName);
  if (it == stages.end()) {
    throw FatalError{
        .reason = FatalReason::ConfigInvalid,
        .detail =
            U"StagePhase::onAfterPush: stageName '{}' が StageDataRegistry "
            U"にありません"_fmt(m_stageName),
    };
  }

  m_player = PlayerFactory::Create(registry, {});

  m_enemies.clear();
  for (const auto& enemyParam : it->second.enemies) {
    m_enemies.push_back(EnemyFactory::Create(registry, enemyParam));
  }
}

PhaseCommand StagePhase::update(
    entt::registry& registry, const FrameData& frameData
) {
  BattleSystem::Update(registry, frameData);

  // 撃破を先に見る：最後の敵の消滅と撃破が同じフレームに重なったら
  // GAME OVER を優先する
  if (registry.valid(m_player) && registry.all_of<Dead>(m_player)) {
    return PhaseCommand::Push{
        .nextPhase = std::make_unique<StageGameOverPhase>()
    };
  }

  m_enemies.remove_if([&](entt::entity e) { return !registry.valid(e); });
  if (m_enemies.empty()) {
    return PhaseCommand::Push{.nextPhase = std::make_unique<StageClearPhase>()};
  }

  if (KeyEscape.down()) {
    return PhaseCommand::Pop{};
  }

  return PhaseCommand::None{};
}

void StagePhase::onBeforePop(entt::registry& registry) {
  if (m_player != entt::null) {
    Hierarchy::DestroyWithChildren(registry, m_player);
    m_player = entt::null;
  }
  for (const auto entity : m_enemies) {
    if (registry.valid(entity)) {
      Hierarchy::DestroyWithChildren(registry, entity);
    }
  }
  m_enemies.clear();

  BattleSystem::Cleanup(registry);
}
