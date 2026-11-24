#include "Factory/PlantFactory.hpp"

#include "Component/DrawColor.hpp"
#include "Component/DrawPos.hpp"
#include "Component/Drawable.hpp"
#include "Component/Hierarchy.hpp"
#include "Component/Plant.hpp"
#include "System/PlantSwaySystem.hpp"

namespace {
/// 背景（1.0）と水面線（0.0）の間
constexpr double kPlantDepth = 0.5;
}  // namespace

entt::entity PlantFactory::Create(
    entt::registry& registry, const TitleConfig& config,
    const PlantConfig& plant
) {
  const Vec2 root{plant.x, config.surfaceY};

  const auto parent = registry.create();
  registry.emplace<DrawPos>(
      parent,
      DrawPos{
          .pos = root, .layer = DrawLayer::kBackground, .depth = kPlantDepth
      }
  );
  registry.emplace<Plant>(
      parent,
      Plant{
          .period = plant.period,
          .offset = plant.offset,
          .spreadAmplitude = config.plantSpreadAmplitude
      }
  );

  for (const auto& stemConfig : config.plantStems) {
    const auto stem = registry.create();
    registry.emplace<PlantStem>(stem, PlantStem{.dx = stemConfig.dx});
    registry.emplace<DrawPos>(
        stem,
        DrawPos{
            .pos = root, .layer = DrawLayer::kBackground, .depth = kPlantDepth
        }
    );
    registry.emplace<Drawable>(
        stem,
        RectDrawable{
            .size = SizeF{config.plantStemWidth, stemConfig.height},
            .anchor = DrawAnchor::BottomCenter
        }
    );
    registry.emplace<DrawColor>(stem, DrawColor{.color = config.plantColor});
    Hierarchy::Attach(registry, parent, stem);
  }

  PlantSwaySystem::Place(registry, parent);
  return parent;
}
