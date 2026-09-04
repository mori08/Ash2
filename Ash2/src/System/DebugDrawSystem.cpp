#include "System/DebugDrawSystem.hpp"

#include "Component/Attack.hpp"
#include "Component/Hp.hpp"
#include "Screen.hpp"

namespace {

/// カプセル輪郭の太さ（drawFrame/drawArc は内外均等に配分する）
constexpr double kThickness = 2.0;
constexpr double kHalfThickness = kThickness / 2.0;
/// 接地線の太さ
constexpr double kGroundThickness = 2.0;
/// 接地線のアルファ
// カプセルより一段弱い情報だと判別できるようにする
constexpr double kGroundAlpha = 0.8;

/// 攻撃判定（Collider + Attack）の色
constexpr ColorF kAttackColor = Palette::Red;
/// 被弾判定（Collider + Hp、Attack を除く）の色
constexpr ColorF kHpColor = Palette::Green;
/// 判定に参加していない Collider（残り）の色
constexpr ColorF kNeutralColor = Palette::Gray;

/// @brief WorldPos + Collider のオフセット（x=w y=h z=d）を画面座標に変換する
[[nodiscard]] Vec2 ToScreen(const WorldPos& origin, const Vec3& offset) {
  const WorldPos world{
      .w = origin.w + offset.x,
      .h = origin.h + offset.y,
      .d = origin.d + offset.z,
  };
  return WorldToScreen(world);
}

/// @brief カプセルの中点のワールド座標を返す
[[nodiscard]] WorldPos CapsuleCenter(
    const WorldPos& origin, const Collider& capsule
) {
  const Vec3 mid = (capsule.segmentStart + capsule.segmentEnd) / 2.0;
  return WorldPos{
      .w = origin.w + mid.x, .h = origin.h + mid.y, .d = origin.d + mid.z
  };
}

/// @brief ビューの全エンティティに DrawGroundLine・DrawCapsule を適用する
template <class View>
void DrawEach(View view, const ColorF& color) {
  // コンポーネント数の異なる3つのビューを1つの view.each() で回すことは
  // できない（構造化束縛は要素数が固定される）ため、エンティティを反復して
  // view.get<T>() で取り出す
  for (const entt::entity entity : view) {
    const auto& pos = view.template get<const WorldPos>(entity);
    const auto& col = view.template get<const Collider>(entity);
    DebugDrawSystem::DrawGroundLine(pos, col, color);
    DebugDrawSystem::DrawCapsule(pos, col, color);
  }
}

}  // namespace

void DebugDrawSystem::DrawCapsule(
    const WorldPos& origin, const Collider& capsule, const ColorF& color
) {
  const Vec2 p0 = ToScreen(origin, capsule.segmentStart);
  const Vec2 p1 = ToScreen(origin, capsule.segmentEnd);
  const double r = capsule.radius;

  // 高さ方向・奥行き方向のカプセルは投影の性質上どちらも同じ形に映る
  // （区別したい場合は DrawGroundLine の接地線を併用する）
  const Vec2 d = p1 - p0;
  if (d.isZero()) {
    // 近接攻撃のヒットボックス等、線分が退化して点になる形状は珍しくない
    Circle{p0, r}.drawFrame(kHalfThickness, kHalfThickness, color);
    return;
  }

  // Siv3D の角度は 0 時の方向（真上）から時計回り。getAngle() は
  // atan2(x, -y) と同義で、drawArc の角度規約とそのまま噛み合う
  const double angle = d.getAngle();
  // 端点の弧は外側を向く半円だけを描く（丸ごと描くと内側に弧が残り輪郭が読めない）
  Circle{p1, r}.drawArc(
      angle - Math::HalfPi, Math::Pi, kHalfThickness, kHalfThickness, color
  );
  Circle{p0, r}.drawArc(
      angle + Math::HalfPi, Math::Pi, kHalfThickness, kHalfThickness, color
  );
  const Vec2 n = d.normalized().rotated90() * r;
  Line{p0 + n, p1 + n}.draw(kThickness, color);
  Line{p0 - n, p1 - n}.draw(kThickness, color);
}

void DebugDrawSystem::DrawGroundLine(
    const WorldPos& origin, const Collider& capsule, const ColorF& color
) {
  const WorldPos mid = CapsuleCenter(origin, capsule);
  const Vec2 from = WorldToScreen(mid);
  // h だけを 0 にした点が接地点。toScreen() は {w, -(d+h)} なので、
  // 画面上では from の真下へ h ピクセル降りる線になる
  const Vec2 to = WorldToScreen(WorldPos{.w = mid.w, .h = 0.0, .d = mid.d});
  Line{from, to}.draw(
      LineStyle::SquareDot, kGroundThickness, color.withAlpha(kGroundAlpha)
  );
}

void DebugDrawSystem::DrawColliders(const entt::registry& registry) {
  DrawEach(
      registry.view<const WorldPos, const Collider, const Attack>(),
      kAttackColor
  );
  DrawEach(
      registry.view<const WorldPos, const Collider, const Hp>(
          entt::exclude<Attack>
      ),
      kHpColor
  );
  DrawEach(
      registry.view<const WorldPos, const Collider>(entt::exclude<Attack, Hp>),
      kNeutralColor
  );
}
