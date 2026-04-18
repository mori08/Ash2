# ARCHITECTURE.md

## 概要

「沼に焚べ」の技術構成とコード設計をまとめたドキュメント。  
実装の進捗に合わせて随時更新する。

---

## ディレクトリ構成

```
Ash2/
├── Ash2/
│   ├── src/                    # ゲームソースコード
│   │   ├── Main.cpp            # エントリーポイント
│   │   ├── stdafx.h            # プリコンパイルヘッダ（Siv3D 一括インクルード）
│   │   ├── Component/
│   │   │   ├── Drawable.hpp    # 描画コンポーネント（variant）
│   │   │   ├── Name.hpp        # エンティティ名コンポーネント
│   │   │   ├── Player.hpp      # プレイヤータグ
│   │   │   ├── Velocity.hpp    # 速度コンポーネント
│   │   │   └── WorldPos.hpp    # ワールド座標
│   │   ├── Config/
│   │   │   ├── PlayerConfig.hpp   # プレイヤー設定値
│   │   │   ├── PlayerConfig.cpp
│   │   │   ├── ScenarioData.hpp   # シナリオデータ（ロード済みステップ一覧）
│   │   │   └── ScenarioData.cpp
│   │   ├── Input/
│   │   │   └── PlayerInputAction.hpp  # プレイヤー操作のキー割り当て
│   │   ├── System/
│   │   │   ├── DrawSystem.hpp  # 描画システム
│   │   │   ├── DrawSystem.cpp
│   │   │   └── NameLookup.hpp  # 名前→エンティティ参照コンテキスト
│   │   └── Phase/
│   │       ├── IPhase.hpp          # フェーズ基底クラス
│   │       ├── PhaseStack.hpp      # フェーズスタック
│   │       ├── PhaseStack.cpp
│   │       ├── DemoPhase.hpp       # プレイヤー操作デモシーン
│   │       ├── DemoPhase.cpp
│   │       ├── PhaseRegistry.hpp   # フェーズ名→ファクトリ関数の対応表
│   │       ├── ScenarioPhase.hpp   # TOML シナリオ進行フェーズ
│   │       ├── ScenarioPhase.cpp
│   │       ├── WaitPhase.hpp       # 指定秒数待機フェーズ
│   │       └── WaitPhase.cpp
│   ├── App/
│   │   └── config/
│   │       ├── player.toml     # プレイヤー設定ファイル
│   │       └── scenario.toml   # シナリオ定義ファイル
│   ├── tests/
│   │   ├── TestWorldPos.cpp    # WorldPos ユニットテスト
│   │   ├── TestPlayerConfig.cpp # PlayerConfig ユニットテスト
│   │   └── standalone/main.cpp # スタンドアロンテスト実行環境
│   └── ThirdParty/
│       └── entt/entt.hpp       # EnTT v3.16.0（ECS ライブラリ）
└── docs/
    ├── GIT.md
    ├── TEST.md
    └── ARCHITECTURE.md         # このファイル
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
       ├─ PhaseStack::update(registry)          ← Scene::DeltaTime() を取得
       │    └─ IPhase::update(registry, dt)     ← 現在の最前面フェーズ
       └─ DrawSystem::draw(registry)            ← 描画（depth ソート後）
```

`entt::registry` をゲーム全体で共有し、フェーズ間でエンティティの状態を引き継ぐ。

### フェーズ管理（PhaseStack / IPhase）

シーン・状態をスタック構造で管理する。

```
PhaseStack
  └─ Array<unique_ptr<IPhase>>  （末尾 = 最前面）
       ├─ IPhase（抽象）
       │   ├─ onAfterPush()        プッシュ直後に呼ばれる
       │   ├─ update(registry, dt) 毎フレーム更新（純粋仮想）→ PhaseCommand を返す
       │   └─ onBeforePop()        ポップ直前に呼ばれる
       └─ PhaseCommand
           ├─ None   継続
           ├─ Pop    自身をポップ
           ├─ Push   新フェーズをプッシュ
           └─ Reset  全クリア → 新フェーズをプッシュ
```

`IPhase` を継承してフェーズ（タイトル・ゲームプレイ等）を実装する。

### ECS（EnTT）

エンティティ（キャラクター・弾・エフェクト等）のデータを `entt::registry` で管理する。  
コンポーネントはすべて `src/Component/` に配置する。位置は `WorldPos` を直接使用する。

| コンポーネント | 役割 |
|--------------|------|
| `WorldPos` | 位置（w/h/d） |
| `Velocity` | 速度（w/h/d、ピクセル/秒） |
| `Player` | プレイヤーを示すタグ（空構造体） |
| `Drawable` | 描画形状の variant（`RectDrawable` 等） |
| `Name` | エンティティを識別する名前（`NameLookup` と連携） |

### 入力管理（Input）

プレイヤー操作のキー割り当てを `PlayerInputAction` 構造体で管理する。

- 各アクション（moveLeft / moveRight / moveForward / moveBackward / jump 等）を Siv3D の `InputGroup` で保持
- `InputGroup` は複数のキーを OR でまとめられるため、「左矢印またはA」のような複合割り当てが可能
- `Default()` ファクトリでデフォルト割り当てを生成し、`registry.ctx()` に格納
- キーコンフィグ対応時は `PlayerInputAction` の中身を差し替えるだけでよい

