#ifdef _DEBUG
#include <ThirdParty/Catch2/catch.hpp>
#include <entt/entt.hpp>

#include "FrameData.hpp"
#include "Phase/PhaseStack.hpp"

namespace {

struct PhaseSpy {
  int32 afterPushCount = 0;
  int32 beforePopCount = 0;
  int32 updateCount = 0;
};

/// @brief MockPhase が update() で返すコマンドの種類
enum class MockCommand : uint8 {
  None,
  Pop,
  Push,
  Reset,
};

class MockPhase : public IPhase {
 public:
  explicit MockPhase(
      std::shared_ptr<PhaseSpy> spy, MockCommand cmd = MockCommand::None,
      std::unique_ptr<IPhase> next = nullptr
  )
      : m_spy(std::move(spy)), m_cmd(cmd), m_next(std::move(next)) {}

  void onAfterPush(entt::registry&) override { m_spy->afterPushCount++; }
  void onBeforePop(entt::registry&) override { m_spy->beforePopCount++; }

  PhaseCommand update(entt::registry&, const FrameData&) override {
    m_spy->updateCount++;
    switch (m_cmd) {
      case MockCommand::None:
        return PhaseCommand::None{};
      case MockCommand::Pop:
        return PhaseCommand::Pop{};
      case MockCommand::Push:
        return PhaseCommand::Push{.nextPhase = std::move(m_next)};
      case MockCommand::Reset:
        return PhaseCommand::Reset{.nextPhase = std::move(m_next)};
    }
    return PhaseCommand::None{};
  }

 private:
  std::shared_ptr<PhaseSpy> m_spy;
  MockCommand m_cmd;
  std::unique_ptr<IPhase> m_next;
};

}  // namespace

TEST_CASE("PhaseStack - None command leaves stack unchanged") {
  entt::registry registry;
  auto spy = std::make_shared<PhaseSpy>();
  PhaseStack stack{std::make_unique<MockPhase>(spy), registry};

  stack.update(registry, FrameData{});
  REQUIRE(spy->updateCount == 1);

  stack.update(registry, FrameData{});
  REQUIRE(spy->updateCount == 2);
}

TEST_CASE("PhaseStack - Pop command calls onBeforePop and removes phase") {
  entt::registry registry;
  auto spy = std::make_shared<PhaseSpy>();
  PhaseStack stack{
      std::make_unique<MockPhase>(spy, MockCommand::Pop), registry
  };

  REQUIRE(spy->beforePopCount == 0);
  stack.update(registry, FrameData{});
  REQUIRE(spy->beforePopCount == 1);

  stack.update(registry, FrameData{});
  REQUIRE(spy->updateCount == 1);
}

TEST_CASE("PhaseStack - Push command calls onAfterPush on new phase") {
  entt::registry registry;
  auto spy1 = std::make_shared<PhaseSpy>();
  auto spy2 = std::make_shared<PhaseSpy>();
  PhaseStack stack{
      std::make_unique<MockPhase>(
          spy1, MockCommand::Push, std::make_unique<MockPhase>(spy2)
      ),
      registry
  };

  REQUIRE(spy1->afterPushCount == 1);
  REQUIRE(spy2->afterPushCount == 0);

  stack.update(registry, FrameData{});
  REQUIRE(spy2->afterPushCount == 1);
}

TEST_CASE("PhaseStack - Reset command pops all phases then pushes new phase") {
  entt::registry registry;
  auto spy1 = std::make_shared<PhaseSpy>();
  auto spy2 = std::make_shared<PhaseSpy>();
  PhaseStack stack{
      std::make_unique<MockPhase>(
          spy1, MockCommand::Reset, std::make_unique<MockPhase>(spy2)
      ),
      registry
  };

  stack.update(registry, FrameData{});
  REQUIRE(spy1->beforePopCount == 1);
  REQUIRE(spy2->afterPushCount == 1);
}

TEST_CASE("PhaseStack - update on empty stack does nothing") {
  entt::registry registry;
  auto spy = std::make_shared<PhaseSpy>();
  PhaseStack stack{
      std::make_unique<MockPhase>(spy, MockCommand::Pop), registry
  };

  stack.update(registry, FrameData{});
  REQUIRE(spy->updateCount == 1);

  stack.update(registry, FrameData{});
  REQUIRE(spy->updateCount == 1);
}

#endif
