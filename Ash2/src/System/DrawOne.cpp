#include "System/DrawOne.hpp"

#include "Util/Overloaded.hpp"

void DrawOne(
    const Drawable& drawable, const Vec2& screenPos, const ColorF& color
) {
  std::visit(
      Overloaded{
          [&screenPos, &color](const RectDrawable& data) {
            const RectF rect = [&] {
              switch (data.anchor) {
                case DrawAnchor::BottomCenter:
                  return RectF{
                      Arg::bottomCenter(screenPos), data.size.x, data.size.y
                  };
                case DrawAnchor::TopLeft:
                  return RectF{
                      Arg::topLeft(screenPos), data.size.x, data.size.y
                  };
                case DrawAnchor::Center:
                default:
                  return RectF{
                      Arg::center(screenPos), data.size.x, data.size.y
                  };
              }
            }();
            rect.draw(color);
          },
          [&screenPos, &color](const CircleDrawable& data) {
            const Circle circle{screenPos, data.radius};
            circle.draw(color);
          },
          [&screenPos, &color](const TextureDrawable& data) {
            // 図形は AA が効くので丸めない。テクスチャのみ整数化する
            const Vec2 anchorPos = Math::Round(screenPos + data.drawOffset);
            switch (data.anchor) {
              case DrawAnchor::BottomCenter:
                data.region.draw(Arg::bottomCenter(anchorPos), color);
                break;
              case DrawAnchor::TopLeft:
                data.region.draw(Arg::topLeft(anchorPos), color);
                break;
              case DrawAnchor::Center:
              default:
                data.region.draw(Arg::center(anchorPos), color);
                break;
            }
          },
          [&screenPos, &color](const TextDrawable& data) {
            // テクスチャと同じく整数位置にして、にじみを防ぐ
            const Vec2 anchorPos = Math::Round(screenPos);
            switch (data.anchor) {
              case DrawAnchor::BottomCenter:
                data.font(data.text).draw(Arg::bottomCenter(anchorPos), color);
                break;
              case DrawAnchor::TopLeft:
                data.font(data.text).draw(Arg::topLeft(anchorPos), color);
                break;
              case DrawAnchor::Center:
              default:
                data.font(data.text).draw(Arg::center(anchorPos), color);
                break;
            }
          },
      },
      drawable
  );
}
