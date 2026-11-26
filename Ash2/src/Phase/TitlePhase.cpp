#include "Phase/TitlePhase.hpp"

#include "Component/Bubble.hpp"
#include "Component/DrawColor.hpp"
#include "Component/DrawPos.hpp"
#include "Component/Drawable.hpp"
#include "Component/FocusColor.hpp"
#include "Component/Hierarchy.hpp"
#include "Component/Ripple.hpp"
#include "Config/TitleConfig.hpp"
#include "Debug.hpp"
#include "Factory/MenuFactory.hpp"
#include "Factory/PlantFactory.hpp"
#include "FrameData.hpp"
#include "Phase/ScenarioPhase.hpp"
#include "System/BubbleSystem.hpp"
#include "System/FocusColorSystem.hpp"
#include "System/MenuSystem.hpp"
#include "System/PlantSwaySystem.hpp"
#include "System/RippleSystem.hpp"
#include "UiFonts.hpp"

namespace {
constexpr StringView kLogoTextureKey = U"assets/images/logo.png";

/// @brief ボタンに文字と選択色を付ける
void AddButtonLook(
    entt::registry& registry, entt::entity button, const String& label,
    const TitleConfig& config
) {
  registry.emplace<Drawable>(
      button, TextDrawable{.text = label, .font = FontAsset{UiFonts::kLarge}}
  );
  registry.emplace<DrawColor>(
      button, DrawColor{.color = config.buttonNormalColor}
  );
  registry.emplace<FocusColor>(
      button,
      FocusColor{
          .normal = config.buttonNormalColor,
          .focused = config.buttonFocusedColor
      }
  );
}
}  // namespace

void TitlePhase::onAfterPush(entt::registry& registry) {
  const auto& config = registry.ctx().get<TitleConfig>();

  // 前後は生成順でなく depth で決める（背景が奥）
  m_background = registry.create();
  registry.emplace<DrawPos>(
      m_background,
      DrawPos{
          .pos = Vec2{0.0, 0.0}, .layer = DrawLayer::kBackground, .depth = 1.0
      }
  );
  registry.emplace<Drawable>(
      m_background,
      RectDrawable{.size = SizeF{Scene::Size()}, .anchor = DrawAnchor::TopLeft}
  );
  registry.emplace<DrawColor>(
      m_background, DrawColor{.color = config.backgroundColor}
  );

  m_surfaceLine = registry.create();
  registry.emplace<DrawPos>(
      m_surfaceLine,
      DrawPos{
          .pos = Vec2{Scene::CenterF().x, config.surfaceY},
          .layer = DrawLayer::kBackground,
          .depth = 0.0
      }
  );
  registry.emplace<Drawable>(
      m_surfaceLine,
      RectDrawable{
          .size =
              SizeF{
                  static_cast<double>(Scene::Width()), config.surfaceLineWidth
              },
          .anchor = DrawAnchor::Center
      }
  );
  registry.emplace<DrawColor>(
      m_surfaceLine, DrawColor{.color = config.surfaceLineColor}
  );

  m_plants.clear();
  for (const auto& plant : config.plants) {
    m_plants.push_back(PlantFactory::Create(registry, config, plant));
  }

  m_logo = registry.create();
  registry.emplace<DrawPos>(
      m_logo,
      DrawPos{
          .pos = Vec2{Scene::CenterF().x, config.logoY},
          .layer = DrawLayer::kHud
      }
  );
  registry.emplace<Drawable>(
      m_logo, TextureDrawable{.region = TextureAsset{kLogoTextureKey}}
  );
  registry.emplace<DrawColor>(m_logo, DrawColor{.color = config.logoColor});

  // 上下が互いを指すため、2ボタンを先に作ってから設定する
  m_menu = registry.create();
  const auto startButton = registry.create();
  const auto quitButton = registry.create();
  const double centerX = Scene::CenterF().x;
  const SizeF buttonSize{config.buttonWidth, config.buttonHeight};

  MenuFactory::SetupMenu(registry, m_menu, startButton);
  MenuFactory::SetupButton(
      registry, startButton, m_menu,
      {.pos = Vec2{centerX, config.menuY},
       .size = buttonSize,
       .down = quitButton}
  );
  MenuFactory::SetupButton(
      registry, quitButton, m_menu,
      {.pos = Vec2{centerX, config.menuY + config.menuSpacing},
       .size = buttonSize,
       .up = startButton}
  );
  AddButtonLook(registry, startButton, U"はじめる", config);
  AddButtonLook(registry, quitButton, U"おわる", config);

  m_bubbleWait = 0.0;

  m_actions.clear();
  // 開始演出は自分の上に積む。演出側が Reset するため、Title は終わりを待たない
  m_actions[startButton] = [] {
    APP_LOG(U"[TitlePhase] はじめる が押された");
    return PhaseCommand{PhaseCommand::Push{
        .nextPhase = std::make_unique<ScenarioPhase>(
            ScenarioPhase::Param{.sectionName = String{kTitleStartSectionName}}
        )
    }};
  };
  m_actions[quitButton] = [] {
    APP_LOG(U"[TitlePhase] おわる が押された");
    return PhaseCommand{PhaseCommand::Exit{}};
  };
}

