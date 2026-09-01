# REFERENCE.md

「沼に焚べ」の部品一覧。設計の全体像は [ARCHITECTURE.md](ARCHITECTURE.md) を参照。
ゲームデザイン上の仕様（モーションの区間秒数・キャンセル可否・リアクション Lv など）は
[docs/game_design/](game_design/) 配下（[motion_design.md](game_design/motion_design.md) ほか）を参照。

行数上限なし。機能追加のたびに更新する前提のドキュメント。

---

## コンポーネント一覧

| コンポーネント | 役割 |
|---|---|
| [`WorldPos`](../Ash2/src/Component/WorldPos.hpp) | ワールド絶対座標（w/h/d）。`toScreen()`・`isOnGround()` を持つ |
| [`Velocity`](../Ash2/src/Component/Velocity.hpp) | 速度ベクトル（w/h/d、ピクセル/秒） |
| [`Gravity`](../Ash2/src/Component/Gravity.hpp) | 重力の影響を受けるエンティティに付与する重力加速度 |
| [`LocalOffset`](../Ash2/src/Component/LocalOffset.hpp) | 親からの相対座標（w/h/d、Hierarchy 付きエンティティのみ）。`WorldPos` とは別の型で、`toScreen()`・`isOnGround()` は持たない |
| [`Hierarchy`](../Ash2/src/Component/Hierarchy.hpp) | 親子関係（双方向連結リスト、static メンバで操作） |
| [`Drawable`](../Ash2/src/Component/Drawable.hpp) | 描画形状の variant。詳細は下記「描画データ型」参照 |
| [`DrawColor`](../Ash2/src/Component/DrawColor.hpp) | 描画色（`ColorF`）。図形では塗り色、テクスチャでは乗算色として使う。未所持は白・不透明（`kDefaultDrawColor`）として扱われる |
| [`SpriteAnimation`](../Ash2/src/Component/SpriteAnimation.hpp) | アニメーション再生状態（per-entity）。共有データは `AnimationDataRegistry` を `dataKey` で参照する |
| [`Name`](../Ash2/src/Component/Name.hpp) | エンティティ名（`const String`、構築後不変。NameLookup と対応） |
| [`Player`](../Ash2/src/Component/Player.hpp) | プレイヤータグ（データなし） |
| [`Enemy`](../Ash2/src/Component/Enemy.hpp) | 敵エンティティを示すタグ（データなし）。`HitReactionSystem` がリアクション適用対象を絞り込むのに使う |
| [`Projectile`](../Ash2/src/Component/Projectile.hpp) | 飛翔体（弾）タグ（データなし）。`WorldPos`+`Velocity`+`Collider`+`Attack` と組み合わせ、`MovementSystem` が移動を、`ProjectileSystem` が消滅を管理する対象を識別する |
| [`Collider`](../Ash2/src/Component/Collider.hpp) | カプセル形状の当たり判定（形状のみ、役割はコンポーネントの組み合わせで表現） |
| [`ReactionLevel`](../Ash2/src/Component/ReactionLevel.hpp) | 被弾側に生じるリアクションの強さを表す `enum class`（`None`/`Stagger`/`Repel`/`Blow` の4値、Lv0〜Lv3に対応） |
| [`Attack`](../Ash2/src/Component/Attack.hpp) | 攻撃中タグ兼攻撃力（`Collider` と組み合わせて攻撃判定が有効になる）。ヒット済みターゲット集合 `hitTargets` で重複ヒットを防ぎ、複数コライダー構成では `root` が代表エンティティを指す（本番コードでは未設定）。`reaction` の割り当て元と遷移先は下記「リアクションの対応」参照 |
| [`Hp`](../Ash2/src/Component/Hp.hpp) | HP（`Collider` と組み合わせて被弾判定の対象になる） |
| [`Team`](../Ash2/src/Component/Team.hpp) | エンティティの陣営を表す `enum class`（`Player`/`Enemy` の2値）。攻撃判定・被弾判定を持つエンティティに付与し、`HitSystem` が同じ値どうしのヒットを捨てる（自己ヒット・同士討ちの防止）。片方でも持たなければ判定に参加せず従来どおり当たる |
| [`Stamina`](../Ash2/src/Component/Stamina.hpp) | スタミナ（max / current の int32 フィールド、StaminaSystem が管理する回復端数累積 accum と回復ディレイ計測用 recoveryTimer を持つ） |
| [`PlayerMotion::Variant`](../Ash2/src/Component/PlayerMotion.hpp) | プレイヤーの排他的な行動状態（`variant`）。状態ごとの詳細は下記「モーション状態型」参照 |
| [`EnemyMotion::Variant`](../Ash2/src/Component/EnemyMotion.hpp) | 敵の排他的な行動状態（`variant`）。状態ごとの詳細は下記「モーション状態型」参照 |
| [`Hitstop`](../Ash2/src/Component/Hitstop.hpp) | ヒットストップ中であることを示す残り時間タイマー。`HitstopSystem` が減算・除去し、付与中は `MovementSystem`/`GravitySystem`/`AnimationSystem` の対象から除外される。`MotionSystem` は除外せず dt = 0 で呼ぶ |
| [`Invincible`](../Ash2/src/Component/Invincible.hpp) | 無敵状態であることを示すタグ。`HitSystem` の被弾対象ビューから除外される。`PlayerMotion::Dash`（地上・空中いずれも）が構え・ダッシュ中は毎フレーム付与し、後隙入りで除去する。`PlayerMotion::Knockback`/`Downed` も毎フレーム付与し、`GetUp` への遷移時に除去する |
| [`FadeOut`](../Ash2/src/Component/FadeOut.hpp) | 透過しながら消滅する途中であることを示すコンポーネント（`duration`/`remaining`）。`FadeOutSystem` が `DrawColor::color.a` を更新し、満了時にエンティティを破棄する |

---

## 描画データ型

[`Component/Drawable.hpp`](../Ash2/src/Component/Drawable.hpp) が定義する。
`Drawable` は `variant<RectDrawable, CircleDrawable, TextureDrawable>`。
色は形状側ではなく [`DrawColor`](../Ash2/src/Component/DrawColor.hpp) が一括で持つ
（上記「コンポーネント一覧」参照）。

| 名前 | 役割 |
|---|---|
| `RectDrawable` | 矩形描画（サイズ・`DrawAnchor`） |
| `CircleDrawable` | 円描画（半径） |
| `TextureDrawable` | テクスチャ描画（`TextureRegion`・描画オフセット・`DrawAnchor`） |
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

