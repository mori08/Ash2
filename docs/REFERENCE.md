# REFERENCE.md

「沼に焚べ」の部品一覧。設計意図・レイヤー構成・座標系などは [ARCHITECTURE.md](ARCHITECTURE.md) を参照。

行数上限なし。機能追加のたびに更新する前提のドキュメント。

---

## コンポーネント一覧

| コンポーネント | 役割 |
|---|---|
| [`WorldPos`](../Ash2/src/Component/WorldPos.hpp) | ワールド絶対座標（w/h/d）。`toScreen()`・`isOnGround()` を持つ |
| [`Velocity`](../Ash2/src/Component/Velocity.hpp) | 速度ベクトル（w/h/d、ピクセル/秒） |
| [`Gravity`](../Ash2/src/Component/Gravity.hpp) | 重力の影響を受けるエンティティに付与する重力加速度 |
| [`LocalOffset`](../Ash2/src/Component/LocalOffset.hpp) | 親からの相対座標（Hierarchy 付きエンティティのみ） |
| [`Hierarchy`](../Ash2/src/Component/Hierarchy.hpp) | 親子関係（双方向連結リスト、static メンバで操作） |
| [`Drawable`](../Ash2/src/Component/Drawable.hpp) | 描画形状の variant。詳細は下記「描画データ型」参照 |
| [`SpriteAnimation`](../Ash2/src/Component/SpriteAnimation.hpp) | アニメーション再生状態（per-entity）。共有データは `AnimationDataRegistry` を `dataKey` で参照する |
| [`Name`](../Ash2/src/Component/Name.hpp) | エンティティ名（`const String`、構築後不変。NameLookup と対応） |
| [`Player`](../Ash2/src/Component/Player.hpp) | プレイヤータグ（データなし） |
| [`Enemy`](../Ash2/src/Component/Enemy.hpp) | 敵エンティティを示すタグ（データなし）。`HitReactionSystem` がリアクション適用対象を絞り込むのに使う |
| [`Projectile`](../Ash2/src/Component/Projectile.hpp) | 飛翔体（弾）タグ（データなし）。`WorldPos`+`Velocity`+`Collider`+`Attack` と組み合わせ、`MovementSystem` が移動を、`ProjectileSystem` が消滅を管理する対象を識別する |
| [`Collider`](../Ash2/src/Component/Collider.hpp) | カプセル形状の当たり判定（形状のみ、役割はコンポーネントの組み合わせで表現） |
| [`ReactionLevel`](../Ash2/src/Component/ReactionLevel.hpp) | 被弾側に生じるリアクションの強さを表す `enum class`（`None`/`Stagger`/`Repel`/`Blow` の4値、Lv0〜Lv3に対応） |
| [`Attack`](../Ash2/src/Component/Attack.hpp) | 攻撃中タグ兼攻撃力（`Collider` と組み合わせて攻撃判定が有効になる）。フィールドの詳細は下記「複雑なコンポーネントの詳細」参照 |
| [`Hp`](../Ash2/src/Component/Hp.hpp) | HP（`Collider` と組み合わせて被弾判定の対象になる） |
| [`Stamina`](../Ash2/src/Component/Stamina.hpp) | スタミナ（max / current の int32 フィールド、StaminaSystem が管理する回復端数累積 accum と回復ディレイ計測用 recoveryTimer を持つ） |
| [`Motion`](../Ash2/src/Component/Motion.hpp) | エンティティの排他的な行動状態（`variant`）。状態ごとの詳細は下記「モーション状態型」参照 |
| [`Hitstop`](../Ash2/src/Component/Hitstop.hpp) | ヒットストップ中であることを示す残り時間タイマー。`HitstopSystem` が減算・除去し、付与中は `MovementSystem`/`GravitySystem`/`AnimationSystem` の対象から除外される。`MotionSystem` は除外せず dt = 0 で呼ぶ |
| [`Invincible`](../Ash2/src/Component/Invincible.hpp) | 無敵状態であることを示すタグ。`HitSystem` の被弾対象ビューから除外される。`PlayerMotion::Dash`（地上・空中いずれも）が構え・ダッシュ中は毎フレーム付与し、後隙入りで除去する |

### 複雑なコンポーネントの詳細

#### `Attack`

攻撃中タグ兼攻撃力。`Collider` と組み合わせて攻撃判定が有効になる。

- `damage`: 与えるダメージ量
- `root`: 複数コライダー構成のルートを指定する（単体の場合は `entt::null`）。`HitSystem` は
  ルート側の `hitTargets`・`damage` を参照してヒット管理する
- `hitTargets`: 攻撃生存期間中の重複ヒットを防ぐ
- `hitstopSec`: ヒット成立時に攻撃側・被弾側へ付与するヒットストップ時間
- `reaction`: 被弾側に生じるリアクションの強さ（`ReactionLevel`、既定は `None`）。
  `PlayerMotion::Melee`（次段を持つ段は `Stagger`、最終段は `Blow`）・`DashAttack`（`Repel`）・
  `AirAttack`（`Repel`）が `Helper::UpdateAttackHitbox` 経由で確定させる。`Ranged` は設定せず
  既定の `None` のまま（無反応）

