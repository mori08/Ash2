#include "Phase/AnimationViewerPhase.hpp"

#include "Component/Drawable.hpp"
#include "Component/Hierarchy.hpp"
#include "Component/SpriteAnimation.hpp"
#include "Component/WorldPos.hpp"
#include "Config/AnimationData.hpp"
#include "Phase/FrameData.hpp"
#include "System/AnimationSystem.hpp"

namespace {
constexpr double KBgBrightness = 0.15;
constexpr int KTitleX = 20;
constexpr int KTitleY = 20;
constexpr int KClipInfoY = 50;
constexpr int KHintY = 80;
}  // namespace

AnimationViewerPhase::AnimationViewerPhase(const Param& param)
    : m_dataKey(param.dataKey) {}

void AnimationViewerPhase::onAfterPush(entt::registry& registry) {
  const auto& animRegistry = registry.ctx().get<AnimationDataRegistry>();
  const auto it = animRegistry.find(m_dataKey);
  if (it == animRegistry.end()) {
    return;
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
      m_entity, TextureDrawable{.anchor = DrawAnchor::BottomCenter});
  registry.emplace<SpriteAnimation>(
      m_entity,
      SpriteAnimation{.dataKey = m_dataKey,
                      .currentClip = m_clips.empty() ? U"" : m_clips[0]});
  AnimationSystem::Update(registry, 0.0);
}

IPhase::PhaseCommand AnimationViewerPhase::update(entt::registry& registry,
                                                  const FrameData& frameData) {
  if (KeyEscape.down()) {
    return PhaseCommand::Pop();
  }

  if (!m_clips.empty()) {
    bool changed = false;
    if (KeyLeft.down()) {
      m_clipIndex = (m_clipIndex - 1 + static_cast<int>(m_clips.size())) %
                    static_cast<int>(m_clips.size());
      changed = true;
    }
    if (KeyRight.down()) {
      m_clipIndex = (m_clipIndex + 1) % static_cast<int>(m_clips.size());
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
  }

  AnimationSystem::Update(registry, frameData.dt);

  Scene::SetBackground(ColorF{KBgBrightness});
  m_font(U"AnimationViewer: {}"_fmt(m_dataKey)).draw(KTitleX, KTitleY);
  if (!m_clips.empty()) {
    m_font(U"Clip [{}/{}]: {}"_fmt(m_clipIndex + 1, m_clips.size(),
                                   m_clips[m_clipIndex]))
        .draw(KTitleX, KClipInfoY);
  }
  m_font(U"← → : clip  F : flip  Esc : back")
      .draw(KTitleX, KHintY, Palette::Gray);

  return PhaseCommand::None();
}

void AnimationViewerPhase::onBeforePop(entt::registry& registry) {
  if (m_entity == entt::null) {
    return;
  }
  Hierarchy::DestroyWithChildren(registry, m_entity);
  m_entity = entt::null;
}