`PlayerMotion::Variant` は
`std::variant<Neutral, MeleeChain, MeleeFinisher, Ranged, Dash, DashAttack, AirAttack, Landing, Stagger, Knockback, Downed, GetUp>`、
`EnemyMotion::Variant` は `std::variant<Idle, Stagger, Repel, Knockback, Defeated>`。
種別ごとに別の variant で、両者を1つの型にまとめたものは無い（`PlayerMotion::Stagger`/`Knockback`
と `EnemyMotion::Stagger`/`Knockback` は名前が同じでも別の型）。

`Tick()` と `MotionSystem` の関係は [ARCHITECTURE.md](ARCHITECTURE.md) の
「4. Motion」を参照。

4区間タイムライン（構え/攻撃/後隙A/後隙B）を持つ状態は
[`Config/PlayerConfig.hpp`](../Ash2/src/Config/PlayerConfig.hpp) の `MotionTimeline`
（`isActive`/`isCancelable`/`isFinished`/`activeProgress`）で区間判定する。各区間の秒数・
キャンセル可否・リアクション Lv は [motion_design.md](game_design/motion_design.md) を参照。

### プレイヤー（`PlayerMotion`）

状態型の定義と各状態が持つデータは [`Component/PlayerMotion.hpp`](../Ash2/src/Component/PlayerMotion.hpp)、
`Tick()` の宣言は [`System/PlayerMotionSystem.hpp`](../Ash2/src/System/PlayerMotionSystem.hpp)。
`Dash`/`DashAttack` は `air` フラグ1つで地上・空中の両方を表す（#207 で型を統合、旧 `AirDash`/`AirDashAttack` は廃止）。

| 状態 | 主な遷移先 | 実装 |
|---|---|---|
| `Neutral` | 接地中の入力で `MeleeChain`/`Ranged`/`Dash`、空中の入力で `AirAttack`/`Ranged`/`Dash` | [`Neutral.cpp`](../Ash2/src/System/PlayerMotion/Neutral.cpp) |
| `MeleeChain` | 後隙Bの攻撃入力・予約で次段の `MeleeChain`（残っていれば）または `MeleeFinisher`、後隙Bのダッシュ入力で `Dash`、満了で `Neutral` | [`Melee.cpp`](../Ash2/src/System/PlayerMotion/Melee.cpp) |
| `MeleeFinisher` | コンボ継続・キャンセルは受け付けない。満了で `Neutral` | [`Melee.cpp`](../Ash2/src/System/PlayerMotion/Melee.cpp) |
| `Ranged` | 満了で `Neutral`（空中発動時も着地処理を挟まず地上と同一） | [`Ranged.cpp`](../Ash2/src/System/PlayerMotion/Ranged.cpp) |
| `Dash` | 後隙Bの予約で `DashAttack`（`air` を引き継ぐ）／再ダッシュの `Dash`、満了で `Neutral`、接地で `Landing`（`air=true` のみ） | [`Dash.cpp`](../Ash2/src/System/PlayerMotion/Dash.cpp) |
| `DashAttack` | 満了で `Neutral`、接地で `Landing`（`air=true` のみ） | [`DashAttack.cpp`](../Ash2/src/System/PlayerMotion/DashAttack.cpp) |
| `AirAttack` | 接地で `Landing`、接地しなければ満了で `Neutral` | [`AirAttack.cpp`](../Ash2/src/System/PlayerMotion/AirAttack.cpp) |
| `Landing` | 満了で `Neutral` | [`Landing.cpp`](../Ash2/src/System/PlayerMotion/Landing.cpp) |
| `Stagger` | `dashDown` で `Dash`、満了で `Neutral` | [`Damaged.cpp`](../Ash2/src/System/PlayerMotion/Damaged.cpp) |
| `Knockback` | 接地かつ `Velocity.h <= 0` で `Downed`（放物線は `MovementSystem`/`GravitySystem` に委ねる） | [`Damaged.cpp`](../Ash2/src/System/PlayerMotion/Damaged.cpp) |
| `Downed` | 満了で `GetUp` | [`Damaged.cpp`](../Ash2/src/System/PlayerMotion/Damaged.cpp) |
| `GetUp` | `dashDown` で `Dash`、満了で `Neutral` | [`Damaged.cpp`](../Ash2/src/System/PlayerMotion/Damaged.cpp) |

### 敵（`EnemyMotion`）

状態型の定義と各状態が持つデータは [`Component/EnemyMotion.hpp`](../Ash2/src/Component/EnemyMotion.hpp)。
5状態とも `Tick()` の実装は [`EnemyMotionSystem.cpp`](../Ash2/src/System/EnemyMotionSystem.cpp) にまとまっている。

| 状態 | 主な遷移先 |
|---|---|
| `Idle` | なし（通常状態。`Tick()` は何もしない） |
| `Stagger` | 満了で `Idle`（`Tick()` が `RectDrawable::size` を縦縮みさせ、満了時に原寸へ戻す） |
| `Repel` | 満了で `Idle`（満了時に `Velocity.w` を 0 に戻す） |
| `Knockback` | 満了で `Idle`（放物線は `MovementSystem`/`GravitySystem` に委ねる） |
| `Defeated` | なし（`DrawColor::color.a` をフェードさせ、満了エンティティは `EnemySystem` が破棄する） |

### 例外

- `air=true` の `Dash`/`DashAttack` と `AirAttack` は、後隙中も含め毎フレーム接地を検出して
  `Landing` へ強制遷移する（タイマー満了より先に評価する）
- `Dash` は構え・ダッシュ中のみ `Invincible` を毎フレーム付与し、後隙入りで除去する
- `Stagger`/`GetUp` は全区間で `dashDown` によるキャンセルを受ける（スタミナ不足なら無視して継続）
- `Knockback`/`Downed` は `Invincible` を毎フレーム付与し、`GetUp` への遷移時に除去する
- 近接の後隙A/Bの配分は段ごとに異なる（`MeleeChain` はキャンセル可、`MeleeFinisher` はキャンセル不可）
- 敵の状態遷移は `Tick()` の戻り値ではなく `HitReactionSystem` が直接行う
  （[ARCHITECTURE.md](ARCHITECTURE.md) の「例外：外部要因による強制遷移」）。
  プレイヤーの被弾（`Stagger`/`Knockback`）も同様に `HitReactionSystem` が
  `PlayerMotion::MakeDamaged` 経由で直接 `replace` する

---

## システム一覧

