# REFERENCE.md

「沼に焚べ」の部品一覧。設計意図・レイヤー構成・座標系などは [ARCHITECTURE.md](ARCHITECTURE.md) を参照。

行数上限なし。機能追加のたびに更新する前提のドキュメント。

---

## 入力抽象化

| クラス | 役割 |
|---|---|
| [`InputState`](../Ash2/src/Input/InputState.hpp) | フレームの論理入力（`Key`/`TOMLValue` 等のテストしづらい型は持ち込まないが、`Vec2` 等の単純な数学型は許容） |
| [`KeyboardInputAction`](../Ash2/src/Input/KeyboardInputAction.hpp) | キーボード/マウス → InputState 変換（デフォルト: 矢印/WASD 移動、Space ジャンプ、左クリック近距離、右クリック遠距離、Shift ダッシュ、F5 設定リロード） |
| [`XInputAction`](../Ash2/src/Input/XInputAction.hpp) | XInput コントローラー → InputState 変換（左スティック+十字ボタンで移動、A/B/X/Y でジャンプ/近距離/遠距離/ダッシュ） |

[`InputDeviceSelector`](../Ash2/src/Input/InputDeviceSelector.hpp) が毎フレームデバイス入力を検出し、最後にアクティブだったデバイスに切り替える（ボタン入力に加え、左スティックがデッドゾーンを超えて傾いた場合もゲームパッドへの切り替え条件とする）。切断時はキーボードへ自動フォールバック。

**移動入力の正規化方針：** `InputState::moveAxis`（`Vec2`、x=横方向/y=奥行き方向）は「常に長さ 1.0 以下に正規化済み」という不変条件を持つ。この保証の責任は `toInputState()` を実装する各入力レイヤー側にあり、`PlayerMotion::Tick(Neutral&, ...)` は無条件にこの値を信頼してそのまま速度計算に使う（System 側で正規化やクランプを行わない）。`XInputAction` は左スティックのデッドゾーン定数（`LeftThumbDeadZone`、`InputDeviceSelector` も参照する公開 `static constexpr` メンバ）を適用し、十字ボタンの軸ベクトルと加算したうえで `limitLength(1.0)` により正規化する。

---

## コンポーネント一覧

| コンポーネント | 役割 |
|---|---|
| [`WorldPos`](../Ash2/src/Component/WorldPos.hpp) | ワールド絶対座標（w/h/d）。`toScreen()`・`isOnGround()` を持つ |
| [`Velocity`](../Ash2/src/Component/Velocity.hpp) | 速度ベクトル（w/h/d、ピクセル/秒） |
| [`LocalOffset`](../Ash2/src/Component/LocalOffset.hpp) | 親からの相対座標（Hierarchy 付きエンティティのみ） |
| [`Hierarchy`](../Ash2/src/Component/Hierarchy.hpp) | 親子関係（双方向連結リスト、static メンバで操作） |
| [`Drawable`](../Ash2/src/Component/Drawable.hpp) | 描画形状（`variant<RectDrawable/CircleDrawable/PieDrawable/TextureDrawable>`）。`RectDrawable`/`TextureDrawable` は `DrawAnchor` で `WorldPos` の合わせ位置（`Center`/`BottomCenter`）を指定する |
| [`SpriteAnimation`](../Ash2/src/Component/SpriteAnimation.hpp) | アニメーション再生状態（per-entity）。共有データは `AnimationDataRegistry` を `dataKey` で参照する |
| [`Name`](../Ash2/src/Component/Name.hpp) | エンティティ名（不変、NameLookup と対応） |
| [`Player`](../Ash2/src/Component/Player.hpp) | プレイヤータグ（データなし） |
| [`Collider`](../Ash2/src/Component/Collider.hpp) | カプセル形状の当たり判定（形状のみ、役割はコンポーネントの組み合わせで表現） |
| [`Attack`](../Ash2/src/Component/Attack.hpp) | 攻撃中タグ兼攻撃力（`Collider` と組み合わせて攻撃判定が有効になる）。フィールドの詳細は下記「複雑なコンポーネントの詳細」参照 |
| [`Hp`](../Ash2/src/Component/Hp.hpp) | HP（`Collider` と組み合わせて被弾判定の対象になる） |
| [`Stamina`](../Ash2/src/Component/Stamina.hpp) | スタミナ（max / current の int フィールド、StaminaSystem が管理する回復端数累積 accum と回復ディレイ計測用 recoveryTimer を持つ） |
| [`Projectile`](../Ash2/src/Component/Projectile.hpp) | 飛翔体（弾）タグ（データなし）。`WorldPos`+`Velocity`+`Collider`+`Attack` と組み合わせ、`MovementSystem` が移動を、`ProjectileSystem` が消滅を管理する対象を識別する |
| [`Motion`](../Ash2/src/Component/Motion.hpp) | エンティティの排他的な行動状態（`variant`）。状態ごとの詳細は下記「複雑なコンポーネントの詳細」参照 |
| [`Gravity`](../Ash2/src/Component/Gravity.hpp) | 重力の影響を受けるエンティティに付与する重力加速度 |
| [`Hitstop`](../Ash2/src/Component/Hitstop.hpp) | ヒットストップ中であることを示す残り時間タイマー。`HitstopSystem` が減算・除去し、付与中は `MotionSystem`/`MovementSystem`/`GravitySystem`/`AnimationSystem` の対象から除外される（暫定実装、本格化は #132/#134） |
| [`Stagger`](../Ash2/src/Component/Stagger.hpp) | ひるみリアクション中であることを示すタイマー。`StaggerSystem` が `RectDrawable::size` を縮小させ、残り時間が尽きたら `originalSize` に戻す（暫定実装、本格化は #134） |
| [`Invincible`](../Ash2/src/Component/Invincible.hpp) | 無敵状態であることを示すタグ。`HitSystem` の被弾対象ビューから除外される。`PlayerMotion::Dash`（地上・空中いずれも）が構え・ダッシュ中は毎フレーム付与し、後隙入りで除去する |

