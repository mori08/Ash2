#ifdef _DEBUG
#include <ThirdParty/Catch2/catch.hpp>
#include <entt/entt.hpp>

#include "Factory/StageData.hpp"
#include "FatalError.hpp"
#include "Phase/StagePhase.hpp"

TEST_CASE("StagePhase::onAfterPush - throws FatalError for unknown stageName") {
  // StageData を空のまま登録し、未登録のステージ名で呼び出す
  entt::registry registry;
  registry.ctx().emplace<StageData>();

  StagePhase phase{StagePhase::Param{.stageName = U"unknown"}};
  REQUIRE_THROWS_AS(phase.onAfterPush(registry), FatalError);
}

#endif