すべて static 関数（または名前空間内の自由関数）で、インスタンス状態を持たない。

| システム | タイミング | 処理 |
|---|---|---|
| [`HitstopSystem::Update`](../Ash2/src/System/HitstopSystem.hpp) | フェーズ内（PlayerTestPhase、MotionSystem の前） | `Hitstop` を持つエンティティの残り時間を減算し、0 以下になったら除去する |
| [`MotionSystem::Update`](../Ash2/src/System/MotionSystem.hpp) | フェーズ内（PlayerTestPhase、HitstopSystem の後） | `PlayerMotion::Variant` → `EnemyMotion::Variant` の順に、状態ごとの `Tick()` を `std::visit` で呼び、戻り値があれば `replace<M>` する。`Hitstop` を持つエンティティには dt = 0 の `FrameData` を渡す（停止中も入力の受付を続けるため） |
| [`StaminaSystem::Update`](../Ash2/src/System/StaminaSystem.hpp) | フェーズ内（PlayerTestPhase、MotionSystem の後） | `Player + Stamina + PlayerMotion::Variant` を持つエンティティのスタミナを回復する。Neutral 状態のみ `recoveryDelay` 秒の待機後に不足分に比例した速度（`recoveryRate`）で回復し、端数は `accum` に積み立てて誤差を防ぐ |
| [`MovementSystem::Update`](../Ash2/src/System/MovementSystem.hpp) | フェーズ内（PlayerTestPhase、StaminaSystem の後） | `Hitstop` を持たない `WorldPos`+`Velocity` エンティティ（Player・弾・Enemy）の位置を `vel * dt` で更新 |
| [`GravitySystem::Update`](../Ash2/src/System/GravitySystem.hpp) | フェーズ内（PlayerTestPhase、MovementSystem の後） | `Hitstop` を持たない `WorldPos`+`Velocity`+`Gravity` エンティティに重力加速（次フレーム用）と地面クランプ（今フレームの `pos.h`・`vel.h` を 0 にする）を適用 |
| [`AttachmentSystem::UpdateTransform`](../Ash2/src/System/AttachmentSystem.hpp) | 毎フレーム（フェーズ後）＋フェーズ内（PlayerTestPhase、GravitySystem の後・HitSystem の前） | Hierarchy ルートから子孫へ WorldPos 伝播。PlayerTestPhase では HitSystem が同フレーム内の最新座標（光の珠の LocalOffset 反映後）を見られるよう追加で呼び出す |
| [`HitSystem::Update`](../Ash2/src/System/HitSystem.hpp) | フェーズ内（PlayerTestPhase、AttachmentSystem の後） | `Collider+Attack` と `Collider+Hp`（`Invincible` を除く）の間でカプセル重なり検出 → Hp 減算。双方が `Team` を持ち値が等しいヒットはスキップする（片方でも持たなければ従来どおり当たる）。攻撃側本体（ヒットボックスの `Hierarchy` 親、親を持たなければ攻撃側自身）を解決し、`Attack` の `hitstopSec`/`reaction` の写しとともに新たに成立したヒットの `HitEvent` 配列を返す |
| [`HitReactionSystem::Apply`](../Ash2/src/System/HitReactionSystem.hpp) | フェーズ内（PlayerTestPhase、HitSystem の後） | `HitSystem::Update` が返した `HitEvent` ごとに、攻撃側本体と被弾側へ `Hitstop` を付与し、被弾側が持つモーション variant（`EnemyMotion::Variant`/`PlayerMotion::Variant`）に応じて `ApplyEnemyReaction`/`ApplyPlayerReaction`（匿名名前空間）へ分岐する。前者は `EnemyMotion::Variant` と `Velocity` を、後者は `PlayerMotion::MakeDamaged` 経由で `PlayerMotion::Variant` を、下記「リアクションの対応」に従って強制遷移させる |
| [`ProjectileSystem::Update`](../Ash2/src/System/ProjectileSystem.hpp) | フェーズ内（PlayerTestPhase、HitReactionSystem の後） | Projectile の着弾（hitTargets 非空）/ 画面外での破棄 |
| [`EnemySystem::Update`](../Ash2/src/System/EnemySystem.hpp) | フェーズ内（PlayerTestPhase、ProjectileSystem の後） | `EnemyMotion::Defeated` の残り時間が尽きたエンティティを収集し、`MotionSystem` のビュー走査外でまとめて破棄する |
| [`FadeOutSystem::Update`](../Ash2/src/System/FadeOutSystem.hpp) | フェーズ内（PlayerTestPhase、EnemySystem の後） | `FadeOut` の残り時間を減算して `DrawColor::color.a`（`get_or_emplace` で確保）に反映し、満了したエンティティを破棄する。`Hitstop` による除外はしない |
| [`AnimationSystem::Update`](../Ash2/src/System/AnimationSystem.hpp) | フェーズ内（各フェーズが直接呼出） | `Hitstop` を持たない SpriteAnimation の elapsed を進め、切り出した `TextureRegion` を `TextureDrawable` に反映する（`facingRight` なら反転）。`AnimationClip::loop` が false のクリップは最終コマで停止し、先頭へ戻らない |
| [`DrawSystem::Draw`](../Ash2/src/System/DrawSystem.hpp) | 毎フレーム（HudSystem の前） | WorldPos+Drawable を奥行き順にソートして描画。カメラは `Scene::Center()` の固定オフセットのみ（スクロールなし。`ProjectileSystem` の画面外判定も同じオフセットを使う）。`DrawColor`（未所持は白・不透明）を塗り色・テクスチャの乗算色として適用する。関数スコープに閉じた `ScopedRenderStates2D` で最近傍サンプラーを適用し、`TextureDrawable` の描画位置は `Math::Round` で整数化する（HUD・フォントには波及しない） |
| [`HudSystem::Draw`](../Ash2/src/System/HudSystem.hpp) | 毎フレーム（DrawSystem の後） | Player の Hp / Stamina を画面左上にゲージ描画（プレイヤー 1 体のみ想定）。他のシステムと異なり実装をヘッダに直書きしている |
| [`NameLookupSystem::Connect`](../Ash2/src/System/NameLookup.hpp) | 起動時 | Name 追加・削除時に NameLookup を自動同期するシグナル登録 |
| [`HierarchySystem::Connect`](../Ash2/src/System/HierarchySystem.hpp) | 起動時 | Hierarchy 削除時に Detach を自動呼び出しするシグナル登録 |

### 呼び出し順の制約