---

## 描画データ型

[`Component/Drawable.hpp`](../Ash2/src/Component/Drawable.hpp) が定義する。
`Drawable` は `variant<RectDrawable, CircleDrawable, PieDrawable, TextureDrawable>`。

| 名前 | 役割 |
|---|---|
| `RectDrawable` | 矩形描画（サイズ・色・枠線・`DrawAnchor`） |
| `CircleDrawable` | 円描画（半径・色・枠線） |
| `PieDrawable` | 扇形描画（半径・開始角・角度・色・枠線）。角度は12時方向から時計回りのラジアン |
| `TextureDrawable` | テクスチャ描画（`TextureRegion`・描画オフセット・`DrawAnchor`） |
| `BorderStyle` | 枠線スタイル（色・太さ）。各形状の `border` が `none` なら枠線なし |
| `DrawAnchor` | `WorldPos` を形状のどこに合わせるか（`Center` / `BottomCenter`）。`RectDrawable` と `TextureDrawable` のみが持ち、既定は `Center` |

---

## `Hierarchy` の static メンバ関数

メンバの更新は必ず以下を経由する（不整合防止）。

| 名前 | 役割 |
|---|---|
| `Hierarchy::Attach` | 子を親の先頭に O(1) で挿入する。すでに別の親を持つ場合は先に Detach し、`Hierarchy` 未所持なら自動追加したうえで `LocalOffset` を設定する |
| `Hierarchy::Detach` | 子を親から O(1) で切り離し、`LocalOffset` を除去する。`on_destroy<Hierarchy>` に接続されているため破棄時に自動で呼ばれる |
| `Hierarchy::DestroyWithChildren` | エンティティと全子孫を再帰的に破棄する。破棄済みエンティティを渡しても安全 |

---

## モーション状態型

`Motion` は `std::variant<PlayerMotion::Neutral, Melee, Ranged, Dash, DashAttack, AirAttack, Landing, EnemyMotion::Idle, Stagger, Repel, Knockback, Defeated>`。

4区間タイムライン（構え/攻撃/後隙A/後隙B）を持つ状態は
[`Config/PlayerConfig.hpp`](../Ash2/src/Config/PlayerConfig.hpp) の `MotionTimeline`
（`isActive`/`isCancelable`/`isFinished`/`activeProgress`）で区間判定する。

### プレイヤー（`PlayerMotion`）

状態型の定義は [`Component/PlayerMotion.hpp`](../Ash2/src/Component/PlayerMotion.hpp)、
`Tick()` の宣言は [`System/PlayerMotionSystem.hpp`](../Ash2/src/System/PlayerMotionSystem.hpp)。
`Dash`/`DashAttack` は `air` フラグ1つで地上・空中の両方を表す（#207 で型を統合、旧 `AirDash`/`AirDashAttack` は廃止）。

- **Neutral**: 通常状態（待機・移動・ジャンプ）。各アクションへの入場判定を行う。保持データなし。
  接地中は Melee/Ranged/Dash へ、空中では AirAttack/Ranged/Dash へ入場できる
- **Ranged**: 再生中クリップの残り時間（`timer`）を持つ。`Neutral` から接地・空中いずれでも
  入場できる（空中発動時も専用の遷移や着地処理は持たず、Neutral 復帰までの挙動は地上と同一）
- **Melee**: コンボ段インデックス（`stage`）を持つ単一の型。段ごとの設定は `cfg.melee.stages[stage]`
  （`MeleeStageConfig`、軌道は `MeleeTrajectory::Thrust`/`Slash`）を参照する。モーション開始からの
  経過時間（`elapsed`）・攻撃判定の子エンティティ（`hitboxEntity`）を持つ。次段が存在する段
  （1・2段目相当）は次段への遷移予約フラグ（`comboQueued`）と後隙中のダッシュキャンセルを持つが、
  次段を持たない最終段（締め技、3段目相当）はコンボ継続・ダッシュキャンセルのいずれも持たず
  タイマー満了で `Neutral` へ戻るのみ。各段の `MotionTimeline` は `recoveryASec=0` として後隙全体を
  キャンセル可能区間として扱う（Melee は元々後隙A/B分割を持たないため）
- **Dash**: `air` フラグで地上・空中を切り替える。構え・ダッシュ・後隙A・後隙Bの4区間を
  `elapsed` 1本で管理する。構え・ダッシュ中（`elapsed < activeEnd()`）は地上・空中いずれも
  `Invincible` を毎フレーム付与し、後隙入りで除去する。ダッシュ中・後隙A・B中の攻撃入力
  （`attackDown`）でダッシュ攻撃を予約（`dashAttackQueued`）し、後隙B中に `DashAttack`
  （`air` を引き継ぐ）へ遷移する。後隙B中のダッシュ入力（`dashDown`）は再ダッシュにキャンセルする
  （`dashQueued`、地上・空中共通の仕様）。ダッシュ移動中の方向を `lastDashDir` に記録し
  `DashAttack::dashDir` へ引き渡す。`air=true` の場合のみ、移動区間中の垂直速度を 0 に固定して
  重力の影響を受けず（暫定仕様）、後隙中も含め毎フレーム接地を検出して接地した時点で
  （`Invincible` を除去したうえで）`Landing` へ強制遷移する。`air=false`（地上）の場合は
  これらの `air` 専用分岐を通らず、接地遷移も持たない
