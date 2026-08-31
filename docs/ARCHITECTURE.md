# ARCHITECTURE.md

「沼に焚べ」の設計の全体像。何がどう組み合わさって動いているかを説明する。
コンポーネント・システム・フェーズ等の一覧は [REFERENCE.md](REFERENCE.md) を参照。

## 全体の構成

1. **座標系** — 疑似3D。位置を持つものすべての前提
2. **ECS** — エンティティとデータと処理の分け方
3. **フェーズ** — 起動と遷移
4. **Motion** — プレイヤーと敵の行動状態
5. **Config** — 数値と資産の外部化
6. **入力** — デバイス差の吸収
7. **描画** — ワールドを画面へ

コードの入口は `Main.cpp`。ゲームループの全体が1ファイルに収まっている。

---

## 前提

### ゲーム

疑似3D視点のベルトアクションゲーム。奥行きを持つ平面をプレイヤーが移動し、敵と戦う。
ゲームデザイン上の仕様は [docs/game_design/](game_design/) を参照。

### 技術構成

| 技術 | バージョン | 役割 |
|------|-----------|------|
| C++ | std:cpplatest | 実装言語 |
| Siv3D | v0.6.16 | ゲームフレームワーク（描画・入力・数学型） |
| EnTT | v3.16.0 | ECS（Entity Component System） |
| Catch2 | Siv3D 同梱版 | ユニットテスト |

### 用語

| 語 | 意味 |
|---|---|
| w / h / d | 座標の3軸。横／高さ／奥行き。`WorldPos` コンポーネントのメンバ名 |
| `FrameData` | そのフレームの更新データ（経過時間 `dt` と入力）。`Main` が組み立てる |
| フェーズ | 「今どの局面か」を表す単位（プレイ中・メニュー・ビューア等）。`IPhase` の実装 |
| シナリオ | フェーズの進行順を書いた TOML（`assets/config/scenario.toml`） |
| モーション | エンティティの排他的な行動状態（`PlayerMotion::Variant` / `EnemyMotion::Variant` コンポーネント） |

### ディレクトリ構成

```
Ash2/src/
├── Main.cpp              # エントリポイント・ゲームループ
├── GameSetup.hpp/.cpp    # registry 初期化・設定リロード
├── Asset.hpp             # アセット登録・パス解決ユーティリティ
├── FrameData.hpp         # フレームごとの更新データ（dt + InputState）
├── UiFonts.hpp           # UI 描画用フォント一式（registry.ctx() に格納）
├── Debug.hpp             # APP_LOG マクロ等のデバッグ用ユーティリティ
├── FatalError.hpp        # 致命エラーの型（分類と詳細）
├── CrashHandler.hpp/.cpp # 致命エラーの記録・表示・終了
├── Component/            # ECS コンポーネント（データのみ）
├── Config/               # TOML 設定データ（FromToml 付き構造体）
├── Input/                # 入力抽象化
├── Phase/                # フェーズ管理（ゲーム状態機械）
├── System/               # ECS システム（ロジックのみ）
│   └── PlayerMotion/     # PlayerMotionSystem の状態別 Tick() 実装
└── Util/                 # フレームワーク非依存の汎用ヘルパー
```

依存の向きは `Component` / `Config` → `System` → `Phase` の一方向。System は Phase を知らず、
Component は System を知らない。Config は最下層で、他のどのディレクトリも include しない。
System が Config を読むのは `registry.ctx()` 経由のみ。

---

## 1. 座標系 — 疑似3D

`WorldPos { w, h, d }` の3軸を使う。`w` は横位置（右が正）、`h` は高さ（地面=0、上が正）、
`d` は奥行き（大きいほど奥 = 画面上方）。

- 画面座標は `toScreen()` で `Vec2{ w, -(d + h) }`。描画順は `d` の降順（奥から手前）
- 高さと奥行きは同じ画面軸に潰れるが、当たり判定は3軸を保ったまま行う。描画では潰れる
  `h` と `d` を、判定では区別する

**基準点：** `WorldPos` は描画と当たり判定の共通基準点だが、「中心」か「接地点」かは
エンティティごとに異なる。`DrawAnchor` のデフォルトは `Center`、接地キャラクターは生成時に
`BottomCenter` を明示する。位置や範囲を `Vec3` で表すときは x=w、y=h、z=d に対応させる。