### 複雑なコンポーネントの詳細

#### `Attack`

攻撃中タグ兼攻撃力。`Collider` と組み合わせて攻撃判定が有効になる。

- `root`: 複数コライダー構成のルートを指定する
- `hitTargets`: 攻撃生存期間中の重複ヒットを防ぐ
- `hitstopSec`: ヒット成立時に攻撃側・被弾側へ付与するヒットストップ時間

#### `Motion`

エンティティの排他的な行動状態（`std::variant<PlayerMotion::Neutral, Melee, Ranged, Dash, DashAttack, AirAttack, Landing>`）。`Dash`/`DashAttack` は `air` フラグ1つで地上・空中の両方を表す（#207 で型を統合、旧 `AirDash`/`AirDashAttack` は廃止）。4区間タイムライン（構え/攻撃/後隙A/後隙B）を持つ状態は `Config/PlayerConfig.hpp` の `MotionTimeline`（`isActive`/`isCancelable`/`isFinished`/`activeProgress`）で区間判定する。

- **Ranged**: 再生中クリップの残り時間を持つ。`Neutral` から接地・空中いずれでも入場できる（空中発動時も専用の遷移や着地処理は持たず、Neutral 復帰までの挙動は地上と同一）
- **Melee**: コンボ段インデックス（`stage`）を持つ単一の型。段ごとの設定は `cfg.melee.stages[stage]`（`MeleeStageConfig`、軌道は `MeleeTrajectory::Thrust`/`Slash`）を参照する。モーション開始からの経過時間（`elapsed`）・攻撃判定の子エンティティ（`hitboxEntity`）を持つ。次段が存在する段（1・2段目相当）は次段への遷移予約フラグ（`comboQueued`）と後隙中のダッシュキャンセルを持つが、次段を持たない最終段（締め技、3段目相当）はコンボ継続・ダッシュキャンセルのいずれも持たずタイマー満了で `Neutral` へ戻るのみ。各段の `MotionTimeline` は `recoveryASec=0` として後隙全体をキャンセル可能区間として扱う（Melee は元々後隙A/B分割を持たないため）
- **Dash**: `air` フラグで地上・空中を切り替える。構え・ダッシュ・後隙A・後隙Bの4区間を `elapsed` 1本で管理する。ダッシュ中・後隙A・B中の攻撃入力（`attackDown`）でダッシュ攻撃を予約（`dashAttackQueued`）し、後隙B中に `DashAttack`（`air` を引き継ぐ）へ遷移する。後隙B中のダッシュ入力（`dashDown`）は再ダッシュにキャンセルする（`dashQueued`、地上・空中共通の仕様）。ダッシュ移動中の方向を `lastDashDir` に記録し `DashAttack::dashDir` へ引き渡す。`air=true` の場合のみ、構え・ダッシュ中は移動区間中の垂直速度を 0 に固定して重力の影響を受けず（暫定仕様）、`Invincible` を構え・ダッシュ中に付与して後隙入りで除去し、後隙中も含め毎フレーム接地を検出して接地した時点で（`Invincible` を除去したうえで）`Landing` へ強制遷移する。`air=false`（地上）の場合はこれらの `air` 専用分岐を通らず、接地遷移も持たない
- **DashAttack**: `air` フラグで地上・空中を切り替える。構え・攻撃・後隙の3区間（`recoveryBSec=0` として即 Neutral 復帰）を持ち、攻撃判定（`hitboxEntity`）を w-d 平面の円軌道上で更新する。`air=true` の場合のみ、突進フェーズ中の垂直速度を 0 に固定し、後隙中も含め毎フレーム接地を検出して接地した時点で（残っていればヒットボックスを破棄したうえで）`Landing` へ強制遷移する。`air=false`（地上）の場合は接地遷移を持たない。リアクション Lv2・スタミナ枯渇時の威力低下は未実装（暫定のダメージ+ヒットストップのみ、本格対応は #134 のスコープ）
- **AirAttack**: `Neutral` が空中（`!WorldPos::isOnGround()`）で攻撃入力を受けたときに入場する。地上に対応する型を持たない唯一の空中専用状態。構え・攻撃・後隙の3区間（`recoveryBSec=0`）を持ち、攻撃判定（`hitboxEntity`）を w-h 平面（垂直面）の円軌道上で更新する。後隙中も含め毎フレーム接地を検出し、接地した時点で（残っていればヒットボックスを破棄したうえで）`Landing` へ強制遷移する。接地せずに後隙が満了した場合はタイマー満了で `Neutral` へ戻る。リアクション Lv2・スタミナ枯渇時の威力低下は `DashAttack` 同様に未実装（暫定のダメージ+ヒットストップのみ、本格対応は #134 のスコープ）
- **Landing**: 着地硬直のタイマー状態。空中アクションの接地検出から遷移する