- **DashAttack**: `air` フラグで地上・空中を切り替える。構え・攻撃・後隙の3区間
  （`recoveryBSec=0` として即 Neutral 復帰）を持ち、攻撃判定（`hitboxEntity`）を w-d 平面の
  円軌道上で更新する。`air=true` の場合のみ、突進フェーズ中の垂直速度を 0 に固定し、後隙中も
  含め毎フレーム接地を検出して接地した時点で（残っていればヒットボックスを破棄したうえで）
  `Landing` へ強制遷移する。`air=false`（地上）の場合は接地遷移を持たない。スタミナ枯渇時の
  威力低下（リアクション降格）は未実装、本格対応は #233 のスコープ
- **AirAttack**: `Neutral` が空中（`!WorldPos::isOnGround()`）で攻撃入力を受けたときに入場する。
  地上に対応する型を持たない唯一の空中専用状態。構え・攻撃・後隙の3区間（`recoveryBSec=0`）を
  持ち、攻撃判定（`hitboxEntity`）を w-h 平面（垂直面）の円軌道上で更新する（回転方向は
  プレイヤーの向きに応じて左右反転する）。後隙中も含め毎フレーム接地を検出し、接地した時点で
  （残っていればヒットボックスを破棄したうえで）`Landing` へ強制遷移する。接地せずに後隙が
  満了した場合はタイマー満了で `Neutral` へ戻る。スタミナ枯渇時の威力低下は `DashAttack` 同様に
  未実装、本格対応は #233 のスコープ
- **Landing**: 着地硬直のタイマー状態（`timer`）。空中アクションの接地検出から遷移する

### 敵（`EnemyMotion`）

状態型の定義は [`Component/EnemyMotion.hpp`](../Ash2/src/Component/EnemyMotion.hpp)。
各状態は `remaining`（残り時間）1つだけを持つ（`Idle` は空構造体）。

- **Idle**: 敵の通常状態（無反応）
- **Stagger**: ひるみ中。`Tick()` が `RectDrawable::size` を `EnemyConfig::size` を基準に縦縮みさせ、
  満了時に原寸へ戻して `Idle` へ遷移する（縮み幅は duration の中間で最大、両端で原寸に近づく）
- **Repel**: 弾かれ中。被弾時に `HitReactionSystem` が `Velocity.w` を押し出し方向×
  `EnemyConfig::repelSpeed` に設定し、`Tick()` は残り時間を減算するのみ。満了時に `Velocity.w` を
  0 に戻し `Idle` へ遷移する
- **Knockback**: 吹っ飛び中。被弾時に `HitReactionSystem` が `Velocity.w`/`Velocity.h` を設定し、
  放物線は `MovementSystem`/`GravitySystem` に委ねる（`Gravity` の付与が前提）。`Tick()` は残り時間を
  減算し、接地中かつ下降中（`isOnGround()` かつ `vel.h <= 0`）は `Velocity.w` を 0 に固定する
  （打ち上げ直後は接地したまま Tick に入るため、上昇中は止めない）。満了時に `Idle` へ遷移する
- **Defeated**: 撃破後の消滅演出中。`HitReactionSystem` が `Hp` 枯渇を検出した時点で
  `Collider`/`Hp` を外し（被弾判定から除外）入場させる。`Tick()` は残り時間を減算しながら
  `RectDrawable::color.a` を残り時間比でフェードアウトさせる（`Idle` への遷移はしない）。
  満了エンティティの破棄は `EnemySystem` が行う

---

## システム一覧

すべて static 関数（または名前空間内の自由関数）で、インスタンス状態を持たない。

