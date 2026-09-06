#ifdef _DEBUG
#include <ThirdParty/Catch2/catch.hpp>
#include <entt/entt.hpp>

#include "Config/AnimationData.hpp"
#include "FatalError.hpp"
#include "Phase/AnimationViewerPhase.hpp"

TEST_CASE(
    "AnimationViewerPhase::onAfterPush - throws FatalError for unregistered "
    "dataKey"
) {
  // AnimationDataRegistry を空のまま登録し、未登録キーで呼び出す
  entt::registry registry;
  registry.ctx().emplace<AnimationDataRegistry>();

  AnimationViewerPhase phase{
      AnimationViewerPhase::Param{.dataKey = U"unknown"}
  };
  REQUIRE_THROWS_AS(phase.onAfterPush(registry), FatalError);
}

#endif
