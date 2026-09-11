#include "ReactionLevel.hpp"

std::expected<ReactionLevel, String> ParseReactionLevel(const String& value) {
  if (value == U"none") return ReactionLevel::None;
  if (value == U"stagger") return ReactionLevel::Stagger;
  if (value == U"repel") return ReactionLevel::Repel;
  if (value == U"blow") return ReactionLevel::Blow;
  return std::unexpected{
      U"ParseReactionLevel: 不明な reaction \"" + value + U"\""
  };
}
