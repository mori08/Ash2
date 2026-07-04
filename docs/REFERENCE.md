# REFERENCE.md

「沼に焚べ」の部品一覧。設計意図・レイヤー構成・座標系などは [ARCHITECTURE.md](ARCHITECTURE.md) を参照。

行数上限なし。機能追加のたびに更新する前提のドキュメント。

---

## 入力抽象化

| クラス | 役割 |
|---|---|
| [`InputState`](../Ash2/src/Input/InputState.hpp) | フレームの論理入力（`Key`/`TOMLValue` 等のテストしづらい型は持ち込まないが、`Vec2` 等の単純な数学型は許容） |
| [`KeyboardInputAction`](../Ash2/src/Input/KeyboardInputAction.hpp) | キーボード/マウス → InputState 変換 |
| [`XInputAction`](../Ash2/src/Input/XInputAction.hpp) | XInput コントローラー → InputState 変換（左スティック+十字ボタンで移動、A/B/X/Y でジャンプ/近距離/遠距離/ダッシュ） |

[`InputDeviceSelector`](../Ash2/src/Input/InputDeviceSelector.hpp) が毎フレームデバイス入力を検出し、最後にアクティブだったデバイスに切り替える（ボタン入力に加え、左スティックがデッドゾーンを超えて傾いた場合もゲームパッドへの切り替え条件とする）。切断時はキーボードへ自動フォールバック。

**移動入力の正規化方針：** `InputState::moveAxis`（`Vec2`、x=横方向/y=奥行き方向）は「常に長さ 1.0 以下に正規化済み」という不変条件を持つ。この保証の責任は `toInputState()` を実装する各入力レイヤー側にあり、`PlayerMotion::Tick(Neutral&, ...)` は無条件にこの値を信頼してそのまま速度計算に使う（System 側で正規化やクランプを行わない）。`XInputAction` は左スティックのデッドゾーン定数（`LeftThumbDeadZone`、`InputDeviceSelector` も参照する公開 `static constexpr` メンバ）を適用し、十字ボタンの軸ベクトルと加算したうえで `limitLength(1.0)` により正規化する。

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
| [`Attack`](../Ash2/src/Component/Attack.hpp) | 攻撃中タグ兼攻撃力（`Collider` と組み合わせて攻撃判定が有効になる）。フィールドの詳細は下記「複雑なコンポーネントの詳細」参照 |
| [`Hp`](../Ash2/src/Component/Hp.hpp) | HP（`Collider` と組み合わせて被弾判定の対象になる） |
| [`Stamina`](../Ash2/src/Component/Stamina.hpp) | スタミナ（max / current の int フィールド、StaminaSystem が管理する回復端数累積 accum と回復ディレイ計測用 recoveryTimer を持つ） |
| [`Projectile`](../Ash2/src/Component/Projectile.hpp) | 飛翔体（弾）タグ（データなし）。`WorldPos`+`Velocity`+`Collider`+`Attack` と組み合わせ、`MovementSystem` が移動を、`ProjectileSystem` が消滅を管理する対象を識別する |
| [`Motion`](../Ash2/src/Component/Motion.hpp) | エンティティの排他的な行動状態（`variant`）。状態ごとの詳細は下記「複雑なコンポーネントの詳細」参照 |
| [`Gravity`](../Ash2/src/Component/Gravity.hpp) | 重力の影響を受けるエンティティに付与する重力加速度 |
| [`Hitstop`](../Ash2/src/Component/Hitstop.hpp) | ヒットストップ中であることを示す残り時間タイマー。`HitstopSystem` が減算・除去し、付与中は `MotionSystem`/`MovementSystem`/`GravitySystem`/`AnimationSystem` の対象から除外される（暫定実装、本格化は #132/#134） |
| [`Stagger`](../Ash2/src/Component/Stagger.hpp) | ひるみリアクション中であることを示すタイマー。`StaggerSystem` が `RectDrawable::size` を縮小させ、残り時間が尽きたら `originalSize` に戻す（暫定実装、本格化は #134） |
| [`Invincible`](../Ash2/src/Component/Invincible.hpp) | 無敵状態であることを示すタグ。`HitSystem` の被弾対象ビューから除外される。`PlayerMotion::Dash` が構え・ダッシュ中は毎フレーム付与し、後隙入りで除去する |

### 複雑なコンポーネントの詳細

#### `Attack`

攻撃中タグ兼攻撃力。`Collider` と組み合わせて攻撃判定が有効になる。