入れ替えると壊れる組み合わせ。フェーズの `update` に並べるときはこれを守る。

| 制約 | 理由 |
|---|---|
| `AttachmentSystem` は `MotionSystem`（子の `LocalOffset` 更新）より後、`HitSystem` より前 | 子の絶対座標が確定していないと判定がずれる |
| `ProjectileSystem` は `MovementSystem` と `HitSystem` の後 | 着弾判定を `Attack::hitTargets` の中身で行うため |
| `GravitySystem` の「加速」と「地面クランプ」は1つの関数に留める | 時間軸が違う（次フレーム用／今フレーム確定）。分割すると跳ね方が変わる |

### リアクションの対応

`Attack.reaction`（`ReactionLevel`）は各 `PlayerMotion` の `Tick()` が `PlayerMotion::UpdateAttackHitbox`
経由で固定値を割り当て、`HitReactionSystem::Apply` が被弾側が持つモーション variant
（`EnemyMotion::Variant`/`PlayerMotion::Variant`）に応じて遷移先を決める。

**`Enemy` の被弾側**

| `Attack.reaction` | 割り当て元 | 遷移先 |
|---|---|---|
| `None`（既定） | `Ranged` の弾 | 遷移しない（無反応） |
| `Stagger` | `MeleeChain` | `EnemyMotion::Stagger` |
| `Repel` | `DashAttack` / `AirAttack` | `EnemyMotion::Repel` |
| `Blow` | `MeleeFinisher` | `EnemyMotion::Knockback` |

`Hp` 枯渇時は `reaction` によらず `Collider`/`Hp` を除去して `EnemyMotion::Defeated` へ遷移する。
`Repel`/`Knockback` の `Velocity` は攻撃側本体との `WorldPos.w` 比較で向きを決める。

**`Player` の被弾側**

| `Attack.reaction` | 遷移先 |
|---|---|
| `None` | 遷移しない（ダメージのみ） |
| `Stagger` / `Repel` | `PlayerMotion::Stagger` |
| `Blow` | `PlayerMotion::Knockback` |

空中で被弾した場合は `reaction` によらず `PlayerMotion::Knockback` になる（空中の仰け反りは
設計に無く、`Downed` を経由しないと落下後の復帰先が決まらないため）。`Knockback` の
`Velocity` は攻撃側本体との `WorldPos.w` 比較で向きを、`PlayerConfig::DamageConfig` で
大きさを決める。

### システム付随の型・関数

| 名前 | 役割 |
|---|---|
| [`HitEvent`](../Ash2/src/System/HitSystem.hpp) | 成立したヒット1件の情報（被弾側 `target`・攻撃側本体 `attackerOwner`・ヒット成立時点の `hitstopSec`/`reaction` の写し）。`HitSystem` → `HitReactionSystem` の受け渡しに使う。攻撃側のエンティティ自体は保持せず、被弾処理で `Attack` が外れても引き直さずに済む形にしている |
| [`MotionState`](../Ash2/src/System/MotionSystem.hpp) | variant `M` の状態型 `S` が満たすべきコンセプト（ADL で解決される `Tick()` が `Optional<M>` を返すこと） |
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
`Ash2/src/System/PlayerMotion/` 配下に状態ごとに分かれている（対応は上記「モーション状態型」の
「実装」列を参照）。状態別 `.cpp` は自身の `Tick()` と入場関数の定義を持つ。被弾4状態
（`Stagger`/`Knockback`/`Downed`/`GetUp`）のみ `Damaged.cpp` 1ファイルにまとめる（被弾から
復帰までを1本の流れとして読める方を優先し、各状態が `timer` 1つ以下しか持たない単純さは
`EnemyMotionSystem.cpp` と同じ）。

| 名前 | 役割 |
|---|---|
| [`Helper.hpp`](../Ash2/src/System/PlayerMotion/Helper.hpp) | 複数状態が使う共通ヘルパー |
| [`Transition.hpp`](../Ash2/src/System/PlayerMotion/Transition.hpp) | 状態遷移用の入場関数の宣言（定義は遷移先の状態の `.cpp` が持つ） |

### プレイヤーモーションの補助部品

| 名前 | 役割 | 定義 |
|---|---|---|
| `HitboxSpec` | 攻撃判定エンティティの生成仕様（半径・ダメージ・リアクション・ヒットストップ時間・フェード時間・描画有無 `drawOrb`）をまとめた構造体。近接だけ `drawOrb = false` を渡し、見た目を光エンティティ側に分離する | `Helper.hpp` |
| `LightSpec` | 見た目だけを担う光エンティティ群の生成仕様（数・半径・フェード時間） | `Helper.hpp` |
| `SetClip` | クリップが変化していれば差し替え、再生位置をリセットする | `Helper.hpp`/`.cpp` |
| `StopHorizontalMovement` | 横方向（w・d）の速度を 0 にする | `Helper.hpp`/`.cpp` |
| `ReleaseAttackHitbox` | ヒットボックス（判定・光いずれも）を `Hierarchy::Detach` → `Attack`/`Collider` 除去 → `FadeOut` 付与の順で解放する。`fadeSec` が 0 以下なら即座に破棄する。光は元々 `Attack`/`Collider` を持たないため切り離しとフェード付与だけが働く | `Helper.hpp`/`.cpp` |
| `UpdateAttackHitbox` | active 区間に応じて攻撃判定エンティティを生成・`LocalOffset` 更新し、後隙入りで `ReleaseAttackHitbox` を呼ぶ。オフセットは `offsetFn(progress)` で決まる | `Helper.hpp`/`.cpp` |
| `UpdateAttackLights` | active 区間に応じて光エンティティ群を生成・`LocalOffset` 更新し、後隙入りで解放する。オフセットは `offsetFn(progress, index)` で決まる | `Helper.hpp`/`.cpp` |
| `ReleaseMotionEntities` | 状態が `hitboxEntity`/`lightEntities` を持つ場合のみ `ReleaseAttackHitbox` で解放する（`requires` で有無を判定するテンプレート、`Variant` を受ける `std::visit` 版も持つ）。被弾による強制遷移の後始末に使う | `Helper.hpp`/`.cpp` |
| `MakeMeleeChain` | 指定段の `MeleeChain` を生成（`melee_{stage+1}` クリップを先頭から再生） | `Transition.hpp` / `Melee.cpp` |
| `MakeMeleeFinisher` | `MeleeFinisher` を生成（`melee_finish` クリップを先頭から再生） | `Transition.hpp` / `Melee.cpp` |
| `MakeRanged` | `Ranged` を生成（スタミナ消費、`timer` は `RangedConfig::recoverySec`） | `Transition.hpp` / `Ranged.cpp` |
| `MakeDash` | `Dash` を生成（スタミナ消費、`air` フラグ設定） | `Transition.hpp` / `Dash.cpp` |
| `MakeDashAttack` | `DashAttack` を生成（`air`・`dashDir` を引き継ぐ） | `Transition.hpp` / `DashAttack.cpp` |
| `MakeAirAttack` | `AirAttack` を生成 | `Transition.hpp` / `AirAttack.cpp` |
| `SpawnProjectile` | 遠距離攻撃の弾エンティティを生成する（`WorldPos`+`Velocity`+`Collider`+`Attack`+`Drawable`+`Projectile`+`Team::Player`） | `Transition.hpp` / `Ranged.cpp` |
| `MakeDamaged` | 被弾による強制遷移を生成する。上書き前の `Motion` を値で取得して `ReleaseMotionEntities` で後始末し、`Velocity` リセットと `Invincible` 除去のうえで `reaction`・接地状態に応じ `Stagger`/`Knockback` を返す | `Transition.hpp` / `Damaged.cpp` |

