#pragma once

#include <Siv3D.hpp>

#include <functional>
#include <memory>

class IPhase;

/// @brief フェーズ名とファクトリ関数の対応表
using PhaseFactory =
    std::function<std::unique_ptr<IPhase>(const s3d::TOMLValue&)>;

/// @brief フェーズ名 → ファクトリ関数のマップ
using PhaseRegistry = s3d::HashTable<s3d::String, PhaseFactory>;