**位置とずれは別の型で表す。** 絶対座標は `WorldPos`、親からの相対位置は `LocalOffset` に置く。

---

## 2. ECS — 能力は組み合わせで表現する

EnTT を使用し、実体（Entity）とデータ（Component）、処理（System）を分けて管理する。

- **レジストリ**（`entt::registry`）
  - 1つだけ作成し、エンティティとコンポーネント、コンテキストを保持する
- **エンティティ**（`entt::entity`）
  - 純粋な ID。`registry.create()` で作成する
- **コンポーネント**（`Component/` に型を配置）
  - 振る舞いを持たない構造体
  - `registry.emplace<C>(entity)` で持たせ、その組み合わせでオブジェクトを表現する
  - 整合性を保つ必要がある場合のみ、`const` 化やメンバ関数への限定を行う
- **システム**（`System/` に関数を配置）
  - 状態を持たない静的関数
  - 引数は `registry` と `FrameData`
- **コンテキスト**（`registry.ctx()`）
  - グローバルな読み取り専用データの置き場
  - システムの引数が `registry` 1つで済み、テストは registry を作り直すだけで隔離できる

---

## 3. フェーズ — 起動と遷移

プレイ中・メニュー・ビューアといったフェーズを、スタック（`PhaseStack`）で持つ。フェーズを
積むと前のフェーズは下に残り、取り出すとそこへ戻る。会話が始まったら会話のフェーズを積み、
終われば元のフェーズが続きから動く、という入れ子がそのまま書ける。

### フェーズは自分の終わりだけを知る

先頭のフェーズ（`IPhase`）の `update` だけが毎フレーム呼ばれ、`PhaseCommand`（`None` / `Pop` /
`Push` / `Reset`）を返す。スタックを実際に操作するのは `PhaseStack` だけで、フェーズは意図を
返すに留まる。

この形のため、フェーズは**次に何が動くかを知らない**。自分が終わったら `Pop` を返すだけで、
次に動くのは下に残っていたフェーズになる。`WaitPhase` のような汎用のフェーズが、呼び出し元
ごとの遷移先を持たずに済む。

### ScenarioPhase だけが進行順を知る

画面の流れは C++ ではなく `assets/config/scenario.toml` にある。`Main.cpp` が最初に積むのは、
その `init` セクションを指す `ScenarioPhase` 1つだけ。

`ScenarioPhase` は毎フレーム1ステップ進み、`push` のステップなら次のフェーズを積む。
**自身はスタックの下に残る。** 積まれたフェーズが `Pop` すると `ScenarioPhase` が再び先頭に
なり、次のステップから再開する。進行順を知っているのは `ScenarioPhase` だけで、積まれた側は
知らない。

```
ScenarioPhase          ← 下に残り続ける
  └ push → TestMenuPhase   ← Pop すると ScenarioPhase の続きへ戻る
       └ push → PlayerTestPhase
```

TOML を読むのは Config 層で、具象フェーズを知らない。

- `ScenarioData`（`Config/ScenarioData.hpp`）— セクション名 → ステップ列（`ScenarioStep` は
  `StepPush` / `StepReset` の variant）
- `PhaseLoaderTable`（同上。実体は `Phase/PhaseLoaders.cpp` の `GetPhaseLoaders()`）—
  TOML のフェーズ名を生成子に変換する表。`ScenarioData::FromToml` が引数で受け取り、
  配線は `GameSetup` が行う

参照先セクションの実在は、全セクションを登録し終えた後にまとめて検証する。前方参照でき、
不在なら起動時に失敗するので、ゲームループ内で例外は飛ばない。

### ECS との連携

`PhaseStack` が持つのはフェーズの積み重ねだけで、ゲームの状態は持たない。エンティティと
コンポーネントはすべて registry の側にある。両者は、`Main.cpp` が作った registry を毎フレーム
渡すことで噛み合う。

```
Main.cpp ── registry を1つ作る
  └ PhaseStack::update(registry, frameData)
       └ IPhase::update(registry, frameData)    ← 先頭のフェーズだけ
            └ 各 System::Update(registry, ...)  ← そのフェーズに必要なものを順に
```

