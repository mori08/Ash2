# ARCHITECTURE.md

「沼に焚べ」の設計意図をまとめたドキュメント。個々のコンポーネント・システム・フェーズ等の一覧は [REFERENCE.md](REFERENCE.md) を参照。

## プロジェクト概要

「沼に焚べ」— 疑似3D視点のベルトアクションゲーム。

| 技術 | バージョン | 役割 |
|------|-----------|------|
| C++ | std:cpplatest | 実装言語 |
| Siv3D | v0.6.16 | ゲームフレームワーク（描画・入力・数学型） |
| EnTT | v3.16.0 | ECS（Entity Component System） |
| Catch2 | Siv3D 同梱版 | ユニットテスト |

---

## ディレクトリ構成

```
Ash2/src/
├── Main.cpp             # エントリポイント・ゲームループ
├── GameSetup.hpp/.cpp   # registry 初期化・設定リロード
├── Asset.hpp            # アセット登録・パス解決ユーティリティ
├── Debug.hpp            # APP_LOG マクロ等のデバッグ用ユーティリティ
├── Component/           # ECS コンポーネント（データのみ）
├── Config/              # TOML 設定データ（FromToml 付き構造体）
├── Input/               # 入力抽象化
├── Phase/               # フェーズ管理（ゲーム状態機械）
├── System/               # ECS システム（ロジックのみ）
│   └── PlayerMotion/     # PlayerMotionSystem の状態別 Tick() 実装
└── Util/                 # フレームワーク非依存の汎用ヘルパー
```

---

## レイヤー構成

ゲームの中核は `entt::registry`（ECS）と `PhaseStack`（状態のスタック管理）の組み合わせで駆動する。
ECS がオブジェクトのデータとロジックを分離し、`PhaseStack` がゲーム状態の遷移をスタックで管理することで、
前状態への復帰や状態ごとのロジック追加を簡潔に表現できる。

```
Main.cpp
  ├── PhaseStack          ← ゲーム状態をスタックで管理
  │     └── IPhase        ← 各フェーズが ECS を操作
  ├── AttachmentSystem    ← 毎フレーム: 親子座標伝播
  ├── DrawSystem          ← 毎フレーム: ワールド描画
  └── HudSystem           ← 毎フレーム: 画面固定 HUD
```

**設計方針：**
- ECS（EnTT）でデータとロジックを分離。Component はデータのみ、System はロジックのみ。
- フェーズがゲームロジックを持ち、System は描画・座標伝播などの横断的処理を担う。
- Config / Input はフレームワーク非依存の構造体として分離し、テスト・リロードを容易にする。
- 依存方向は一方向（`Phase → System → Component/Config`）。`Component` は他レイヤーに依存しない（`Hierarchy` のみ、自身の整合性を保つため `entt::registry` を直接操作する例外）。
- 副作用の自動化はシグナルで行う（`Name` の追加・削除 → `NameLookup` 同期、`Hierarchy` の削除 → 自動 Detach）。
- 排他的な行動状態は `Motion`（`std::variant`）で表現し、`MotionSystem` が `std::visit` で状態ごとの `Tick()` にディスパッチする。状態遷移は `Tick()` の返り値 `Optional<Motion>` でのみ行う（`Tick()` 内で直接 `replace` しない）。

---

## ゲームループ（[Main.cpp](../Ash2/src/Main.cpp)）

```
while (System::Update()) {
    入力デバイス切り替え（キーボード/コントローラー自動検出）
    FrameData 生成（dt + InputState）
    設定リロード（F5、Debug ビルドのみ）
    PhaseStack::update(registry, frameData)
    AttachmentSystem::UpdateTransform(registry)
    DrawSystem::Draw(registry)
    HudSystem::Draw(registry)
}
```

ゲームプレイ系システムの呼び出し順はフェーズが所有する。標準の実行順（PlayerTestPhase、後続システムはこの順序を前提とする）：

```
HitstopSystem → MotionSystem → StaminaSystem → MovementSystem → GravitySystem
→ AttachmentSystem → HitSystem →（ヒットリアクション付与）→ ProjectileSystem
→ StaggerSystem → AnimationSystem
```

起動時に `InitializeRegistry()` が `registry.ctx()` へ以下をセット：
- `NameLookup` — 名前→エンティティの逆引きテーブル
- `PlayerConfig` — プレイヤー設定
- `AnimationDataRegistry` — アニメーション設定
- `ScenarioData` — シナリオデータ

Debug ビルドでは `Console.open()` で起動する。環境変数 `ASH2_RUN_TESTS` が設定されている場合はゲームループに入らず Catch2 のテストランナーを実行して終了する（`tools/run-tests.sh` 経由）。未捕捉の `std::exception` は `crash.log` に追記してから再 throw する。

---

## 座標系

```
WorldPos { w, h, d }
  w : 横位置（右が正）
  h : 高さ（地面=0、上が正）
  d : 奥行き（大きいほど奥 = 画面上方）

画面座標: Vec2{ w, -(d + h) }   // toScreen()
描画順:   d が大きい（奥）→ 先に描画（DrawOrderLess: a.d > b.d）
```

カメラは `Scene::Center()` の固定オフセットのみ（スクロールなし）。接地判定は `h <= 0`（`WorldPos::isOnGround()`）で、`GravitySystem` が地面への沈み込みを 0 にクランプする。

**制約：** `WorldPos` は常に絶対座標。子エンティティも `WorldPos` を持ち、`AttachmentSystem` が毎フレーム親の絶対座標 + `LocalOffset` で上書きする。

**描画・判定の基準点：** `WorldPos` は `Drawable`（`DrawAnchor`）と `Collider`（オフセット）の共通基準点だが、「中心」か「接地点」かはエンティティごとに異なる。`DrawAnchor` のデフォルトは `Center`、接地キャラクター（プレイヤー等、`Collider` を「足元からのカプセル」として持つエンティティ）は生成時に `BottomCenter` を明示する。当たり判定のカプセルは w/h/d 空間の線分 + 半径で表し、`Collider` の `Vec3` は x=w、y=h、z=d に対応する。

---

## 主要な制約

- **ビルドは `tools/build.sh`、実行は `tools/run.sh` で行う。** デバッガを使った調査はユーザーが Visual Studio 2022 で行う。
- **Hitstop 除外規約：** `Hitstop` を持つエンティティは `MotionSystem` / `MovementSystem` / `GravitySystem` / `AnimationSystem` の view から `entt::exclude` で除外される。時間依存のシステムを追加するときは同様の除外が必要か検討すること。
- **`Name` は構築後不変。** `NameLookup` が構築・破棄シグナルでのみ同期されるため。
- **アセットのパス解決は必ず `AssetPath()` を通す。** デバッグ（ファイル）とリリース（埋め込みリソース）の差を吸収する。
- このドキュメントは 200 行上限。超過する場合はコード例→実装詳細→未使用の設計説明の順で削る。