| システム | タイミング | 処理 |
|---|---|---|
| [`HitstopSystem::Update`](../Ash2/src/System/HitstopSystem.hpp) | フェーズ内（PlayerTestPhase、MotionSystem の前） | `Hitstop` を持つエンティティの残り時間を減算し、0 以下になったら除去する |
| [`MotionSystem::Update`](../Ash2/src/System/MotionSystem.hpp) | フェーズ内（PlayerTestPhase、HitstopSystem の後） | `Motion` の全状態ごとの `Tick()` を呼び、移動・ジャンプ・向き・クリップ決定・状態遷移（攻撃判定/弾エンティティ生成、タイマー満了、無敵の付与/除去、接地検出）を行う。`Hitstop` を持つエンティティには dt = 0 の `FrameData` を渡し、タイムラインを凍結したまま入力の予約（`comboQueued` 等）だけ拾わせる |
| [`StaminaSystem::Update`](../Ash2/src/System/StaminaSystem.hpp) | フェーズ内（PlayerTestPhase、MotionSystem の後） | `Player + Stamina + Motion` を持つエンティティのスタミナを回復する。Neutral 状態のみ `recoveryDelay` 秒の待機後に不足分に比例した速度（`recoveryRate`）で回復し、端数は `accum` に積み立てて誤差を防ぐ |
| [`MovementSystem::Update`](../Ash2/src/System/MovementSystem.hpp) | フェーズ内（PlayerTestPhase、StaminaSystem の後） | `Hitstop` を持たない `WorldPos`+`Velocity` エンティティ（Player・弾・Enemy）の位置を `vel * dt` で更新 |
| [`GravitySystem::Update`](../Ash2/src/System/GravitySystem.hpp) | フェーズ内（PlayerTestPhase、MovementSystem の後） | `Hitstop` を持たない `WorldPos`+`Velocity`+`Gravity` エンティティに重力加速（次フレーム用）と地面クランプ（今フレームの `pos.h`・`vel.h` を 0 にする）を適用 |
| [`AttachmentSystem::UpdateTransform`](../Ash2/src/System/AttachmentSystem.hpp) | 毎フレーム（フェーズ後）＋フェーズ内（PlayerTestPhase、GravitySystem の後・HitSystem の前） | Hierarchy ルートから子孫へ WorldPos 伝播。PlayerTestPhase では HitSystem が同フレーム内の最新座標（光の珠の LocalOffset 反映後）を見られるよう追加で呼び出す |
| [`HitSystem::Update`](../Ash2/src/System/HitSystem.hpp) | フェーズ内（PlayerTestPhase、AttachmentSystem の後） | `Collider+Attack` と `Collider+Hp`（`Invincible` を除く）の間でカプセル重なり検出 → Hp 減算。新たに成立したヒットの `HitPair`（attacker/target）配列を返す |
| [`HitReactionSystem::Apply`](../Ash2/src/System/HitReactionSystem.hpp) | フェーズ内（PlayerTestPhase、HitSystem の後） | `HitSystem::Update` が返した `HitPair` ごとに、攻撃側本体（ヒットボックスの `Hierarchy` 親）と被弾側へ `Attack.hitstopSec` 分の `Hitstop` を付与する（`hitstopSec<=0` はスキップ、既に付与中なら `Max` で長い方の残り時間を残す）。`Enemy` を持つ被弾側に限り、`Hp` 枯渇時は `reaction` によらず `Defeated` へ（`Collider`/`Hp` を除去）、それ以外は `Attack.reaction` に応じて `Stagger`/`Repel`/`Knockback` へ `Motion` を強制遷移させ、攻撃側本体との `WorldPos.w` 比較で決めた向きの `Velocity` を設定する |
| [`ProjectileSystem::Update`](../Ash2/src/System/ProjectileSystem.hpp) | フェーズ内（PlayerTestPhase、HitReactionSystem の後） | Projectile の着弾（hitTargets 非空）/ 画面外での破棄 |
| [`EnemySystem::Update`](../Ash2/src/System/EnemySystem.hpp) | フェーズ内（PlayerTestPhase、ProjectileSystem の後） | `EnemyMotion::Defeated` の残り時間が尽きたエンティティを収集し、`MotionSystem` のビュー走査外でまとめて破棄する |
| [`AnimationSystem::Update`](../Ash2/src/System/AnimationSystem.hpp) | フェーズ内（各フェーズが直接呼出） | `Hitstop` を持たない SpriteAnimation の elapsed を進め、切り出した `TextureRegion` を `TextureDrawable` に反映する（`facingRight` なら反転） |
| [`DrawSystem::Draw`](../Ash2/src/System/DrawSystem.hpp) | 毎フレーム（HudSystem の前） | WorldPos+Drawable を奥行き順にソートして描画 |
| [`HudSystem::Draw`](../Ash2/src/System/HudSystem.hpp) | 毎フレーム（DrawSystem の後） | Player の Hp / Stamina を画面左上にゲージ描画（プレイヤー 1 体のみ想定）。他のシステムと異なり実装をヘッダに直書きしている |
| [`NameLookupSystem::Connect`](../Ash2/src/System/NameLookup.hpp) | 起動時 | Name 追加・削除時に NameLookup を自動同期するシグナル登録 |
| [`HierarchySystem::Connect`](../Ash2/src/System/HierarchySystem.hpp) | 起動時 | Hierarchy 削除時に Detach を自動呼び出しするシグナル登録 |

### システム付随の型・関数

| 名前 | 役割 |
|---|---|
| [`HitPair`](../Ash2/src/System/HitSystem.hpp) | 攻撃側・被弾側エンティティの組。`HitSystem` → `HitReactionSystem` の受け渡しに使う |
| [`MotionState`](../Ash2/src/System/MotionSystem.hpp) | `Motion` の状態型が満たすべきコンセプト（ADL で解決される `Tick()` が `Optional<Motion>` を返すこと） |
| [`DrawOrderLess`](../Ash2/src/System/DrawSystem.hpp) | 描画順の比較関数（`a.d > b.d` で奥が先） |
| [`NameLookup`](../Ash2/src/System/NameLookup.hpp) | 名前 → エンティティの `HashTable`。`registry.ctx()` に格納 |