---

## フェーズシステム

`PhaseStack` がスタックで `IPhase` を管理。各フレームで先頭フェーズの `update()` を呼び、返り値の `PhaseCommand` でスタックを操作する。

### 基盤

| クラス | 役割 |
|---|---|
| [`IPhase`](../Ash2/src/Phase/IPhase.hpp) | フェーズ基底クラス。`onAfterPush` / `update` / `onBeforePop` と `PhaseCommand` を定義する |
| [`PhaseStack`](../Ash2/src/Phase/PhaseStack.hpp) | フェーズのスタック管理。最前面のみ更新し、`PhaseCommand` に従い操作する |

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
| [`PlayerTestPhase`](../Ash2/src/Phase/PlayerTestPhase.hpp) | `player_test` | プレイヤー操作・物理・アニメーションのビジュアルテスト。`EnemyConfig` から敵（`Enemy`+`EnemyMotion::Variant`+`Collider`+`Hp` 等）を1体生成し、`HitReactionSystem`/`EnemySystem` に被弾リアクション・撃破後の破棄を委ねる。敵が破棄されたら `EnemyConfig::respawnSec` 後に再生成する。F5 でプレイヤー・設定再生成、Esc で Pop |
| [`AnimationViewerPhase`](../Ash2/src/Phase/AnimationViewerPhase.hpp) | `animation_viewer` | アニメーションクリップ単体確認（←→切替、F反転、Rでリプレイ、Esc で Pop） |
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
`PhaseLoader` は `LoadedPhase`（`maker` と、別セクションを参照する場合のみ設定される
`Optional<String> referencedSection`）を返す。`SectionRefParam` コンセプトを満たす
`Param`（`sectionName` を持つ、`ScenarioPhase::Param` など）は `MakeLoader` が自動で
`referencedSection` へ詰める。
`ScenarioData::FromToml` はこのテーブルを `PhaseLoaderTable` として引数で受け取る。
呼び出し元は `GameSetup` が `GetPhaseLoaders()` を渡して配線する。
`FromToml` は全セクションを登録し終えた後、集めた `referencedSection` が実在するセクションを
指すかまとめて検証する（前方参照を許すため、セクションごとのループの外で行う）。不在なら
`unexpected` を返し、`ScenarioPhase::update` がゲームループ内で `at()` の例外を投げることはない。

---

## 設定（Config）

`Ash2/src/Config/` 配下。TOML から `FromToml()` で構築し、`registry.ctx()` に格納する。
`PlayerConfig`/`EnemyConfig`/`AnimationData`/`ScenarioData` はいずれも、`FromToml` 自身が
`std::expected<T, String>` を返す。必須キーの欠落は失敗として伝播し、終了を決めるのは
呼び出し元（`Run()` が `FatalError{FatalReason::ConfigInvalid, ...}` に変える）である。

[`TomlFields`](../Ash2/src/Config/TomlFields.hpp) は1テーブル分の必須キー読み出しと
欠落キーの記録をまとめるヘルパーで、上記3設定の `FromToml`（`PlayerConfig` は下位の
`Parse` 系関数）が共通で使う。

### [`PlayerConfig`](../Ash2/src/Config/PlayerConfig.hpp)

- `Ash2/App/assets/config/player.toml` から読み込むプレイヤー設定
- 基本値（移動速度 `speed`・ジャンプ初速 `jumpSpeed`・重力 `gravity`・当たり判定カプセルの
  `capsuleRadius`/`capsuleHeight`）と、`MeleeConfig` / `RangedConfig` / `DashConfig` /
  `DashAttackConfig` / `AirAttackConfig` / `StaminaConfig` / `LandingConfig` /
  `AttackEffectConfig` / `DamageConfig` の各サブ設定を持つ
- 地上・空中を共有する `Dash`/`DashAttack`（`air` フラグで区別）は、それぞれ単一の
  `DashConfig`/`DashAttackConfig` を共通で参照する（専用設定は持たない）

同ヘッダが定義する部品：

