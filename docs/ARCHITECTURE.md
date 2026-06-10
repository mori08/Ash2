# ARCHITECTURE.md

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
├── Component/           # ECS コンポーネント（データのみ）
├── Config/              # TOML 設定データ（FromToml 付き構造体）
├── Input/               # 入力抽象化
├── Phase/               # フェーズ管理（ゲーム状態機械）
└── System/              # ECS システム（ロジックのみ）
```

---

## レイヤー構成

```
Main.cpp
  ├── PhaseStack          ← ゲーム状態をスタックで管理
  │     └── IPhase        ← 各フェーズが ECS を操作
  ├── AttachmentSystem    ← 毎フレーム: 親子座標伝播
  └── DrawSystem          ← 毎フレーム: 描画
```

**設計方針：**
- ECS（EnTT）でデータとロジックを分離。Component はデータのみ、System はロジックのみ。
- フェーズがゲームロジックを持ち、System は描画・座標伝播などの横断的処理を担う。
- Config / Input はフレームワーク非依存の構造体として分離し、テスト・リロードを容易にする。

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

起動時に `InitializeRegistry()` が `registry.ctx()` へ以下をセット：
- `NameLookup` — 名前→エンティティの逆引きテーブル
- `PlayerConfig` — プレイヤー設定
- `AnimationDataRegistry` — アニメーション設定
- `ScenarioData` — シナリオデータ

Debug ビルドでは `Console.open()` で起動し、未捕捉例外をコンソールに出力して待機する。

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

**制約：** `WorldPos` は常に絶対座標。子エンティティも `WorldPos` を持ち、`AttachmentSystem` が毎フレーム親の絶対座標 + `LocalOffset` で上書きする。

**描画・判定の基準点：** `WorldPos` は `Drawable`（`DrawAnchor`）と `Collider`（オフセット）の共通基準点だが、「中心」か「接地点」かはエンティティごとに異なる。`DrawAnchor` のデフォルトは `Center`、接地キャラクター（プレイヤー等、`Collider` を「足元からのカプセル」として持つエンティティ）は生成時に `BottomCenter` を明示する。

---

## 入力抽象化

| クラス | 役割 |
|---|---|
| [`InputState`](../Ash2/src/Input/InputState.hpp) | フレームの論理入力（`Key`/`TOMLValue` 等のテストしづらい型は持ち込まないが、`Vec2` 等の単純な数学型は許容） |
| [`KeyboardInputAction`](../Ash2/src/Input/KeyboardInputAction.hpp) | キーボード/マウス → InputState 変換 |
| [`XInputAction`](../Ash2/src/Input/XInputAction.hpp) | XInput コントローラー → InputState 変換（左スティック+十字ボタンで移動、A/B/Y でジャンプ/近距離/遠距離） |

`Main.cpp` が毎フレームデバイス入力を検出し、最後にアクティブだったデバイスに切り替える。切断時はキーボードへ自動フォールバック。

**移動入力の正規化方針：** `InputState::moveAxis`（`Vec2`、x=横方向/y=奥行き方向）は「常に長さ 1.0 以下に正規化済み」という不変条件を持つ。この保証の責任は `toInputState()` を実装する各入力レイヤー側にあり、`PlayerMovementSystem` は無条件にこの値を信頼してそのまま速度計算に使う（System 側で正規化やクランプを行わない）。`XInputAction` は左スティックにデッドゾーン処理（`DeadZone`）を適用し、十字ボタンの軸ベクトルと加算したうえで `limitLength(1.0)` により正規化する。

---

## コンポーネント一覧

| コンポーネント | 役割 |
|---|---|
| [`WorldPos`](../Ash2/src/Component/WorldPos.hpp) | ワールド絶対座標（w/h/d） |
| [`Velocity`](../Ash2/src/Component/Velocity.hpp) | 速度ベクトル（w/h/d、ピクセル/秒） |
| [`LocalOffset`](../Ash2/src/Component/LocalOffset.hpp) | 親からの相対座標（Hierarchy 付きエンティティのみ） |
| [`Hierarchy`](../Ash2/src/Component/Hierarchy.hpp) | 親子関係（双方向連結リスト、static メンバで操作） |
| [`Drawable`](../Ash2/src/Component/Drawable.hpp) | 描画形状（`variant<RectDrawable/CircleDrawable/PieDrawable/TextureDrawable>`）。`RectDrawable`/`TextureDrawable` は `DrawAnchor` で `WorldPos` の合わせ位置（`Center`/`BottomCenter`）を指定する |
| [`SpriteAnimation`](../Ash2/src/Component/SpriteAnimation.hpp) | アニメーション再生状態（per-entity） |
| [`Name`](../Ash2/src/Component/Name.hpp) | エンティティ名（不変、NameLookup と対応） |
| [`Player`](../Ash2/src/Component/Player.hpp) | プレイヤータグ（データなし） |
| [`Collider`](../Ash2/src/Component/Collider.hpp) | カプセル形状の当たり判定（形状のみ、役割はコンポーネントの組み合わせで表現） |
| [`Attack`](../Ash2/src/Component/Attack.hpp) | 攻撃中タグ兼攻撃力（`Collider` と組み合わせて攻撃判定が有効になる）。`root` で複数コライダー構成のルートを指定し、`hitTargets` で攻撃生存期間中の重複ヒットを防ぐ |
| [`Hp`](../Ash2/src/Component/Hp.hpp) | HP（`Collider` と組み合わせて被弾判定の対象になる） |
| [`Stamina`](../Ash2/src/Component/Stamina.hpp) | スタミナ（max / current の int フィールド） |
| [`Projectile`](../Ash2/src/Component/Projectile.hpp) | 飛翔体（弾）タグ（データなし）。`WorldPos`+`Velocity`+`Collider`+`Attack` と組み合わせ、`ProjectileSystem` が移動・消滅を管理する対象を識別する |

---

## フェーズシステム

`PhaseStack` がスタックで `IPhase` を管理。各フレームで先頭フェーズの `update()` を呼び、返り値の `PhaseCommand` でスタックを操作する。

| PhaseCommand | 動作 |
|---|---|
| `None` | 何もしない |
| `Pop` | 先頭フェーズを取り出す |
| `Push(phase)` | 新フェーズを積む |
| `Reset(phase)` | スタックを全クリアして新フェーズを積む |

### 実装済みフェーズ

| フェーズ | 役割 |
|---|---|
| [`ScenarioPhase`](../Ash2/src/Phase/ScenarioPhase.hpp) | TOML シナリオを 1 ステップずつ実行（push/reset） |
| [`TestMenuPhase`](../Ash2/src/Phase/TestMenuPhase.hpp) | テストフェーズ一覧メニュー（↑↓選択、Enter で Push） |
| [`PlayerTestPhase`](../Ash2/src/Phase/PlayerTestPhase.hpp) | プレイヤー操作・物理・アニメーションのビジュアルテスト |
| [`AnimationViewerPhase`](../Ash2/src/Phase/AnimationViewerPhase.hpp) | アニメーションクリップ単体確認（←→切替、F反転） |
| [`WaitPhase`](../Ash2/src/Phase/WaitPhase.hpp) | 指定秒数待機して Pop |

[`Config/ScenarioData`](../Ash2/src/Config/ScenarioData.hpp) がシナリオロード時に各ステップを `IPhaseMaker`（型消去された `make() -> unique_ptr<IPhase>`）を持つ `ScenarioStep`（`StepPush`/`StepReset`）に変換する。
変換テーブルは [`Config/ScenarioData.cpp`](../Ash2/src/Config/ScenarioData.cpp) の `kPhaseLoaders` で定義されており、**新フェーズ追加時はここにもエントリを追加する必要がある。**

---

## システム一覧

| システム | タイミング | 処理 |
|---|---|---|
| [`AttachmentSystem::UpdateTransform`](../Ash2/src/System/AttachmentSystem.hpp) | 毎フレーム（フェーズ後） | Hierarchy ルートから子孫へ WorldPos 伝播 |
| [`DrawSystem::Draw`](../Ash2/src/System/DrawSystem.hpp) | 毎フレーム（最後） | WorldPos+Drawable を奥行き順にソートして描画 |
| [`AnimationSystem::Update`](../Ash2/src/System/AnimationSystem.hpp) | フェーズ内（各フェーズが直接呼出） | SpriteAnimation の elapsed を進め Drawable を更新 |
| [`NameLookupSystem::Connect`](../Ash2/src/System/NameLookup.hpp) | 起動時 | Name 追加・削除時に NameLookup を自動同期するシグナル登録 |
| [`HierarchySystem::Connect`](../Ash2/src/System/HierarchySystem.hpp) | 起動時 | Hierarchy 削除時に Detach を自動呼び出しするシグナル登録 |
| [`HitSystem::Update`](../Ash2/src/System/HitSystem.hpp) | フェーズ内（攻撃入力時） | `Collider+Attack` と `Collider+Hp` の間でカプセル重なり検出 → Hp 減算 |
| [`PlayerMovementSystem::Update`](../Ash2/src/System/PlayerMovementSystem.hpp) | フェーズ内（PlayerTestPhase） | Player の移動・ジャンプ・重力・クリップ選択・向き更新 |
| [`ProjectileSystem::Update`](../Ash2/src/System/ProjectileSystem.hpp) | フェーズ内（弾が存在する間、毎フレーム） | Projectile の移動、着弾（hitTargets 非空）/ 画面外での破棄 |
| [`HudSystem::Draw`](../Ash2/src/System/HudSystem.hpp) | 毎フレーム（DrawSystem の後） | Player の Hp / Stamina を画面左上にゲージ描画 |

---

## アセット管理（[Asset.hpp](../Ash2/src/Asset.hpp)）

- デバッグ: `assets/asset_list` をファイルから読む
- リリース: `assets/asset_list` を埋め込みリソースから読む
- `.png` → `TextureAsset`、`.mp3` → `AudioAsset` としてキー（相対パス）で登録
- アニメーション設定: `assets/config/animation/*.toml`（起動時に全ファイルをスキャン）

---

## 主要な制約

- **ビルドは `tools/build.sh`、実行は `tools/run.sh` で行う。** デバッガを使った調査はユーザーが Visual Studio 2022 で行う。
- `Hierarchy` のメンバは必ず static メンバ関数（Attach/Detach/DestroyWithChildren）経由で操作する（不整合防止）。
- `Drawable` の型変更は `std::visit` を使い、DrawSystem と AnimationSystem の両方への影響を確認する。
- 新クラス追加時は `Ash2.vcxproj` と `Ash2.vcxproj.filters` にも追加が必要。
- 新フェーズ追加時は `Config/ScenarioData.cpp` の `kPhaseLoaders` にもエントリを追加する必要がある。
- `NameLookup` への挿入・削除は `NameLookupSystem::Connect` で自動化されている（`Name` コンポーネントの追加・削除に連動）。手動での `NameLookup[key] = entity` 登録は不要。
- このドキュメントは 200 行上限。超過する場合はコード例→実装詳細→未使用の設計説明の順で削る。