---

## フェーズシステム

`PhaseStack` がスタックで `IPhase` を管理。各フレームで先頭フェーズの `update()` を呼び、返り値の `PhaseCommand` でスタックを操作する。

### 基盤

| クラス | 役割 |
|---|---|
| [`IPhase`](../Ash2/src/Phase/IPhase.hpp) | フェーズ基底クラス。`onAfterPush` / `update` / `onBeforePop` と `PhaseCommand` を定義する |
| [`PhaseStack`](../Ash2/src/Phase/PhaseStack.hpp) | フェーズのスタック管理。最前面のみ更新し、`PhaseCommand` に従い操作する |
| [`FrameData`](../Ash2/src/Phase/FrameData.hpp) | フレームごとの更新データ（`dt` + `InputState`） |

| PhaseCommand | 動作 |
|---|---|
| `None` | 何もしない |
| `Pop` | 先頭フェーズを取り出す |
| `Push(phase)` | 新フェーズを積む |
| `Reset(phase)` | スタックを全クリアして新フェーズを積む |

### 実装済みフェーズ

| フェーズ | TOML 名 | 役割 |
|---|---|---|
| [`ScenarioPhase`](../Ash2/src/Phase/ScenarioPhase.hpp) | `scenario` | TOML シナリオを 1 ステップずつ実行（push/reset）。起動時の最初のフェーズ（`init` セクション） |
| [`TestMenuPhase`](../Ash2/src/Phase/TestMenuPhase.hpp) | `test_menu` | テストフェーズ一覧メニュー（↑↓選択、Enter で Push） |
| [`PlayerTestPhase`](../Ash2/src/Phase/PlayerTestPhase.hpp) | `player_test` | プレイヤー操作・物理・アニメーションのビジュアルテスト。`HitSystem::Update` が返す `HitPair` を見て攻撃側・被弾側へ `Hitstop`/`Stagger` を付与する（暫定実装）。F5 でプレイヤー再生成、Esc で Pop |
| [`AnimationViewerPhase`](../Ash2/src/Phase/AnimationViewerPhase.hpp) | `animation_viewer` | アニメーションクリップ単体確認（←→切替、F反転、Esc で Pop） |
| [`WaitPhase`](../Ash2/src/Phase/WaitPhase.hpp) | `wait` | 指定秒数待機して Pop |

[`Config/ScenarioData`](../Ash2/src/Config/ScenarioData.hpp) がシナリオロード時に各ステップを `IPhaseMaker`（型消去された `make() -> unique_ptr<IPhase>`）を持つ `ScenarioStep`（`StepPush`/`StepReset`）に変換する。
変換テーブルは [`Config/ScenarioData.cpp`](../Ash2/src/Config/ScenarioData.cpp) の `kPhaseLoaders` で定義されており、**新フェーズ追加時はここにもエントリを追加する必要がある。**

