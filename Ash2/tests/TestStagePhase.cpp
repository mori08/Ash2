#ifdef _DEBUG
#include <ThirdParty/Catch2/catch.hpp>
#include <entt/entt.hpp>

#include "FatalError.hpp"
#include "Phase/StageData.hpp"
#include "Phase/StagePhase.hpp"

TEST_CASE(
    "StagePhase::onAfterPush - throws FatalError for unregistered stageName"
) {
  // StageDataRegistry を空のまま登録し、未登録キーで呼び出す
  entt::registry registry;
  registry.ctx().emplace<StageDataRegistry>();

  StagePhase phase{StagePhase::Param{.stageName = U"unknown"}};
  REQUIRE_THROWS_AS(phase.onAfterPush(registry), FatalError);
}

#endif
