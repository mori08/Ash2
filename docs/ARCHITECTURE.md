# ARCHITECTURE.md

## 概要

「沼に焚べ」の技術構成とコード設計をまとめたドキュメント。  
実装の進捗に合わせて随時更新する。

---

## ディレクトリ構成

```
Ash2/
├── Ash2/
│   ├── src/
│   │   ├── Main.cpp
│   │   ├── stdafx.h
│   │   ├── Component/
│   │   │   ├── Drawable.hpp    # 描画コンポーネント（variant）
│   │   │   ├── Hierarchy.hpp/.cpp  # 親子関係（双方向連結リスト）
│   │   │   ├── LocalOffset.hpp # 親からの相対座標
│   │   │   ├── Name.hpp        # エンティティ名
│   │   │   ├── Player.hpp      # プレイヤータグ
│   │   │   ├── Velocity.hpp    # 速度
│   │   │   └── WorldPos.hpp    # ワールド座標（常に絶対座標）
│   │   ├── Config/
│   │   │   ├── PlayerConfig.hpp/.cpp
│   │   │   └── ScenarioData.hpp/.cpp
│   │   ├── Input/
│   │   │   └── PlayerInputAction.hpp
│   │   ├── System/
│   │   │   ├── AttachmentSystem.hpp/.cpp  # 親子座標伝播
│   │   │   ├── DrawSystem.hpp/.cpp
│   │   │   └── NameLookup.hpp
│   │   └── Phase/
│   │       ├── IPhase.hpp / PhaseStack.hpp/.cpp
│   │       ├── DemoPhase.hpp/.cpp
│   │       ├── PhaseRegistry.hpp / PhaseRegistration.cpp
│   │       ├── ScenarioPhase.hpp/.cpp
│   │       └── WaitPhase.hpp/.cpp
│   ├── App/config/
│   │   ├── player.toml
│   │   └── scenario.toml
│   ├── tests/
│   └── ThirdParty/entt/entt.hpp
└── docs/
```

---

## 主要な技術

| 技術 | バージョン | 役割 |
|------|-----------|------|
| C++ | std:cpplatest | 実装言語 |
| Siv3D | v0.6.16 | ゲームフレームワーク（描画・入力・数学型） |
| EnTT | v3.16.0 | ECS（Entity Component System） |
| Catch2 | Siv3D 同梱版 | ユニットテスト |

---

## アーキテクチャ

### ゲームループ

```
Main()
  └─ System::Update() ループ
       ├─ PhaseStack::update(registry)
       │    └─ IPhase::update(registry, dt)
       ├─ AttachmentSystem::UpdateTransform(registry)  ← 親子座標伝播
       └─ DrawSystem::Draw(registry)                   ← depth ソート後に描画
```

`entt::registry` をゲーム全体で共有し、フェーズ間でエンティティの状態を引き継ぐ。

### フェーズ管理（PhaseStack / IPhase）

シーン・状態をスタック構造で管理する。

```
PhaseStack
  └─ Array<unique_ptr<IPhase>>  （末尾 = 最前面）
       ├─ IPhase（抽象）
       │   ├─ onAfterPush()
       │   ├─ update(registry, frameData) → PhaseCommand
       │   └─ onBeforePop()
       └─ PhaseCommand: None / Pop / Push / Reset
```

`update()` に渡される `FrameData` は `dt` と `InputState` をまとめた Siv3D 非依存の構造体。

### ECS（EnTT）

エンティティのデータを `entt::registry` で管理する。コンポーネントは `src/Component/` に配置。

| コンポーネント | 役割 |
|--------------|------|
| `WorldPos` | 絶対座標（w/h/d）。常に絶対値を保持 |
| `Velocity` | 速度（w/h/d、ピクセル/秒） |
| `Player` | プレイヤーを示すタグ（空構造体） |
| `Drawable` | 描画形状の variant（`RectDrawable` 等） |
| `Name` | エンティティを識別する名前（`NameLookup` と連携） |
| `Hierarchy` | 親子関係（双方向連結リスト構造で管理） |
| `LocalOffset` | 親からの相対座標（`Hierarchy` を持つエンティティに付ける） |

### 親子追従システム（Hierarchy / AttachmentSystem）

武器・エフェクトなどを親エンティティに追従させる仕組み。

