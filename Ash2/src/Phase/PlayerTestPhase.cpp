#include "Phase/PlayerTestPhase.hpp"

#include "Component/Dead.hpp"
#include "Component/Hierarchy.hpp"
#include "Component/WorldPos.hpp"
#include "Config/EnemyConfig.hpp"
#include "DebugOnly.hpp"
#include "Factory/EnemyFactory.hpp"
#include "Factory/PlayerFactory.hpp"
#include "FrameData.hpp"
#include "Phase/StageGameOverPhase.hpp"
#include "System/BattleSystem.hpp"

void PlayerTestPhase::onAfterPush(entt::registry& registry) {
  m_playerRoot = PlayerFactory::Create(registry, {});

  m_dummyTarget = EnemyFactory::Create(
      registry, {.pos = WorldPos{.w = registry.ctx().get<EnemyConfig>().spawnW}}
  );
}

PhaseCommand PlayerTestPhase::update(
    entt::registry& registry, const FrameData& frameData
) {
  const double dt = frameData.dt;

  BattleSystem::Update(registry, frameData);

  if (m_playerRoot != entt::null && registry.all_of<Dead>(m_playerRoot)) {
    return PhaseCommand::Push{
        .nextPhase = std::make_unique<StageGameOverPhase>()
    };
  }

  // 敵が撃破され破棄されたら respawnSec 後に再生成する
  if (m_dummyTarget != entt::null && !registry.valid(m_dummyTarget)) {
    m_dummyTarget = entt::null;
    m_respawnTimer = registry.ctx().get<EnemyConfig>().respawnSec;
  } else if (m_dummyTarget == entt::null) {
    m_respawnTimer -= dt;
    if (m_respawnTimer <= 0.0) {
      m_dummyTarget = EnemyFactory::Create(
          registry,
          {.pos = WorldPos{.w = registry.ctx().get<EnemyConfig>().spawnW}}
      );
    }
  }

  if (DebugOnly::IsEnemySpawnRequested()) {
    // 複数体でのロック対象選択を確認するための固定配置テーブル（乱数は
    // 使わない）。押すたびに1体ずつ、テーブルを使い切ったら末尾で止まる
    static const Array<WorldPos> kExtraEnemySpawns{
        WorldPos{.w = -150.0, .d = 80.0},
        WorldPos{.w = 250.0, .d = -60.0},
        WorldPos{.w = -100.0, .d = -120.0},
        WorldPos{.w = 100.0, .d = 150.0},
    };
    if (m_extraEnemies.size() < kExtraEnemySpawns.size()) {
      m_extraEnemies.push_back(
          EnemyFactory::Create(
              registry, {.pos = kExtraEnemySpawns[m_extraEnemies.size()]}
          )
      );
    }
  }

  if (DebugOnly::IsConfigReloadRequested()) {
    reloadPlayer(registry);
  }

  if (KeyEscape.down()) {
    return PhaseCommand::Pop{};
  }

  return PhaseCommand::None{};
}

void PlayerTestPhase::reloadPlayer(entt::registry& registry) {
  onBeforePop(registry);
  onAfterPush(registry);
}

void PlayerTestPhase::onBeforePop(entt::registry& registry) {
  // 攻撃判定・光エンティティ（AttackOrb タグを持つ珠）は m_playerRoot
  // の子孫なので DestroyWithChildren で連動して破棄される
  if (m_playerRoot != entt::null) {
    Hierarchy::DestroyWithChildren(registry, m_playerRoot);
    m_playerRoot = entt::null;
  }
  if (m_dummyTarget != entt::null && registry.valid(m_dummyTarget)) {
    // 敵に付いたレティクル（LockOn.target のアタッチ先）も連動して破棄する
    Hierarchy::DestroyWithChildren(registry, m_dummyTarget);
    m_dummyTarget = entt::null;
  }
  for (const auto entity : m_extraEnemies) {
    Hierarchy::DestroyWithChildren(registry, entity);
  }
  m_extraEnemies.clear();

  BattleSystem::Cleanup(registry);
}