### 設定値管理（Config）

ゲームの定数値を型付き構造体（`PlayerConfig` 等）として管理する。

- `FromToml()` 静的メソッドで TOML ファイルから生成
- `registry.ctx()` に格納してフェーズ間で共有

### シナリオシステム（ScenarioPhase）

ゲームの進行を `scenario.toml` で管理する。再コンパイル不要でシナリオを編集できる。

- `ScenarioData::FromToml()` でロード時に全セクションを `HashTable<String, Array<TOMLValue>>` へ変換し `registry.ctx()` に格納
- `ScenarioPhase` は `IPhase` を継承し、1フレームに1ステップを処理する
- エンティティを名前で参照するため `NameLookup`（`HashTable<String, entity>`）を `registry.ctx()` で共有
- `ScenarioPhase::onBeforePop()` でこのフェーズが生成したエンティティと `NameLookup` エントリを削除

フェーズ生成は `PhaseRegistry`（`HashTable<String, PhaseFactory>`）で管理する。
各フェーズクラスが `fromToml()` で自身のインスタンス化を担い、`ScenarioPhase` は他フェーズを `#include` しない。
`Main.cpp` で登録を行い、新しいフェーズの追加は `fromToml()` の実装と登録のみで完結する。

TOML の `action` フィールドはスタック操作を表し、`phase` フィールドは `PhaseRegistry` のキーを指定する：

| `action` | `phase` フィールド | 処理 |
|---------|------------------|------|
| `push` | フェーズ名（`wait` 等） | `PhaseRegistry` でフェーズを生成して Push |
| `reset` | フェーズ名（`scenario` 等） | `PhaseRegistry` でフェーズを生成して Reset |
| `make` | — | エンティティを生成し `Name`・`WorldPos`・`Drawable` を付与 |

### 描画システム（DrawSystem）

`Drawable` コンポーネントは描画形状の `std::variant` で、現在 `RectDrawable`（矩形）に対応している。
形状を追加する場合は `Drawable` に型を追加し、`DrawSystem` の `std::visit` にハンドラを追記する。

毎フレーム、`WorldPos` を持つエンティティを `DrawOrderLess`（奥 → 手前）でソートしてから描画することで、疑似3Dの前後関係を再現する。

### 座標系（WorldPos）

疑似3Dの奥行き表現に使うワールド座標。`w`（横）・`h`（高さ、地面=0）・`d`（奥行き）の3軸。

- 画面 y 座標は `-(d + h)`。奥にあるほど・高いほど画面上方に表示される。
- `DrawOrderLess(a, b)` で奥 → 手前の描画順ソートができる。
- `isOnGround()` で着地判定（重力・ジャンプ処理で使用）。

---

## ファイル別クラス一覧

| ファイル | クラス / 構造体 | 説明 |
|---------|---------------|------|
| `src/Component/WorldPos.hpp` | `WorldPos` | ワールド座標（w, h, d）と画面座標変換 |
| `src/Component/Drawable.hpp` | `RectDrawable`, `Drawable` | 描画コンポーネント（形状の variant） |
| `src/Component/Name.hpp` | `Name` | エンティティ名コンポーネント |
| `src/Component/Player.hpp` | `Player` | プレイヤータグ（空構造体） |
| `src/Component/Velocity.hpp` | `Velocity` | 速度コンポーネント（w, h, d、ピクセル/秒） |
| `src/Config/PlayerConfig.hpp/.cpp` | `PlayerConfig` | プレイヤー設定値（速度・ジャンプ・重力） |
| `src/Config/ScenarioData.hpp/.cpp` | `ScenarioData` | シナリオデータ（ロード済みステップ一覧） |
| `src/Input/PlayerInputAction.hpp` | `PlayerInputAction` | プレイヤー操作のキー割り当て（InputGroup） |
| `src/Phase/IPhase.hpp` | `IPhase` | フェーズ基底クラス |
| `src/Phase/IPhase.hpp` | `IPhase::PhaseCommand` | フェーズスタック操作コマンド |
| `src/Phase/PhaseStack.hpp/.cpp` | `PhaseStack` | フェーズをスタックで管理 |
| `src/Phase/PhaseRegistry.hpp` | `PhaseFactory`, `PhaseRegistry` | フェーズ名→ファクトリ関数の対応表（型エイリアス） |
| `src/Phase/DemoPhase.hpp/.cpp` | `DemoPhase` | プレイヤー操作デモシーン |
| `src/Phase/ScenarioPhase.hpp/.cpp` | `ScenarioPhase` | TOML シナリオ進行フェーズ |
| `src/Phase/WaitPhase.hpp/.cpp` | `WaitPhase` | 指定秒数待機してから Pop するフェーズ |
| `src/System/DrawSystem.hpp/.cpp` | `DrawSystem` | depth ソート後に Drawable を描画 |
| `src/System/NameLookup.hpp` | `NameLookup` | 名前→エンティティ参照コンテキスト（型エイリアス） |
