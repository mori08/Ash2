#include "ReactionLevel.hpp"

ReactionLevel ParseReactionLevel(const String& value) {
  if (value == U"none") return ReactionLevel::None;
  if (value == U"stagger") return ReactionLevel::Stagger;
  if (value == U"repel") return ReactionLevel::Repel;
  if (value == U"blow") return ReactionLevel::Blow;
  throw Error{U"reaction が不明な値"};
}