---

## 4. Motion — プレイヤーと敵の行動状態

プレイヤーと敵の行動を、排他的な状態として持つ。同時に2つは成立しない。種別ごとに状態は
`PlayerMotion::Variant` / `EnemyMotion::Variant`（いずれも `std::variant`）のコンポーネント
1つで表し、その状態でだけ必要なデータは状態の型が自分で持つ。種別をまたぐ variant は無く、
プレイヤーのエンティティに敵の状態を持たせることは型として成立しない。

### 状態は次を返すだけで、自分を書き換えない

各状態型には `Tick` があり、自身の variant 型 `M` に対する `Optional<M>` を返す。`none`
なら継続、値があればその状態へ遷移する。variant を書き換えるのは `MotionSystem` 1つだけ。

```
MotionSystem::Update(registry, frameData)
  └ PlayerMotion::Variant → EnemyMotion::Variant の順に
       └ std::visit ── 現在の状態型を選ぶ
            └ Tick(state, registry, entity, frameData) → Optional<M>
                 ├ none  ── 継続
                 └ 値あり ── MotionSystem が replace<M>
```

`Tick` は ADL で解決される自由関数で、`MotionState<S, M>` コンセプトが契約を強制する。状態型を
足して `Tick` を書かないとビルドが通らない。走査対象の variant を増やすときは
`MotionSystem.cpp` の `UpdateMotions` へ型を足すだけで済む。

### 例外：外部要因による強制遷移

被弾だけは `HitReactionSystem` が Motion の variant を直接 `replace` する。このとき前の状態が
`Tick` の満了時に行うはずだった後始末が飛ばされるので、上書きする側が代わりに行う。
この例外は増やさない。

---

## 5. Config — 数値と資産の外部化

数値はコードに埋めず TOML に置き、`FromToml` で構造体に変換して `registry.ctx()` に載せる。
`FromToml` は `std::expected` を返し、キー欠落や型不一致は起動時の失敗になる。

Debug ビルドでは F5（`InputState::reloadConfig`）で再読込できる。読み込みに失敗した場合は
旧データを維持するため、調整中に書き損じてもゲームは落ちない。

アセットは `Ash2/App/assets/asset_list` を単一の入り口として扱い、Debug はファイルから・Release は
埋め込みリソースから読む差異を `Asset.hpp` に閉じ込める。パス解決は必ず `AssetPath()` を通す。

---

## 6. 入力 — デバイス差の吸収

キーボードとゲームパッドの違いは `InputDeviceSelector` が吸収し、以降は `InputState`
（そのフレームの論理入力）だけを扱う。`Main.cpp` が毎フレーム作り、`FrameData` に載せて
フェーズとシステムへ渡す。

```
KeyboardInputAction ─┐
XInputAction ────────┴→ InputDeviceSelector ── InputState → FrameData
                        （最後に操作されたデバイスを選ぶ）
```

`InputState` は `Key` のような実行環境に依存する型を持たない。値を組み立てるだけで、
入力を伴うシステムのテストが書ける。

---

## 7. 描画 — ワールドを画面へ

画面を描くシステムは `DrawSystem` と `HudSystem` の2つだけ（Release ビルドの場合）。
見た目の話はここに閉じている。Debug ビルドはこれに加え `DebugDrawSystem` が
`Collider` の輪郭を重ねて描く。本番の描画パスへ `_DEBUG` 分岐を持ち込まないよう
別クラスに分け、呼び出し自体を `Main.cpp` 側の `#ifdef _DEBUG` で囲む。

`DrawSystem` は `WorldPos` と `Drawable` を持つエンティティをまとめて描く。種別ごとの描画
コードはない。`Drawable` は形状の variant（矩形・円・テクスチャ）で、色は `DrawColor` が別に
持つ。持たないエンティティは白・不透明として扱う。

`HudSystem` はワールドを通らず、画面座標へ直接描く。

アニメーションは `TextureDrawable` の中身の差し替えとして表す。`AnimationSystem` が
`SpriteAnimation` の経過時間を進め、切り出した `TextureRegion` を書き込む。`DrawSystem` は
それが動いているかどうかを知らない。