### 敵モーションの実装ファイル

`MotionSystem::Update` が呼ぶ `EnemyMotion::Tick()` は
[`EnemyMotionSystem.hpp`](../Ash2/src/System/EnemyMotionSystem.hpp)/`.cpp` に5状態分すべてを
宣言・実装する（`PlayerMotion` と異なり状態ごとのファイル分割はしない。各状態が `remaining`
1つだけを持つ単純さのため）。

### プレイヤーモーションの実装ファイル

`MotionSystem::Update` が呼ぶ `Tick()` は
[`PlayerMotionSystem.hpp`](../Ash2/src/System/PlayerMotionSystem.hpp) が宣言し、実体は
`Ash2/src/System/PlayerMotion/` 配下に状態ごとに分かれている。

- 状態別の `Tick()` と入場関数
  - [`Neutral.cpp`](../Ash2/src/System/PlayerMotion/Neutral.cpp) / [`Melee.cpp`](../Ash2/src/System/PlayerMotion/Melee.cpp) / [`Ranged.cpp`](../Ash2/src/System/PlayerMotion/Ranged.cpp) / [`Dash.cpp`](../Ash2/src/System/PlayerMotion/Dash.cpp) / [`DashAttack.cpp`](../Ash2/src/System/PlayerMotion/DashAttack.cpp) / [`AirAttack.cpp`](../Ash2/src/System/PlayerMotion/AirAttack.cpp) / [`Landing.cpp`](../Ash2/src/System/PlayerMotion/Landing.cpp)
- 複数状態が使う共通ヘルパー
  - [`Helper.hpp`](../Ash2/src/System/PlayerMotion/Helper.hpp)
- 状態遷移用の入場関数の宣言
  - [`Transition.hpp`](../Ash2/src/System/PlayerMotion/Transition.hpp)（定義は遷移先の状態の `.cpp` が持つ）

### プレイヤーモーションの補助部品

| 名前 | 役割 | 定義 |
|---|---|---|
| `HitboxSpec` | 攻撃判定エンティティの生成仕様（半径・ダメージ・リアクション・ヒットストップ時間）をまとめた構造体 | `Helper.hpp` |
| `SetClip` | クリップが変化していれば差し替え、再生位置をリセットする | `Helper.hpp`/`.cpp` |
| `StopHorizontalMovement` | 横方向（w・d）の速度を 0 にする | `Helper.hpp`/`.cpp` |
| `UpdateAttackHitbox` | active 区間に応じて攻撃判定エンティティ（光の珠）を生成・`LocalOffset` 更新・破棄する。オフセットは `offsetFn(progress)` で決まる | `Helper.hpp`/`.cpp` |
| `MakeMelee` | 指定段の `Melee` を生成（`melee_{stage+1}` クリップを先頭から再生） | `Transition.hpp` / `Melee.cpp` |
| `MakeRanged` | `Ranged` を生成（スタミナ消費、`timer` はクリップ再生時間から算出） | `Transition.hpp` / `Ranged.cpp` |
| `MakeDash` | `Dash` を生成（スタミナ消費、`air` フラグ設定） | `Transition.hpp` / `Dash.cpp` |
| `MakeDashAttack` | `DashAttack` を生成（`air`・`dashDir` を引き継ぐ） | `Transition.hpp` / `DashAttack.cpp` |
| `MakeAirAttack` | `AirAttack` を生成 | `Transition.hpp` / `AirAttack.cpp` |
| `SpawnProjectile` | 遠距離攻撃の弾エンティティを生成する（`WorldPos`+`Velocity`+`Collider`+`Attack`+`Drawable`+`Projectile`） | `Transition.hpp` / `Ranged.cpp` |

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
| [`PlayerTestPhase`](../Ash2/src/Phase/PlayerTestPhase.hpp) | `player_test` | プレイヤー操作・物理・アニメーションのビジュアルテスト。`EnemyConfig` から敵（`Enemy`+`Motion`+`Collider`+`Hp` 等）を1体生成し、`HitReactionSystem`/`EnemySystem` に被弾リアクション・撃破後の破棄を委ねる。敵が破棄されたら `EnemyConfig::respawnSec` 後に再生成する。F5 でプレイヤー・設定再生成、Esc で Pop |
| [`AnimationViewerPhase`](../Ash2/src/Phase/AnimationViewerPhase.hpp) | `animation_viewer` | アニメーションクリップ単体確認（←→切替、F反転、Esc で Pop） |
| [`WaitPhase`](../Ash2/src/Phase/WaitPhase.hpp) | `wait` | 指定秒数待機して Pop |

### シナリオ TOML から生成できるフェーズ名

| フェーズ名 | 生成される型 | TOML パラメータ |
|---|---|---|
| `player_test` | `PlayerTestPhase` | なし |
| `test_menu` | `TestMenuPhase` | なし |
| `animation_viewer` | `AnimationViewerPhase` | `param`（`AnimationDataRegistry` のキー） |
| `scenario` | `ScenarioPhase` | `param`（セクション名） |
| `wait` | `WaitPhase` | `duration`（秒） |