- `Hierarchy` クラスにメンバを private で保持し、連結リストの操作を static メンバ関数に集約することで不整合を防ぐ
- `Hierarchy::Attach(registry, parent, child, offset)` : 子を親に O(1) で追加し `LocalOffset` も設定する
- `Hierarchy::Detach(registry, child)` : O(1) で切り離し
- `Hierarchy::DestroyWithChildren(registry, entity)` : 子孫ごと再帰破棄する（`registry.destroy()` の代わりに使う）
- `AttachmentSystem::UpdateTransform(registry)` : ルートから再帰 DFS で `WorldPos` を伝播する（毎フレーム呼び出す）

### 入力管理（Input）

Humble Object パターンで Siv3D 依存を `Main.cpp` に閉じ込める。

- `PlayerInputAction`（Siv3D 依存）: キー割り当てを保持。`toInputState()` で毎フレーム変換
- `InputState`（Siv3D 非依存）: plain `bool` の入力状態。`FrameData` に格納してフェーズへ渡す

### 設定値管理（Config）

ゲームの定数値を型付き構造体で管理。`FromToml()` で生成し `registry.ctx()` に格納してフェーズ間で共有。

### シナリオシステム（ScenarioPhase）

ゲームの進行を `scenario.toml` で管理。`NameLookup`（名前→エンティティ対応表）を `registry.ctx()` で共有。フェーズ生成は `PhaseRegistry`（ファクトリ関数テーブル）で管理し、`PhaseRegistration.cpp` への1行追記で新フェーズを追加できる。

### 描画システム（DrawSystem）

毎フレーム `DrawOrderLess`（奥 → 手前）でソートしてから描画。`Drawable` は `std::variant` で形状を表現。

### 座標系（WorldPos）

疑似3Dのワールド座標。`w`（横）・`h`（高さ、地面=0）・`d`（奥行き）の3軸。画面 y 座標は `-(d + h)`。

---

## ファイル別クラス一覧

| ファイル | クラス / 構造体 | 説明 |
|---------|---------------|------|
| `src/Component/WorldPos.hpp` | `WorldPos` | ワールド座標と画面座標変換 |
| `src/Component/Hierarchy.hpp/.cpp` | `Hierarchy` | 親子関係（双方向連結リスト、操作は static メンバ関数経由のみ） |
| `src/Component/LocalOffset.hpp` | `LocalOffset` | 親からの相対座標 |
| `src/Component/Drawable.hpp` | `RectDrawable`, `Drawable` | 描画コンポーネント（形状の variant） |
| `src/Component/Name.hpp` | `Name` | エンティティ名コンポーネント |
| `src/Component/Player.hpp` | `Player` | プレイヤータグ（空構造体） |
| `src/Component/Velocity.hpp` | `Velocity` | 速度コンポーネント |
| `src/Config/PlayerConfig.hpp/.cpp` | `PlayerConfig` | プレイヤー設定値 |
| `src/Config/ScenarioData.hpp/.cpp` | `ScenarioData` | シナリオデータ |
| `src/Input/PlayerInputAction.hpp` | `PlayerInputAction` | プレイヤー操作のキー割り当て |
| `src/Phase/IPhase.hpp` | `IPhase`, `PhaseCommand` | フェーズ基底クラスとコマンド |
| `src/Phase/PhaseStack.hpp/.cpp` | `PhaseStack` | フェーズをスタックで管理 |
| `src/Phase/PhaseRegistry.hpp` | `PhaseFactory`, `PhaseRegistry` | フェーズ名→ファクトリ関数の対応表 |
| `src/Phase/PhaseRegistration.cpp` | `MakeDefaultPhaseRegistry` | ゲーム用フェーズのファクトリ登録 |
| `src/Phase/DemoPhase.hpp/.cpp` | `DemoPhase` | プレイヤー操作デモシーン |
| `src/Phase/ScenarioPhase.hpp/.cpp` | `ScenarioPhase` | TOML シナリオ進行フェーズ |
| `src/Phase/WaitPhase.hpp/.cpp` | `WaitPhase` | 指定秒数待機してから Pop するフェーズ |
| `src/System/AttachmentSystem.hpp/.cpp` | `AttachmentSystem` | 親子座標伝播システム |
| `src/System/DrawSystem.hpp/.cpp` | `DrawSystem` | depth ソート後に Drawable を描画 |
| `src/System/NameLookup.hpp` | `NameLookup` | 名前→エンティティ参照コンテキスト |