---

## システム一覧

| システム | タイミング | 処理 |
|---|---|---|
| [`AttachmentSystem::UpdateTransform`](../Ash2/src/System/AttachmentSystem.hpp) | 毎フレーム（フェーズ後）＋フェーズ内（PlayerTestPhase、GravitySystem の後・HitSystem の前） | Hierarchy ルートから子孫へ WorldPos 伝播。PlayerTestPhase では HitSystem が同フレーム内の最新座標（光の珠の LocalOffset 反映後）を見られるよう追加で呼び出す |
| [`DrawSystem::Draw`](../Ash2/src/System/DrawSystem.hpp) | 毎フレーム（HudSystem の前） | WorldPos+Drawable を奥行き順にソートして描画 |
| [`AnimationSystem::Update`](../Ash2/src/System/AnimationSystem.hpp) | フェーズ内（各フェーズが直接呼出） | `Hitstop` を持たない SpriteAnimation の elapsed を進め Drawable を更新 |
| [`NameLookupSystem::Connect`](../Ash2/src/System/NameLookup.hpp) | 起動時 | Name 追加・削除時に NameLookup を自動同期するシグナル登録 |
| [`HierarchySystem::Connect`](../Ash2/src/System/HierarchySystem.hpp) | 起動時 | Hierarchy 削除時に Detach を自動呼び出しするシグナル登録 |
| [`HitSystem::Update`](../Ash2/src/System/HitSystem.hpp) | フェーズ内（攻撃入力時） | `Collider+Attack` と `Collider+Hp`（`Invincible` を除く）の間でカプセル重なり検出 → Hp 減算。新たに成立したヒットの `HitPair`（attacker/target）配列を返す |
| [`HitstopSystem::Update`](../Ash2/src/System/HitstopSystem.hpp) | フェーズ内（PlayerTestPhase、MotionSystem の前） | `Hitstop` を持つエンティティの残り時間を減算し、0 以下になったら除去する（暫定実装） |
| [`MotionSystem::Update`](../Ash2/src/System/MotionSystem.hpp) | フェーズ内（PlayerTestPhase、HitstopSystem の後） | `Hitstop` を持たない `Motion` の全状態（Neutral/Melee/Ranged/Dash/DashAttack/AirAttack/Landing の7状態、`Dash`/`DashAttack` は `air` フラグで地上・空中を兼ねる）ごとの `Tick()` を呼び、移動・ジャンプ・向き・クリップ決定・状態遷移（攻撃判定/弾エンティティ生成、タイマー満了、無敵の付与/除去、接地検出）を行う。各状態の `Tick()` 実体は [`PlayerMotionSystem.cpp`](../Ash2/src/System/PlayerMotionSystem.cpp) にある。デバッグビルドでは遷移を `APP_LOG` に出力する |
| [`MovementSystem::Update`](../Ash2/src/System/MovementSystem.hpp) | フェーズ内（PlayerTestPhase、MotionSystem の後） | `Hitstop` を持たない `WorldPos`+`Velocity` エンティティ（Player・弾）の位置を `vel * dt` で更新 |
| [`GravitySystem::Update`](../Ash2/src/System/GravitySystem.hpp) | フェーズ内（PlayerTestPhase、MovementSystem の後） | `Hitstop` を持たない `WorldPos`+`Velocity`+`Gravity` エンティティに重力加速（次フレーム用）と地面クランプ（今フレームの `pos.h` を 0 にする）を適用 |
| [`ProjectileSystem::Update`](../Ash2/src/System/ProjectileSystem.hpp) | フェーズ内（弾が存在する間、毎フレーム） | Projectile の着弾（hitTargets 非空）/ 画面外での破棄 |
| [`StaggerSystem::Update`](../Ash2/src/System/StaggerSystem.hpp) | フェーズ内（PlayerTestPhase、HitSystem の後） | `Stagger` を持つエンティティの残り時間を減算し `RectDrawable::size` を縮小、0 以下で `originalSize` に戻して除去する（暫定実装） |
| [`StaminaSystem::Update`](../Ash2/src/System/StaminaSystem.hpp) | フェーズ内（PlayerTestPhase、MotionSystem の後） | `Player + Stamina + Motion` を持つエンティティのスタミナを回復する。Neutral 状態のみ `recoveryDelay` 秒の待機後に不足分に比例した速度（`recoveryRate`）で回復し、端数は `accum` に積み立てて誤差を防ぐ |
| [`HudSystem::Draw`](../Ash2/src/System/HudSystem.hpp) | 毎フレーム（DrawSystem の後） | Player の Hp / Stamina を画面左上にゲージ描画（プレイヤー 1 体のみ想定） |

