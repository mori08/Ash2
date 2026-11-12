#include "Factory/MenuFactory.hpp"

#include "Component/DrawPos.hpp"
#include "Component/Hierarchy.hpp"
#include "Component/Menu.hpp"

void MenuFactory::SetupMenu(
    entt::registry& registry, entt::entity menuEntity,
    entt::entity defaultButton
) {
  registry.emplace<Menu>(menuEntity, Menu{.defaultButton = defaultButton});
}

void MenuFactory::SetupButton(
    entt::registry& registry, entt::entity buttonEntity,
    entt::entity menuEntity, const ButtonParam& param
) {
  registry.emplace<Button>(
      buttonEntity,
      Button{
          .size = param.size,
          .up = param.up,
          .down = param.down,
          .left = param.left,
          .right = param.right,
      }
  );
  registry.emplace<DrawPos>(
      buttonEntity, DrawPos{.pos = param.pos, .layer = param.layer}
  );
  Hierarchy::Attach(registry, menuEntity, buttonEntity);
}