| 名前 | 役割 |
|---|---|
| `MotionTimeline` | 攻撃・ダッシュ系共通の4区間タイムライン（windup / active / 後隙A＝キャンセル不可 / 後隙B＝キャンセル可）。`activeStart`/`activeEnd`/`recoveryAEnd`/`recoveryBEnd` と `isActive`/`isCancelable`/`isFinished`/`activeProgress` を提供する。`DashConfig`/`DashAttackConfig`/`AirAttackConfig`/`MeleeSwingConfig` が共通で持つ |
| `MeleeTrajectory` | 近接攻撃の軌道パターン（`Thrust` 突き出し / `Slash` 斬り上げ。`Slash` は `slashCurve` で弧の曲がり具合を指定し、0 なら直線になる） |
| `MeleeSwingConfig` | 近接1振り分の共通設定（`timeline`/`radius`/`trajectory`/`slashRiseHeight`/`slashCurve`/`hitstopSec`）。継続段・締め段の両方が持つ |
| `MeleeFinisherConfig` | 締め段の設定。`MeleeSwingConfig swing` を集約し、見た目の光の数 `lightCount`（2以上、parse 時に検証）と間隔 `lightGap` を足す |
| `MeleeConfig` | 段共通のパラメータ（`capMidH`/`reach`/`damage`）と継続段配列 `chain`（先頭が1段目）・締め段 `finisher` |
| `RangedConfig` | リーチ・半径・ダメージ・弾速・発射高さ・スタミナ消費・発射後の硬直時間 `recoverySec` |
| `DashConfig` | 速度・タイムライン・スタミナ消費 |
| `DashAttackConfig` | タイムライン・突進速度・軌道半径（w-d 平面）・カプセル半径・ダメージ・ヒットストップ時間 |
| `AirAttackConfig` | タイムライン・ドリフト移動速度倍率（地上ニュートラル速度 `speed` に対する `driftRatio`）・軌道半径（w-h 平面）・軌道の開始角/終了角（度、`orbitStartDeg`/`orbitEndDeg`。0°が正面・-90°が頭上・90°が真下・180°が真後ろ）・カプセル半径・ダメージ・ヒットストップ時間 |
| `StaminaConfig` | 回復開始待機秒数 `recoveryDelay`・毎秒の不足分回復割合 `recoveryRate` |
| `LandingConfig` | 着地硬直時間 `recoverySec` |
| `AttackEffectConfig` | ヒットボックス解放後のフェードアウト時間 `fadeSec`（全攻撃共通の1値。`HitboxSpec::fadeSec` として渡される） |
| `DamageConfig` | 被弾リアクションの設定値（仰け反り時間 `staggerSec`、吹き飛ばし初速 `knockbackSpeedW`/`knockbackSpeedH`、ダウン時間 `downSec`、起き上がり時間 `getUpSec`） |

- `hitstopSec`（`MeleeSwingConfig`/`DashAttackConfig`/`AirAttackConfig` が個別に持つ）はヒット成立時に
  `Attack.hitstopSec` へ渡す停止時間で、段・アクションごとに調整できる（`RangedConfig` は持たない。
  弾は `reaction` が `None` で無反応の仕様のため）
- 近接攻撃はスタミナを消費しない（`MeleeConfig` は `staminaCost` を持たない）
- `[[melee.chain]]` が0件（欠落含む）の場合、`FromToml` は失敗を返す
  （`Tick(MeleeChain&, ...)` の `chain[state.stage]` アクセスを不正にしないため）

### [`EnemyConfig`](../Ash2/src/Config/EnemyConfig.hpp)

- `Ash2/App/assets/config/enemy.toml` から読み込む敵設定
- ステータス・形状（`maxHp`/`size`/`capsuleRadius`/`capsuleHeight`/`spawnW`）と、`EnemyMotion`
  各状態の演出パラメータ（`staggerSec`、`repelSpeed`/`repelSec`、`blowSpeedW`/`blowSpeedH`/
  `knockbackSec`、`defeatedSec`/`respawnSec`）を持つ
- `Knockback` の重力加速度は専用の値を持たず、`PlayerConfig::gravity` を敵にもそのまま付与して
  流用する（`PlayerTestPhase::spawnEnemy` 参照）
- 色は toml 化せず `PlayerTestPhase.cpp` 側の定数（`kDummyColor`）に残す（パーサを増やさないため）
- リアクション Lv（`ReactionLevel`）自体は config 化せず、各 `PlayerMotion` の `Tick()` が固定値で
  割り当てる。スタミナ連動の降格表を含む config 化は #233 のスコープ

### [`AnimationData`](../Ash2/src/Config/AnimationData.hpp)

- スプライトシート単位のアニメーション共有データ（テクスチャキー・コマサイズ・描画オフセット・
  クリップ表）
- `AnimationClip` は行番号・コマ数・再生速度（コマ/秒）・ループ有無（`loop`、既定 false）を持ち、
  位相の更新（`advance`）とコマ算出（`columnAt`）、1周の時間（`cycleDuration`）を自身のメンバ関数で
  提供する
- `AnimationDataRegistry`（キー: 設定ファイルのベース名）として
  `Ash2/App/assets/config/animation/*.toml` から一括ロードされる
- `texture`/`width`/`height`/`draw_offset` は予約キーで、それ以外のテーブルがクリップとして扱われる

### [`ScenarioData`](../Ash2/src/Config/ScenarioData.hpp)

- `Ash2/App/assets/config/scenario.toml` から読み込むシナリオ（セクション名 → ステップ列）
- 同ヘッダが `IPhaseMaker` / `LoadedPhase` / `PhaseLoader` / `PhaseLoaderTable` / `StepPush` /
  `StepReset` / `ScenarioStep` / `kInitSectionName` を定義する
- `kInitSectionName`（`U"init"`）は起動時に最初へ渡すセクション名。`Main.cpp` の起動フェーズ生成と
  `InitializeRegistry` の存在確認が同じ定数を参照する
- 詳細は上記「フェーズシステム」を参照

### `registry.ctx()` の内容一覧

`InitializeRegistry()` が起動時にセットする。

| 型 | 用途 |
|---|---|
| `NameLookup` | 名前 → エンティティの逆引きテーブル |
| [`UiFonts`](../Ash2/src/UiFonts.hpp) | UI 描画用フォント一式（`large`/`small`） |
| `PlayerConfig` | プレイヤー設定 |
| `EnemyConfig` | 敵設定 |
| `AnimationDataRegistry` | アニメーション共有データ |
| `ScenarioData` | シナリオデータ |

---

## 入力抽象化

| クラス | 役割 |
|---|---|
| [`InputState`](../Ash2/src/Input/InputState.hpp) | フレームの論理入力（`moveAxis`/`jumpDown`/`attackDown`/`rangedAttackDown`/`dashDown`）。`Key`/`TOMLValue` 等のテストしづらい型は持ち込まないが、`Vec2` 等の単純な数学型は許容する |
| [`KeyboardInputAction`](../Ash2/src/Input/KeyboardInputAction.hpp) | キーボード/マウス → InputState 変換（デフォルト: 矢印/WASD 移動、Space ジャンプ、左クリック近距離、右クリック遠距離、Shift ダッシュ） |
| [`XInputAction`](../Ash2/src/Input/XInputAction.hpp) | XInput コントローラー（プレイヤー0）→ InputState 変換（左スティック+十字ボタンで移動、A/B/X/Y でジャンプ/近距離/遠距離/ダッシュ） |
| [`InputDeviceSelector`](../Ash2/src/Input/InputDeviceSelector.hpp) | 毎フレームデバイス入力を検出し、最後にアクティブだったデバイスに切り替える（ボタン入力に加え、左スティックがデッドゾーンを超えて傾いた場合もゲームパッドへの切り替え条件とする）。切断時はキーボードへ自動フォールバック |