[`Config/ScenarioData`](../Ash2/src/Config/ScenarioData.hpp) がシナリオロード時に各ステップを
`IPhaseMaker`（型消去された `make() -> unique_ptr<IPhase>`）を持つ `ScenarioStep`
（`StepPush`/`StepReset`）に変換する。ステップの `action` は `push`/`reset` に対応する
（`make` は未対応でエラー、#67 スコープ外）。

変換テーブルは [`Phase/PhaseLoaders.cpp`](../Ash2/src/Phase/PhaseLoaders.cpp) の
`GetPhaseLoaders()` で定義されており、**新フェーズ追加時はここにもエントリを追加する必要がある。**
`PhaseWithParam` コンセプトと `PhaseMaker<T>` が TOML パラメータを型消去して保持する。
`ScenarioData::FromToml` はこのテーブルを `PhaseLoaderTable` として引数で受け取る
（Config 層から具象フェーズへの依存を作らないため）。呼び出し元は `GameSetup` が
`GetPhaseLoaders()` を渡して配線する。

---

## 設定（Config）

`Ash2/src/Config/` 配下。TOML から `FromToml()` で構築し、`registry.ctx()` に格納する。

### [`PlayerConfig`](../Ash2/src/Config/PlayerConfig.hpp)

- `assets/config/player.toml` から読み込むプレイヤー設定
- 基本値（移動速度 `speed`・ジャンプ初速 `jumpSpeed`・重力 `gravity`）と、`MeleeConfig` /
  `RangedConfig` / `DashConfig` / `DashAttackConfig` / `AirAttackConfig` / `StaminaConfig` /
  `LandingConfig` の各サブ設定を持つ
- 地上・空中を共有する `Dash`/`DashAttack`（`air` フラグで区別）は、それぞれ単一の
  `DashConfig`/`DashAttackConfig` を共通で参照する（専用設定は持たない）

同ヘッダが定義する部品：

| 名前 | 役割 |
|---|---|
| `MotionTimeline` | 攻撃・ダッシュ系共通の4区間タイムライン（windup / active / 後隙A＝キャンセル不可 / 後隙B＝キャンセル可）。`activeStart`/`activeEnd`/`recoveryAEnd`/`recoveryBEnd` と `isActive`/`isCancelable`/`isFinished`/`activeProgress` を提供する。`DashConfig`/`DashAttackConfig`/`AirAttackConfig`/`MeleeStageConfig` が共通で持つ |
| `MeleeTrajectory` | 近接攻撃の軌道パターン（`Thrust` 突き出し / `Slash` 斬り上げ） |
| `MeleeStageConfig` | コンボ段ごとの設定（`timeline`/`radius`/`trajectory`/`slashRiseHeight`/`hitstopSec`） |
| `MeleeConfig` | 段共通のパラメータ（`capMidH`/`reach`/`damage`）とコンボ段配列 `stages`（先頭が1段目） |
| `RangedConfig` | リーチ・半径・ダメージ・弾速・発射高さ・スタミナ消費 |
| `DashConfig` | 速度・タイムライン・スタミナ消費 |
| `DashAttackConfig` | タイムライン・突進速度・軌道半径（w-d 平面）・カプセル半径・ダメージ・ヒットストップ時間 |
| `AirAttackConfig` | タイムライン・軌道半径（w-h 平面）・カプセル半径・ダメージ・ヒットストップ時間 |
| `StaminaConfig` | 回復開始待機秒数 `recoveryDelay`・毎秒の不足分回復割合 `recoveryRate` |
| `LandingConfig` | 着地硬直時間 `recoverySec` |

- `hitstopSec`（`MeleeStageConfig`/`DashAttackConfig`/`AirAttackConfig` が個別に持つ）はヒット成立時に
  `Attack.hitstopSec` へ渡す停止時間で、段・アクションごとに調整できる（`RangedConfig` は持たない。
  弾は `reaction` が `None` で無反応の仕様のため）
- 近接攻撃はスタミナを消費しない（`MeleeConfig` は `staminaCost` を持たない）
- `[[melee.stage]]` が0件（欠落含む）の場合、`FromToml` は `Error` を投げる
  （`Tick(Melee&, ...)` の `stages[state.stage]` アクセスを不正にしないため）

### [`EnemyConfig`](../Ash2/src/Config/EnemyConfig.hpp)

- `assets/config/enemy.toml` から読み込む敵設定
- ステータス・形状（`maxHp`/`size`/`capsuleRadius`/`capsuleHeight`/`spawnW`）と、`EnemyMotion`
  各状態の演出パラメータ（`staggerSec`、`repelSpeed`/`repelSec`、`blowSpeedW`/`blowSpeedH`/
  `knockbackSec`、`defeatedSec`/`respawnSec`）を持つ
- `Knockback` の重力加速度は専用の値を持たず、`PlayerConfig::gravity` を敵にもそのまま付与して
  流用する（`PlayerTestPhase::spawnEnemy` 参照）
