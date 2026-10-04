#include "System/GaugeSystem.hpp"

#include <cassert>
#include <variant>

#include "Component/Drawable.hpp"
#include "Component/Gauge.hpp"
#include "Component/Hp.hpp"
#include "Component/Stamina.hpp"

namespace {

/// @brief GaugeT を持つゲージの幅を、StatT の current / max の割合で更新する
template <typename GaugeT, typename StatT>
void UpdateFill(entt::registry& registry) {
  for (auto&& [entity, gauge, source, drawable] :
       registry.view<const GaugeT, const GaugeSource, Drawable>().each()) {
    assert(
        registry.valid(source.entity) &&
        "GaugeSource の参照先が破棄されています"
    );
    const auto& stat = registry.get<const StatT>(source.entity);
    auto* rect = std::get_if<RectDrawable>(&drawable);
    assert(
        rect != nullptr &&
        "ゲージの Drawable は RectDrawable である必要があります"
    );
    const double ratio =
        (stat.max > 0)
            ? Clamp(static_cast<double>(stat.current) / stat.max, 0.0, 1.0)
            : 0.0;
    rect->size.x = gauge.fullWidth * ratio;
  }
}

}  // namespace

void GaugeSystem::Update(entt::registry& registry) {
  UpdateFill<HpGauge, Hp>(registry);
  UpdateFill<StaminaGauge, Stamina>(registry);
}