**移動入力の正規化方針：** `InputState::moveAxis`（`Vec2`、x=横方向/y=奥行き方向）は「常に長さ 1.0
以下に正規化済み」という不変条件を持つ。この保証の責任は `toInputState()` を実装する各入力レイヤー側に
あり、`PlayerMotion::Tick(Neutral&, ...)` は無条件にこの値を信頼してそのまま速度計算に使う
（System 側で正規化やクランプを行わない）。`XInputAction` は左スティックのデッドゾーン定数
（`kLeftThumbDeadZone`、`InputDeviceSelector` も参照する公開 `static constexpr` メンバ）を適用し、
十字ボタンの軸ベクトルと加算したうえで `limitLength(1.0)` により正規化する。

**フェーズの直接キー入力：** `TestMenuPhase`（↑↓/Enter）・`PlayerTestPhase`（Esc）・
`AnimationViewerPhase`（Esc/←→/F/R）は `InputState` を経由せず Siv3D の `Key*` を直接参照している。
これらはフェーズ内部状態の操作であり、フェーズ内に閉じているためキーが衝突してもよい。
デバイス差の吸収が必要な操作は `InputState` へ、Release に存在しないデバッグ限定機能とその
キー判定は `DebugOnly` へ置く（下記「基盤・ユーティリティ」参照）。

---

## アセット管理（[Asset.hpp](../Ash2/src/Asset.hpp)）

アセットの実体は `Ash2/App/assets/` 配下。下記2行の `assets/asset_list` は実行時に参照されるパス文字列そのもの。

- デバッグ: `assets/asset_list` をファイルから読む
- リリース: `assets/asset_list` を埋め込みリソースから読む
- `.png` → `TextureAsset`、`.mp3` → `AudioAsset` としてキー（相対パス）で登録
- アニメーション設定: `Ash2/App/assets/config/animation/*.toml`（起動時に全ファイルをスキャン）
- `asset_list` を開けない場合は起動しない（`GetAssetList` が `std::expected` で失敗を返し、
  `Run()` が `FatalError{FatalReason::AssetMissing, ...}` に変えて投げる）
- TOML の読み込みは `OpenToml` を単一の入口とする。開けない場合はパスを含むメッセージの
  `std::expected` で失敗を返す（中身が空の TOML は「開けている」として扱い、キー欠落は
  各 `FromToml` 側の失敗として区別する）
- アニメーション設定の `texture` キーはロード時（`LoadAnimations`）に
  `TextureAsset::IsRegistered`（未登録の検出）と `TextureAsset::Load`（実体読み込みの確定）で
  検証する。タイプミスや `asset_list` の漏れを起動時に検出するため、`AnimationSystem::Update`
  側では毎フレームの確認をしない

---

## 基盤・ユーティリティ

| 名前 | 役割 |
|---|---|
| [`Main`](../Ash2/src/Main.cpp) | アプリの入口。アセット登録 → `Scene::SetTextureFilter(TextureFilter::Nearest)` → registry 初期化 → `PhaseStack` を生成し、毎フレーム `PhaseStack::update` → `AttachmentSystem` → `DrawSystem` → `HudSystem` を回す。`RegisterAssets` の失敗は `FatalError{FatalReason::AssetMissing, ...}` に、`InitializeRegistry` の失敗は `FatalError{FatalReason::ConfigInvalid, ...}` に変えて投げる。例外は `FatalError` / `s3d::Error` / `std::exception` / `...` の4種を捕捉し `ExitWithFatal` へ渡す。起動時に `DebugOnly::RunTestsIfRequested` を呼び、Debug ビルドで環境変数 `ASH2_RUN_TESTS` が設定されていれば Catch2 のテストのみ実行し、成否を終了コードに反映して終了 |
| [`FatalError`](../Ash2/src/FatalError.hpp) | 続行できない失敗を表す型。分類（`FatalReason`）と開発者向けの `detail` を持つ |
| [`ExitWithFatal`](../Ash2/src/CrashHandler.hpp) | 致命エラーを `crash.log` に記録し、Release では分類に応じた文言を表示して終了する |
| [`ExitImmediately`](../Ash2/src/CrashHandler.hpp) | 標準出力を流してから `std::_Exit` でプロセスを終了する。致命エラー終了とテスト実行後の終了で共有する |
| [`InitializeRegistry`](../Ash2/src/GameSetup.hpp) | `registry.ctx()` へ `NameLookup` / `UiFonts` / 各 Config / `AnimationDataRegistry` / `ScenarioData` を登録し、シグナルを接続する。`std::expected<void, String>` を返し、失敗を呼び出し元（`Main`）へ渡す |
| [`LoadAnimations`](../Ash2/src/GameSetup.hpp) | アニメーション設定 TOML を全件読み込み `AnimationDataRegistry` を返す。`InitializeRegistry` と `DebugOnly.cpp`（無名名前空間の `ReloadConfig`）の両方から呼ばれる |
| [`DebugOnly`](../Ash2/src/DebugOnly.hpp) | Debug ビルドにのみ存在する機能とそのキー判定の集約。`RunTestsIfRequested`（`ASH2_RUN_TESTS` によるテスト実行）・`OpenDebugConsole`・`UpdateConfigReload`/`IsConfigReloadRequested`（F5 設定リロード。失敗時は旧データを維持したまま `APP_LOG` に出して戻る）・`ApplyHitReactionTest`/`ClearHitReactionTest`（Key1/2/3 による被弾リアクション仮付与、`PlayerTestPhase` 用）を持つ。Release ビルドでは全関数が空の inline 関数になる |
| [`GetAssetList`](../Ash2/src/Asset.hpp) | `Ash2/App/assets/asset_list` を読んでアセットパス一覧を返す。`std::expected<Array<FilePath>, String>` を返し、開けなければ失敗を返す |
| [`AssetPath`](../Ash2/src/Asset.hpp) | Debug では `FilePath`、Release では `Resource` パスを返す |
| [`RegisterAssets`](../Ash2/src/Asset.hpp) | `.png`/`.mp3` をアセットシステムに登録する。`std::expected<void, String>` を返し、失敗を呼び出し元（`Main`）へ渡す |
| [`OpenToml`](../Ash2/src/Asset.hpp) | `AssetPath()` を通してアセット配下の TOML を開く。`std::expected<TOMLReader, String>` を返し、開けなければパスを含むメッセージを返す |
| [`UiFonts`](../Ash2/src/UiFonts.hpp) | UI 描画に使うフォント一式（`large`/`small`）。`Create()` が `std::expected<UiFonts, String>` を返し、`InitializeRegistry` が `registry.ctx()` に登録する |
| [`APP_LOG`](../Ash2/src/Debug.hpp) | Debug ビルドで `Console` に出力するログマクロ（Release では何もしない） |
| [`FrameData`](../Ash2/src/FrameData.hpp) | フレームごとの更新データ（`dt` + `InputState`）。`Main` が組み立て、フェーズとシステムの双方が受け取る |
| [`AppDebug::testMode`](../Ash2/src/Debug.hpp) | テスト実行中フラグ。true の間 `APP_LOG` を無効化する |
| [`Overloaded`](../Ash2/src/Util/Overloaded.hpp) | 複数のラムダを1つの visitor にまとめる `std::visit` 用ヘルパー |

