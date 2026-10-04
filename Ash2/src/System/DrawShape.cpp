#include "System/DrawShape.hpp"

#include "Util/Overloaded.hpp"

void DrawShape(
    const Drawable& drawable, const Vec2& screenPos, const ColorF& color
) {
  std::visit(
      Overloaded{
          [&screenPos, &color](const RectDrawable& shape) {
            const RectF rect = [&] {
              switch (shape.anchor) {
                case DrawAnchor::BottomCenter:
                  return RectF{
                      Arg::bottomCenter(screenPos), shape.size.x, shape.size.y
                  };
                case DrawAnchor::TopLeft:
                  return RectF{
                      Arg::topLeft(screenPos), shape.size.x, shape.size.y
                  };
                case DrawAnchor::Center:
                default:
                  return RectF{
                      Arg::center(screenPos), shape.size.x, shape.size.y
                  };
              }
            }();
            rect.draw(color);
          },
          [&screenPos, &color](const CircleDrawable& shape) {
            const Circle circle{screenPos, shape.radius};
            circle.draw(color);
          },
          [&screenPos, &color](const TextureDrawable& shape) {
            // 図形は AA が効くので丸めない。テクスチャのみ整数化する
            const Vec2 anchorPos = Math::Round(screenPos + shape.drawOffset);
            switch (shape.anchor) {
              case DrawAnchor::BottomCenter:
                shape.region.draw(Arg::bottomCenter(anchorPos), color);
                break;
              case DrawAnchor::TopLeft:
                shape.region.draw(Arg::topLeft(anchorPos), color);
                break;
              case DrawAnchor::Center:
              default:
                shape.region.draw(Arg::center(anchorPos), color);
                break;
            }
          },
          [&screenPos, &color](const TextDrawable& shape) {
            // テクスチャと同じく整数位置にして、にじみを防ぐ
            const Vec2 anchorPos = Math::Round(screenPos);
            switch (shape.anchor) {
              case DrawAnchor::BottomCenter:
                shape.font(shape.text)
                    .draw(Arg::bottomCenter(anchorPos), color);
                break;
              case DrawAnchor::TopLeft:
                shape.font(shape.text).draw(Arg::topLeft(anchorPos), color);
                break;
              case DrawAnchor::Center:
              default:
                shape.font(shape.text).draw(Arg::center(anchorPos), color);
                break;
            }
          },
      },
      drawable
  );
}