---

## 設定（Config）

`Ash2/src/Config/` 配下。TOML から `FromToml()` で構築し、`registry.ctx()` に格納する。

### [`PlayerConfig`](../Ash2/src/Config/PlayerConfig.hpp)

- `assets/config/player.toml` から読み込むプレイヤー設定
- 基本値（移動速度・ジャンプ初速・重力）と、`MeleeConfig` / `RangedConfig` / `DashConfig` / `DashAttackConfig` / `AirAttackConfig` / `StaminaConfig` / `LandingConfig` の各サブ設定を持つ
- 地上・空中を共有する `Dash`/`DashAttack`（`air` フラグで区別）は、それぞれ単一の `DashConfig`/`DashAttackConfig` を共通で参照する（専用設定は持たない）
- `MotionTimeline`（windup/active/recoveryA/recoveryB の4区間、`isActive`/`isCancelable`/`isFinished`/`activeProgress` を持つ）を `DashConfig`/`DashAttackConfig`/`AirAttackConfig`/`MeleeStageConfig` が共通で持つ
- `MeleeConfig` は段共通のパラメータ（`capMidH`/`reach`/`damage`）と、コンボ段ごとの `MeleeStageConfig`（`timeline`/`radius`/`trajectory`/`slashRiseHeight`）の配列 `stages` を持つ。`trajectory` は `MeleeTrajectory::Thrust`（突き出し）/`Slash`（斬り上げ）

### [`AnimationData`](../Ash2/src/Config/AnimationData.hpp)

- スプライトシート単位のアニメーション共有データ（テクスチャキー・コマサイズ・描画オフセット・クリップ表）
- `AnimationClip` は行番号・コマ数・再生速度（コマ/秒）を持つ
- `AnimationDataRegistry`（キー: 設定ファイルのベース名）として `assets/config/animation/*.toml` から一括ロードされる

### [`ScenarioData`](../Ash2/src/Config/ScenarioData.hpp)

- `assets/config/scenario.toml` から読み込むシナリオ（セクション名 → ステップ列）
- 詳細は上記「フェーズシステム」を参照。`make` アクションは未対応（エラーになる）

---

## アセット管理（[Asset.hpp](../Ash2/src/Asset.hpp)）

- デバッグ: `assets/asset_list` をファイルから読む
- リリース: `assets/asset_list` を埋め込みリソースから読む
- `.png` → `TextureAsset`、`.mp3` → `AudioAsset` としてキー（相対パス）で登録
- アニメーション設定: `assets/config/animation/*.toml`（起動時に全ファイルをスキャン）

---

## セットアップ・ユーティリティ

| 名前 | 役割 |
|---|---|
| [`GameSetup`](../Ash2/src/GameSetup.hpp) | `InitializeRegistry()`（ctx への NameLookup / PlayerConfig / AnimationDataRegistry / ScenarioData 登録とシグナル接続）・`ReloadConfig()`（Debug ビルド専用の設定再読込） |
| [`Debug.hpp`](../Ash2/src/Debug.hpp) | `APP_LOG` マクロ（Debug ビルドのみコンソール出力、テスト実行中は無効化） |
| [`Overloaded`](../Ash2/src/Util/Overloaded.hpp) | `std::visit` に複数のラムダを渡すためのヘルパー構造体 |

---

## 部品追加時の注意

- `Hierarchy` のメンバは必ず static メンバ関数（Attach/Detach/DestroyWithChildren）経由で操作する（不整合防止）。
- `Drawable` の型変更は `std::visit` を使い、DrawSystem と AnimationSystem の両方への影響を確認する。
- 新クラス追加時は `Ash2.vcxproj` と `Ash2.vcxproj.filters` にも追加が必要。
- `NameLookup` への挿入・削除は `NameLookupSystem::Connect` で自動化されている（`Name` コンポーネントの追加・削除に連動）。手動での `NameLookup[key] = entity` 登録は不要。
- `Motion` に新しい状態型を追加したときは、`MotionSystem.cpp` の `MotionName()`（デバッグログ用）にも分岐を追加する。また、`MotionSystem::Update` の `std::visit` は `MotionState` concept（`MotionSystem.hpp`）で制約されているため、新状態型は `Tick(state, registry, entity, frameData) -> Optional<Motion>`（ADL で解決される非修飾 `Tick`）を実装する必要がある。