PhaseCommand TitlePhase::update(
    entt::registry& registry, const FrameData& frameData
) {
  PlantSwaySystem::Update(registry, frameData.dt);
  updateBubbles(registry, frameData);

  const auto pressed = MenuSystem::Update(registry, frameData.input, m_menu);
  FocusColorSystem::Update(registry);

  if (!pressed) return PhaseCommand::None{};
  const auto it = m_actions.find(*pressed);
  if (it == m_actions.end()) {
    throw Error{U"TitlePhase: 押されたボタンに対応する処理がありません"};
  }
  return it->second();
}

void TitlePhase::onBeforePop(entt::registry& registry) {
  Hierarchy::DestroyWithChildren(registry, m_menu);
  m_menu = entt::null;
  for (const auto plant : m_plants) {
    Hierarchy::DestroyWithChildren(registry, plant);
  }
  m_plants.clear();
  // 走査中に destroy しないよう、先に集める
  const auto bubbles =
      registry.view<Bubble>() | std::ranges::to<Array<entt::entity>>();
  for (const auto bubble : bubbles) {
    registry.destroy(bubble);
  }
  const auto ripples =
      registry.view<Ripple>() | std::ranges::to<Array<entt::entity>>();
  for (const auto ripple : ripples) {
    registry.destroy(ripple);
  }
  registry.destroy(m_logo);
  m_logo = entt::null;
  registry.destroy(m_surfaceLine);
  m_surfaceLine = entt::null;
  registry.destroy(m_background);
  m_background = entt::null;
  m_actions.clear();
}

void TitlePhase::updateBubbles(
    entt::registry& registry, const FrameData& frameData
) {
  const auto& config = registry.ctx().get<TitleConfig>();
  const auto& bubble = config.bubble;
  const RectF swamp{
      0.0, config.surfaceY, static_cast<double>(Scene::Width()),
      Scene::Height() - config.surfaceY
  };

  m_bubbleWait -= frameData.dt;
  while (m_bubbleWait <= 0.0) {
    m_bubbleWait +=
        bubble.interval *
        frameData.random.range(
            1.0 - bubble.intervalJitter, 1.0 + bubble.intervalJitter
        );
    BubbleSystem::Spawn(registry, frameData, bubble, swamp);
  }
  const auto pops = BubbleSystem::Update(registry, frameData);

  // 生まれたフレームで時間を進めないよう、Update の後に Spawn する
  RippleSystem::Update(registry, frameData.dt);
  RippleSystem::Spawn(registry, pops, config.ripple);
}