- 色は toml 化せず `PlayerTestPhase.cpp` 側の定数（`KDummyColor`）に残す（パーサを増やさないため）
- リアクション Lv（`ReactionLevel`）自体は config 化せず、各 `PlayerMotion` の `Tick()` が固定値で
  割り当てる。スタミナ連動の降格表を含む config 化は #233 のスコープ

### [`AnimationData`](../Ash2/src/Config/AnimationData.hpp)

- スプライトシート単位のアニメーション共有データ（テクスチャキー・コマサイズ・描画オフセット・
  クリップ表）
- `AnimationClip` は行番号・コマ数・再生速度（コマ/秒）を持つ
- `AnimationDataRegistry`（キー: 設定ファイルのベース名）として `assets/config/animation/*.toml` から
  一括ロードされる
- `texture`/`width`/`height`/`draw_offset` は予約キーで、それ以外のテーブルがクリップとして扱われる

### [`ScenarioData`](../Ash2/src/Config/ScenarioData.hpp)

- `assets/config/scenario.toml` から読み込むシナリオ（セクション名 → ステップ列）
- 同ヘッダが `IPhaseMaker` / `PhaseLoader` / `PhaseLoaderTable` / `StepPush` / `StepReset` /
  `ScenarioStep` を定義する
- 詳細は上記「フェーズシステム」を参照

### `registry.ctx()` の内容一覧

`InitializeRegistry()` が起動時にセットする。

| 型 | 用途 |
|---|---|
| `NameLookup` | 名前 → エンティティの逆引きテーブル |
| `PlayerConfig` | プレイヤー設定 |
| `EnemyConfig` | 敵設定 |
| `AnimationDataRegistry` | アニメーション共有データ |
| `ScenarioData` | シナリオデータ |

---

## 入力抽象化

| クラス | 役割 |
|---|---|
| [`InputState`](../Ash2/src/Input/InputState.hpp) | フレームの論理入力（`moveAxis`/`jumpDown`/`attackDown`/`rangedAttackDown`/`dashDown`/`reloadConfig`）。`Key`/`TOMLValue` 等のテストしづらい型は持ち込まないが、`Vec2` 等の単純な数学型は許容する |
| [`KeyboardInputAction`](../Ash2/src/Input/KeyboardInputAction.hpp) | キーボード/マウス → InputState 変換（デフォルト: 矢印/WASD 移動、Space ジャンプ、左クリック近距離、右クリック遠距離、Shift ダッシュ、F5 設定リロード） |
| [`XInputAction`](../Ash2/src/Input/XInputAction.hpp) | XInput コントローラー（プレイヤー0）→ InputState 変換（左スティック+十字ボタンで移動、A/B/X/Y でジャンプ/近距離/遠距離/ダッシュ） |
| [`InputDeviceSelector`](../Ash2/src/Input/InputDeviceSelector.hpp) | 毎フレームデバイス入力を検出し、最後にアクティブだったデバイスに切り替える（ボタン入力に加え、左スティックがデッドゾーンを超えて傾いた場合もゲームパッドへの切り替え条件とする）。切断時はキーボードへ自動フォールバック |

**移動入力の正規化方針：** `InputState::moveAxis`（`Vec2`、x=横方向/y=奥行き方向）は「常に長さ 1.0
以下に正規化済み」という不変条件を持つ。この保証の責任は `toInputState()` を実装する各入力レイヤー側に
あり、`PlayerMotion::Tick(Neutral&, ...)` は無条件にこの値を信頼してそのまま速度計算に使う
（System 側で正規化やクランプを行わない）。`XInputAction` は左スティックのデッドゾーン定数
（`LeftThumbDeadZone`、`InputDeviceSelector` も参照する公開 `static constexpr` メンバ）を適用し、
十字ボタンの軸ベクトルと加算したうえで `limitLength(1.0)` により正規化する。

**フェーズの直接キー入力：** `TestMenuPhase`（↑↓/Enter）・`PlayerTestPhase`（Esc）・
`AnimationViewerPhase`（Esc/←→/F）は `InputState` を経由せず Siv3D の `Key*` を直接参照している。
これらの操作にはゲームパッドの割り当てがない。

---

## アセット管理（[Asset.hpp](../Ash2/src/Asset.hpp)）

- デバッグ: `assets/asset_list` をファイルから読む
- リリース: `assets/asset_list` を埋め込みリソースから読む
- `.png` → `TextureAsset`、`.mp3` → `AudioAsset` としてキー（相対パス）で登録
- アニメーション設定: `assets/config/animation/*.toml`（起動時に全ファイルをスキャン）

---

## 基盤・ユーティリティ