---

## アニメーションクリップ名

`Ash2/App/assets/config/animation/player.toml` のクリップ名としてコードから参照されるもの。
クリップ名の欠落は `AnimationSystem` の `assert` で検出される。

| クリップ名 | 使用箇所 |
|---|---|
| `idle` | `Neutral`（静止時） |
| `move` | `Neutral`（移動時） |
| `jump_rise` / `jump_fall` | `Neutral`（上昇中／落下中） |
| `melee_1` / `melee_2` | `MakeMeleeChain`（段番号 +1 で決まる） |
| `melee_finish` | `MakeMeleeFinisher` |
| `ranged_attack` | `MakeRanged` |
| `dash` | `MakeDash` |
| `dash_attack` | `MakeDashAttack` |
| `air_attack` | `MakeAirAttack` |
| `landing` | `Landing` の `Tick()` |
| `stagger` | `MakeDamaged`（`Stagger` へ遷移する場合） |
| `knockback` | `MakeDamaged`（`Knockback` へ遷移する場合） |
| `downed` | `Knockback` の `Tick()`（`Downed` へ遷移する瞬間） |
| `get_up` | `Downed` の `Tick()`（`GetUp` へ遷移する瞬間） |

---

## テスト

`Ash2/tests/` に Catch2 のテストを置く。実行方法は [TEST.md](TEST.md) を参照。

| テストファイル | 対象 |
|---|---|
| `TestWorldPos.cpp` | `WorldPos` の座標変換・接地判定、`DrawOrderLess` |
| `TestMovementSystem.cpp` | `MovementSystem` |
| `TestAttachmentSystem.cpp` | `AttachmentSystem` の座標伝播、`Hierarchy` の連結リスト操作 |
| `TestHitSystem.cpp` | `HitSystem` のカプセル交差・重複ヒット防止・root 解決・`Team` による同陣営スキップ |
| `TestHitReactionSystem.cpp` | `HitReactionSystem` のリアクション適用・ヒットストップ付与 |
| `TestHitstopSystem.cpp` | `HitstopSystem` |
| `TestPlayerMotionSystem.cpp` | プレイヤー各状態の `Tick()` |
| `TestEnemyMotionSystem.cpp` | 敵各状態の `Tick()`、`EnemySystem` |
| `TestFadeOutSystem.cpp` | `FadeOutSystem` の `DrawColor::color.a` 減衰・満了時の破棄 |
| `TestProjectileSystem.cpp` | `ProjectileSystem` の消滅条件 |
| `TestNameLookup.cpp` | `NameLookupSystem` のシグナル同期 |
| `TestPlayerConfig.cpp` | `PlayerConfig::FromToml` |
| `TestEnemyConfig.cpp` | `EnemyConfig::FromToml` |
| `TestAnimationData.cpp` | `AnimationData::FromToml` |
| `TestScenarioData.cpp` | `ScenarioData::FromToml` |
| `TestPhaseStack.cpp` | `PhaseStack` の push / pop / reset |
| `TestWaitPhase.cpp` | `WaitPhase` |

---

## 部品追加時の注意

- `Hierarchy` のメンバは必ず static メンバ関数（Attach/Detach/DestroyWithChildren）経由で操作する。
- `Drawable` の型変更は `std::visit` を使い、DrawSystem と AnimationSystem の両方への影響を確認する。
- 図形（`Drawable`）に `DrawColor` を付け忘れると白（`kDefaultDrawColor`）で描かれる。意図した色にしたい場合は忘れず付与すること。
- 新クラス追加時は `Ash2.vcxproj` と `Ash2.vcxproj.filters` にも追加が必要。
- `NameLookup` への挿入・削除は `NameLookupSystem::Connect` で自動化されている（`Name` コンポーネントの追加・削除に連動）。手動での `NameLookup[key] = entity` 登録は不要。
- `PlayerMotion::Variant`/`EnemyMotion::Variant` に新しい状態型を追加したときは、その型に `Tick(state, registry, entity, frameData) -> Optional<M>`（`M` は追加先の variant 型、ADL で解決される非修飾 `Tick`）を実装する必要がある。`MotionSystem::Update` の `std::visit` が `MotionState<S, M>` concept（`MotionSystem.hpp`）で制約されているため。
- `Tick` の満了時に後始末（`Velocity` のクリア等）をする状態を追加したときは、`HitReactionSystem` 側の代替処理も確認する。被弾による強制遷移では満了時の後始末が飛ばされる。
- 新しい Config を追加し F5 で差し替えたいときは、`GameSetup::InitializeRegistry` だけでなく `DebugOnly.cpp`（無名名前空間の `ReloadConfig`）にも追加する。
- アセットファイルを追加・削除したときは `tools/sync-assets.sh` を実行する。`Ash2/App/assets/asset_list` と `Ash2/App/Resource.rc` はその生成物で、手書きしない。
- 新フェーズを追加し TOML から `push`/`reset` できるようにするときは、`Phase/PhaseLoaders.cpp` の `GetPhaseLoaders()` にもエントリを追加する。
- 新しいアニメーションクリップを参照するときは `Ash2/App/assets/config/animation/*.toml` 側にも追加する（欠落は `AnimationSystem` の `assert` で落ちる）。
- アニメーションクリップは既定で一発再生（最終コマで停止）。ループさせたいクリップにのみ `loop = true` を明記する。
- 攻撃判定・被弾判定を持つエンティティ（本体・ヒットボックス・弾）には `Team` を付与する。持たない側は `HitSystem` の同陣営スキップに参加せず、静かに当たる（`assert` では捕まえない仕様）。新しい攻撃エンティティの生成時は `Team` を付与する。