- `root`: 複数コライダー構成のルートを指定する
- `hitTargets`: 攻撃生存期間中の重複ヒットを防ぐ
- `hitstopSec`: ヒット成立時に攻撃側・被弾側へ付与するヒットストップ時間

#### `Motion`

エンティティの排他的な行動状態（`std::variant<PlayerMotion::Neutral, Melee1, Melee2, Melee3, Ranged, Dash, DashAttack, AirAttack, Landing>`）。

- **Ranged**: 再生中クリップの残り時間を持つ
- **Melee1 / Melee2 / Melee3**: コンボの段ごとに分けた型。モーション開始からの経過時間（`elapsed`）・攻撃判定の子エンティティ（`hitboxEntity`）を持つ。`Melee1`/`Melee2` は次段への遷移予約フラグ（`comboQueued`）を持つが、締め技の `Melee3` はコンボ継続を持たずタイマー満了で `Neutral` へ戻るのみ
- **Dash**: 構え・ダッシュ・後隙A・後隙Bの4区間を `elapsed` 1本で管理する。ダッシュ中・後隙A・B中の攻撃入力（`attackDown`）でダッシュ攻撃を予約（`dashAttackQueued`）し、後隙B中に `DashAttack` へ遷移する。後隙B中のダッシュ入力（`dashDown`）は再ダッシュにキャンセルする。ダッシュ移動中の方向を `lastDashDir` に記録し `DashAttack::dashDir` へ引き渡す
- **DashAttack**: 構え・攻撃・後隙の3区間を持ち、攻撃判定（`hitboxEntity`）を w-d 平面の円軌道上で更新する
- **AirAttack**: `Neutral` が空中（`!WorldPos::isOnGround()`）で攻撃入力を受けたときに入場する。構え・攻撃・後隙の3区間を持ち、攻撃判定（`hitboxEntity`）を w-h 平面（垂直面）の円軌道上で更新する。後隙中も含め毎フレーム接地を検出し、接地した時点で（残っていればヒットボックスを破棄したうえで）`Landing` へ強制遷移する。接地せずに後隙が満了した場合はタイマー満了で `Neutral` へ戻る。リアクション Lv2・スタミナ枯渇時の威力低下は `DashAttack` 同様に未実装（暫定のダメージ+ヒットストップのみ、本格対応は #134 のスコープ）
- **Landing**: 着地硬直のタイマー状態。空中アクションの接地検出から遷移する

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
| [`PlayerTestPhase`](../Ash2/src/Phase/PlayerTestPhase.hpp) | プレイヤー操作・物理・アニメーションのビジュアルテスト。`HitSystem::Update` が返す `HitPair` を見て攻撃側・被弾側へ `Hitstop`/`Stagger` を付与する（暫定実装） |
| [`AnimationViewerPhase`](../Ash2/src/Phase/AnimationViewerPhase.hpp) | アニメーションクリップ単体確認（←→切替、F反転） |
| [`WaitPhase`](../Ash2/src/Phase/WaitPhase.hpp) | 指定秒数待機して Pop |

[`Config/ScenarioData`](../Ash2/src/Config/ScenarioData.hpp) がシナリオロード時に各ステップを `IPhaseMaker`（型消去された `make() -> unique_ptr<IPhase>`）を持つ `ScenarioStep`（`StepPush`/`StepReset`）に変換する。
変換テーブルは [`Config/ScenarioData.cpp`](../Ash2/src/Config/ScenarioData.cpp) の `kPhaseLoaders` で定義されており、**新フェーズ追加時はここにもエントリを追加する必要がある。**

---

## システム一覧