| 名前 | 役割 |
|---|---|
| [`Main`](../Ash2/src/Main.cpp) | アプリの入口。アセット登録 → registry 初期化 → `PhaseStack` を生成し、毎フレーム `PhaseStack::update` → `AttachmentSystem` → `DrawSystem` → `HudSystem` を回す。例外は `crash.log` に記録して再 throw。Debug ビルドで環境変数 `ASH2_RUN_TESTS` が設定されていれば Catch2 のテストのみ実行して終了 |
| [`InitializeRegistry`](../Ash2/src/GameSetup.hpp) | `registry.ctx()` へ `NameLookup` / 各 Config / `AnimationDataRegistry` / `ScenarioData` を登録し、シグナルを接続する |
| [`ReloadConfig`](../Ash2/src/GameSetup.hpp) | Debug ビルド専用。`PlayerConfig` / `EnemyConfig` / アニメーションデータを再読込する |
| [`GetAssetList`](../Ash2/src/Asset.hpp) | `assets/asset_list` を読んでアセットパス一覧を返す |
| [`AssetPath`](../Ash2/src/Asset.hpp) | Debug では `FilePath`、Release では `Resource` パスを返す |
| [`RegisterAssets`](../Ash2/src/Asset.hpp) | `.png`/`.mp3` をアセットシステムに登録する |
| [`APP_LOG`](../Ash2/src/Debug.hpp) | Debug ビルドで `Console` に出力するログマクロ（Release では何もしない） |
| [`AppDebug::testMode`](../Ash2/src/Debug.hpp) | テスト実行中フラグ。true の間 `APP_LOG` を無効化する |
| [`Overloaded`](../Ash2/src/Util/Overloaded.hpp) | 複数のラムダを1つの visitor にまとめる `std::visit` 用ヘルパー |

---

## アニメーションクリップ名

`assets/config/animation/player.toml` のクリップ名としてコードから参照されるもの。
クリップ名の欠落は `AnimationSystem` の `assert` で検出される（`ranged_attack` は
`MakeRanged` でも同様の assert で検出する）。

| クリップ名 | 使用箇所 |
|---|---|
| `idle` | `Neutral`（静止時） |
| `move` | `Neutral`（移動時） |
| `jump_rise` / `jump_fall` | `Neutral`（上昇中／落下中） |
| `melee_1` / `melee_2` / `melee_3` | `MakeMelee`（段番号 +1 で決まる） |
| `ranged_attack` | `MakeRanged`（`Ranged::timer` の算出元にもなる） |
| `dash` | `MakeDash` |
| `dash_attack` | `MakeDashAttack` |
| `air_attack` | `MakeAirAttack` |
| `landing` | `Landing` の `Tick()` |

---

## テスト

`Ash2/tests/` に Catch2 のテストを置く。実行方法は [TEST.md](TEST.md) を参照。

| テストファイル | 対象 |
|---|---|
| `TestWorldPos.cpp` | `WorldPos` の座標変換・接地判定、`DrawOrderLess` |
| `TestMovementSystem.cpp` | `MovementSystem` |
| `TestAttachmentSystem.cpp` | `AttachmentSystem` の座標伝播、`Hierarchy` の連結リスト操作 |
| `TestHitSystem.cpp` | `HitSystem` のカプセル交差・重複ヒット防止・root 解決 |
| `TestHitReactionSystem.cpp` | `HitReactionSystem` のリアクション適用・ヒットストップ付与 |
| `TestHitstopSystem.cpp` | `HitstopSystem` |
| `TestPlayerMotionSystem.cpp` | プレイヤー各状態の `Tick()` |
| `TestEnemyMotionSystem.cpp` | 敵各状態の `Tick()`、`EnemySystem` |
| `TestProjectileSystem.cpp` | `ProjectileSystem` の消滅条件 |
| `TestNameLookup.cpp` | `NameLookupSystem` のシグナル同期 |
| `TestPlayerConfig.cpp` | `PlayerConfig::FromToml` |
| `TestAnimationData.cpp` | `AnimationData::FromToml` |
| `TestScenarioData.cpp` | `ScenarioData::FromToml` |
| `TestPhaseStack.cpp` | `PhaseStack` の push / pop / reset |
| `TestWaitPhase.cpp` | `WaitPhase` |

---

## 部品追加時の注意

- `Hierarchy` のメンバは必ず static メンバ関数（Attach/Detach/DestroyWithChildren）経由で操作する。
- `Drawable` の型変更は `std::visit` を使い、DrawSystem と AnimationSystem の両方への影響を確認する。
- 新クラス追加時は `Ash2.vcxproj` と `Ash2.vcxproj.filters` にも追加が必要。
- `NameLookup` への挿入・削除は `NameLookupSystem::Connect` で自動化されている（`Name` コンポーネントの追加・削除に連動）。手動での `NameLookup[key] = entity` 登録は不要。
- `Motion` に新しい状態型を追加したときは、その型に `Tick(state, registry, entity, frameData) -> Optional<Motion>`（ADL で解決される非修飾 `Tick`）を実装する必要がある。`MotionSystem::Update` の `std::visit` が `MotionState` concept（`MotionSystem.hpp`）で制約されているため。
- 新フェーズを追加し TOML から `push`/`reset` できるようにするときは、`Phase/PhaseLoaders.cpp` の `GetPhaseLoaders()` にもエントリを追加する。
- 新しいアニメーションクリップを参照するときは `assets/config/animation/*.toml` 側にも追加する（欠落は `AnimationSystem` の `assert` で落ちる）。
