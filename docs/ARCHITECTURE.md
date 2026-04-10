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
│   │   ├── WorldPos.hpp        # ワールド座標
│   │   ├── Component/
│   │   │   ├── Player.hpp      # プレイヤータグ
│   │   │   └── Velocity.hpp    # 速度コンポーネント
│   │   ├── Config/
│   │   │   ├── PlayerConfig.hpp  # プレイヤー設定値
│   │   │   └── PlayerConfig.cpp  # fromTOML() 実装
│   │   ├── Input/
│   │   │   └── PlayerInputAction.hpp  # プレイヤー操作のキー割り当て
│   │   └── Scene/
│   │       ├── IPhase.hpp      # フェーズ基底クラス
│   │       ├── PhaseStack.hpp  # フェーズスタック
│   │       ├── PhaseStack.cpp
│   │       ├── DemoPhase.hpp   # プレイヤー操作デモシーン
│   │       └── DemoPhase.cpp
│   ├── App/
│   │   └── config/
│   │       └── player.toml     # プレイヤー設定ファイル
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
       └─ PhaseStack::update(registry)
            └─ IPhase::update(registry)  ← 現在の最前面フェーズ
```

`entt::registry` をゲーム全体で共有し、フェーズ間でエンティティの状態を引き継ぐ。

### フェーズ管理（PhaseStack / IPhase）

シーン・状態をスタック構造で管理する。

```
PhaseStack
  └─ Array<unique_ptr<IPhase>>  （末尾 = 最前面）
       ├─ IPhase（抽象）
       │   ├─ onAfterPush()    プッシュ直後に呼ばれる
       │   ├─ update()         毎フレーム更新（純粋仮想）→ PhaseCommand を返す
       │   └─ onBeforePop()    ポップ直前に呼ばれる
       └─ PhaseCommand
           ├─ None   継続
           ├─ Pop    自身をポップ
           ├─ Push   新フェーズをプッシュ
           └─ Reset  全クリア → 新フェーズをプッシュ
```

`IPhase` を継承してフェーズ（タイトル・ゲームプレイ等）を実装する。

### ECS（EnTT）

エンティティ（キャラクター・弾・エフェクト等）のデータを `entt::registry` で管理する。  
コンポーネントは `src/Component/` に配置する。位置は `WorldPos` を直接使用する。

| コンポーネント | 役割 |
|--------------|------|
| `WorldPos` | 位置（w/h/d） |
| `Velocity` | 速度（w/h/d、ピクセル/秒） |
| `Player` | プレイヤーを示すタグ（空構造体） |

### 入力管理（Input）

プレイヤー操作のキー割り当てを `PlayerInputAction` 構造体で管理する。

- 各アクション（moveLeft / moveRight / moveForward / moveBackward / jump 等）を Siv3D の `InputGroup` で保持
- `InputGroup` は複数のキーを OR でまとめられるため、「左矢印またはA」のような複合割り当てが可能
- `Default()` ファクトリでデフォルト割り当てを生成し、`registry.ctx()` に格納
- キーコンフィグ対応時は `PlayerInputAction` の中身を差し替えるだけでよい

```cpp
// デフォルト割り当て
auto actions = PlayerInputAction::Default();
// カスタム割り当て（将来）
actions.moveLeft = KeyLeft | KeyA | GamepadButton(0);
```

### 設定値管理（Config）

ゲームの定数値を型付き構造体（`PlayerConfig` 等）として管理する。

- 構造体メンバは plain C++ 型のみ（Siv3D 型不使用）→ TOML なしでテスト構築可能
- `fromTOML()` 静的メソッドで TOML ファイルから生成
- `registry.ctx()` に格納してフェーズ間で共有

```cpp
registry.ctx().emplace<PlayerConfig>(PlayerConfig::FromToml(toml[U"player"]));
auto& cfg = registry.ctx().get<PlayerConfig>();
```

### 座標系（WorldPos）

疑似3Dの奥行き表現に使うワールド座標。

```
struct WorldPos {
  double w;  // 横位置（画面 x に対応）
  double h;  // 高さ（地面 = 0、上方向が正）
  double d;  // 奥行き（大きいほど奥 = 画面上方）
};

Vec2 toScreen() → { w, -(d + h) }
bool isOnGround() → h <= 0.0
```

- 画面上の y 座標は `-(d + h)` で計算。奥にあるほど・高いほど画面上方に表示される。
- `DrawOrderLess(a, b)` で奥 → 手前の描画順ソートができる。
- `isOnGround()` で着地判定（重力・ジャンプ処理で使用）。

---

## ファイル別クラス一覧

| ファイル | クラス / 構造体 | 説明 |
|---------|---------------|------|
| `src/WorldPos.hpp` | `WorldPos` | ワールド座標（w, h, d）と画面座標変換 |
| `src/Component/Player.hpp` | `Player` | プレイヤータグ（空構造体） |
| `src/Component/Velocity.hpp` | `Velocity` | 速度コンポーネント（w, h, d、ピクセル/秒） |
| `src/Config/PlayerConfig.hpp/.cpp` | `PlayerConfig` | プレイヤー設定値（速度・ジャンプ・重力） |
| `src/Input/PlayerInputAction.hpp` | `PlayerInputAction` | プレイヤー操作のキー割り当て（InputGroup） |
| `src/Scene/IPhase.hpp` | `IPhase` | フェーズ基底クラス |
| `src/Scene/IPhase.hpp` | `IPhase::PhaseCommand` | フェーズスタック操作コマンド |
| `src/Scene/PhaseStack.hpp/.cpp` | `PhaseStack` | フェーズをスタックで管理 |
| `src/Scene/DemoPhase.hpp/.cpp` | `DemoPhase` | プレイヤー操作デモシーン |