| システム | タイミング | 処理 |
|---|---|---|
| [`AttachmentSystem::UpdateTransform`](../Ash2/src/System/AttachmentSystem.hpp) | 毎フレーム（フェーズ後）＋フェーズ内（PlayerTestPhase、GravitySystem の後・HitSystem の前） | Hierarchy ルートから子孫へ WorldPos 伝播。PlayerTestPhase では HitSystem が同フレーム内の最新座標（光の珠の LocalOffset 反映後）を見られるよう追加で呼び出す |
| [`DrawSystem::Draw`](../Ash2/src/System/DrawSystem.hpp) | 毎フレーム（最後） | WorldPos+Drawable を奥行き順にソートして描画 |
| [`AnimationSystem::Update`](../Ash2/src/System/AnimationSystem.hpp) | フェーズ内（各フェーズが直接呼出） | `Hitstop` を持たない SpriteAnimation の elapsed を進め Drawable を更新 |
| [`NameLookupSystem::Connect`](../Ash2/src/System/NameLookup.hpp) | 起動時 | Name 追加・削除時に NameLookup を自動同期するシグナル登録 |
| [`HierarchySystem::Connect`](../Ash2/src/System/HierarchySystem.hpp) | 起動時 | Hierarchy 削除時に Detach を自動呼び出しするシグナル登録 |
| [`HitSystem::Update`](../Ash2/src/System/HitSystem.hpp) | フェーズ内（攻撃入力時） | `Collider+Attack` と `Collider+Hp`（`Invincible` を除く）の間でカプセル重なり検出 → Hp 減算。新たに成立したヒットの `HitPair`（attacker/target）配列を返す |
| [`HitstopSystem::Update`](../Ash2/src/System/HitstopSystem.hpp) | フェーズ内（PlayerTestPhase、MotionSystem の前） | `Hitstop` を持つエンティティの残り時間を減算し、0 以下になったら除去する（暫定実装） |
| [`MotionSystem::Update`](../Ash2/src/System/MotionSystem.hpp) | フェーズ内（PlayerTestPhase、HitstopSystem の後） | `Hitstop` を持たない `Motion`（Neutral/Melee1/Melee2/Melee3/Ranged/Dash）ごとの `Tick()` を呼び、移動・ジャンプ・向き・クリップ決定・状態遷移（攻撃判定/弾エンティティ生成、タイマー満了、無敵の付与/除去）を行う。各状態の `Tick()` 実体は [`PlayerMotionSystem.cpp`](../Ash2/src/System/PlayerMotionSystem.cpp) にある |
| [`MovementSystem::Update`](../Ash2/src/System/MovementSystem.hpp) | フェーズ内（PlayerTestPhase、MotionSystem の後） | `Hitstop` を持たない `WorldPos`+`Velocity` エンティティ（Player・弾）の位置を `vel * dt` で更新 |
| [`GravitySystem::Update`](../Ash2/src/System/GravitySystem.hpp) | フェーズ内（PlayerTestPhase、MovementSystem の後） | `Hitstop` を持たない `WorldPos`+`Velocity`+`Gravity` エンティティに重力加速（次フレーム用）と地面クランプ（今フレームの `pos.h` を 0 にする）を適用 |
| [`ProjectileSystem::Update`](../Ash2/src/System/ProjectileSystem.hpp) | フェーズ内（弾が存在する間、毎フレーム） | Projectile の着弾（hitTargets 非空）/ 画面外での破棄 |
| [`StaggerSystem::Update`](../Ash2/src/System/StaggerSystem.hpp) | フェーズ内（PlayerTestPhase、HitSystem の後） | `Stagger` を持つエンティティの残り時間を減算し `RectDrawable::size` を縮小、0 以下で `originalSize` に戻して除去する（暫定実装） |
| [`StaminaSystem::Update`](../Ash2/src/System/StaminaSystem.hpp) | フェーズ内（PlayerTestPhase、MotionSystem の後） | `Player + Stamina + Motion` を持つエンティティのスタミナを回復する。Neutral 状態のみ `recoveryDelay` 秒の待機後に `(max - current) / 2 * dt` で回復し、端数は `accum` に積み立てて誤差を防ぐ |
| [`HudSystem::Draw`](../Ash2/src/System/HudSystem.hpp) | 毎フレーム（DrawSystem の後） | Player の Hp / Stamina を画面左上にゲージ描画 |

---

## アセット管理（[Asset.hpp](../Ash2/src/Asset.hpp)）

- デバッグ: `assets/asset_list` をファイルから読む
- リリース: `assets/asset_list` を埋め込みリソースから読む
- `.png` → `TextureAsset`、`.mp3` → `AudioAsset` としてキー（相対パス）で登録
- アニメーション設定: `assets/config/animation/*.toml`（起動時に全ファイルをスキャン）

---

## 部品追加時の注意

- `Hierarchy` のメンバは必ず static メンバ関数（Attach/Detach/DestroyWithChildren）経由で操作する（不整合防止）。
- `Drawable` の型変更は `std::visit` を使い、DrawSystem と AnimationSystem の両方への影響を確認する。
- 新クラス追加時は `Ash2.vcxproj` と `Ash2.vcxproj.filters` にも追加が必要。
- `NameLookup` への挿入・削除は `NameLookupSystem::Connect` で自動化されている（`Name` コンポーネントの追加・削除に連動）。手動での `NameLookup[key] = entity` 登録は不要。
