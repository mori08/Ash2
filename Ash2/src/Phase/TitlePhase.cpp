#include "Phase/TitlePhase.hpp"

#include "Component/DrawColor.hpp"
#include "Component/DrawPos.hpp"
#include "Component/Drawable.hpp"
#include "Component/FocusColor.hpp"
#include "Component/Hierarchy.hpp"
#include "Config/TitleConfig.hpp"
#include "Debug.hpp"
#include "Factory/MenuFactory.hpp"
#include "FrameData.hpp"
#include "Phase/ScenarioPhase.hpp"
#include "System/FocusColorSystem.hpp"
#include "System/MenuSystem.hpp"
#include "UiFonts.hpp"

namespace {
constexpr StringView kLogoTextureKey = U"assets/images/logo.png";

// TODO(#386): 色が仮置きで、本番の見た目になっていない
const ColorF kNormalColor{1.0};
const ColorF kFocusedColor{Palette::Yellow};

/// @brief ボタンに文字と選択色を付ける
void AddButtonLook(
    entt::registry& registry, entt::entity button, const String& label
) {
  registry.emplace<Drawable>(
      button, TextDrawable{.text = label, .font = FontAsset{UiFonts::kLarge}}
  );
  registry.emplace<DrawColor>(button, DrawColor{.color = kNormalColor});
  registry.emplace<FocusColor>(
      button, FocusColor{.normal = kNormalColor, .focused = kFocusedColor}
  );
}
}  // namespace

void TitlePhase::onAfterPush(entt::registry& registry) {
  const auto& config = registry.ctx().get<TitleConfig>();

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
  AddButtonLook(registry, startButton, U"はじめる");
  AddButtonLook(registry, quitButton, U"おわる");

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
  registry.destroy(m_logo);
  m_logo = entt::null;
  m_actions.clear();
}
