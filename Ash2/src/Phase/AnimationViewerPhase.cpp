#include "Phase/AnimationViewerPhase.hpp"

#include "Component/Drawable.hpp"
#include "Component/Hierarchy.hpp"
#include "Component/SpriteAnimation.hpp"
#include "Component/WorldPos.hpp"
#include "Config/AnimationData.hpp"
#include "FatalError.hpp"
#include "FrameData.hpp"
#include "System/AnimationSystem.hpp"
#include "UiFonts.hpp"

namespace {
constexpr double kBgBrightness = 0.15;
constexpr int32 kTitleX = 20;
constexpr int32 kTitleY = 20;
constexpr int32 kClipInfoY = 50;
constexpr int32 kHintY = 80;
}  // namespace

AnimationViewerPhase::AnimationViewerPhase(const Param& param)
    : m_dataKey(param.dataKey) {}

void AnimationViewerPhase::onAfterPush(entt::registry& registry) {
  const auto& animRegistry = registry.ctx().get<AnimationDataRegistry>();
  const auto it = animRegistry.find(m_dataKey);
  if (it == animRegistry.end()) {
    throw FatalError{
        .reason = FatalReason::ConfigInvalid,
        .detail =
            U"AnimationViewerPhase::onAfterPush: dataKey '{}' が "
            U"AnimationDataRegistry にありません"_fmt(m_dataKey),
    };
  }

  const auto& data = it->second;
  m_clips.clear();
  for (const auto& [name, _] : data.clips) {
    m_clips.push_back(name);
  }
  m_clips.sort();
  m_clipIndex = 0;

  m_entity = registry.create();
  registry.emplace<WorldPos>(m_entity);
  registry.emplace<Drawable>(
      m_entity, TextureDrawable{.anchor = DrawAnchor::BottomCenter}
  );
  registry.emplace<SpriteAnimation>(
      m_entity,
      SpriteAnimation{
          .dataKey = m_dataKey,
          .currentClip = m_clips.empty() ? U"" : m_clips[0]
      }
  );
  AnimationSystem::Update(registry, 0.0);
}

PhaseCommand AnimationViewerPhase::update(
    entt::registry& registry, const FrameData& frameData
) {
  if (KeyEscape.down()) {
    return PhaseCommand::Pop{};
  }

  if (!m_clips.empty()) {
    bool changed = false;
    if (KeyLeft.down()) {
      m_clipIndex = (m_clipIndex - 1 + m_clips.size()) % m_clips.size();
      changed = true;
    }
    if (KeyRight.down()) {
      m_clipIndex = (m_clipIndex + 1) % m_clips.size();
      changed = true;
    }

    if (changed && m_entity != entt::null) {
      auto* anim = registry.try_get<SpriteAnimation>(m_entity);
      if (anim) {
        anim->currentClip = m_clips[m_clipIndex];
        anim->elapsed = 0.0;
      }
    }

    if (KeyF.down() && m_entity != entt::null) {
      auto* anim = registry.try_get<SpriteAnimation>(m_entity);
      if (anim) {
        anim->facingRight = !anim->facingRight;
      }
    }

    if (KeyR.down() && m_entity != entt::null) {
      auto* anim = registry.try_get<SpriteAnimation>(m_entity);
      if (anim) {
        anim->elapsed = 0.0;
      }
    }
  }

  AnimationSystem::Update(registry, frameData.dt);

  const auto& font = registry.ctx().get<UiFonts>().small;

  Scene::SetBackground(ColorF{kBgBrightness});
  font(U"AnimationViewer: {}"_fmt(m_dataKey)).draw(kTitleX, kTitleY);
  if (!m_clips.empty()) {
    font(
        U"Clip [{}/{}]: {}"_fmt(
            m_clipIndex + 1, m_clips.size(), m_clips[m_clipIndex]
        )
    )
        .draw(kTitleX, kClipInfoY);
  }
  font(U"← → : clip  F : flip  R : replay  Esc : back")
      .draw(kTitleX, kHintY, Palette::Gray);

  return PhaseCommand::None{};
}

void AnimationViewerPhase::onBeforePop(entt::registry& registry) {
  if (m_entity == entt::null) {
    return;
  }
  Hierarchy::DestroyWithChildren(registry, m_entity);
  m_entity = entt::null;
}
