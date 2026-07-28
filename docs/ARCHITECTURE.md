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
├── System/              # ECS システム（ロジックのみ）
│   └── PlayerMotion/    # PlayerMotionSystem の状態別 Tick() 実装
└── Util/                # フレームワーク非依存の汎用ヘルパー
```

---

## 全体構造

```
Main.cpp ── registry を1つ作り、PhaseStack を回すだけ
  ├─ GameSetup ── registry.ctx() に設定を積む（起動時／F5 リロード時）
  ├─ Input ─────→ InputState（デバイス非依存の入力スナップショット）
  ├─ Phase ────── 「今どの局面か」を決め、局面ごとに System を呼ぶ順序を持つ
  │                 └─ System ── Component を読み書きする関数群（状態を持たない）
  └─ 常駐 System ── AttachmentSystem → DrawSystem → HudSystem
```

依存の向きは上から下への一方向。System は Phase を知らず、Component は System を知らない。
Config は最下層で、System からは `registry.ctx()` 経由でのみ読まれる。

### ゲームループを Phase が持つ

`Main.cpp` のループは「入力を取る → PhaseStack を1回回す → 描画する」しかしない。
どのシステムをどの順で呼ぶかは各 Phase の `update` が決める。

局面（プレイ中・メニュー・ビューア）ごとに必要なシステムは違う。`Main.cpp` に全システムを
並べると、メニュー中に当たり判定が走るような「その局面では意味のない更新」を条件分岐で
抑える羽目になる。例外は座標伝播・描画・HUD の3つで、局面に依らず必要なため常駐させる。

Debug ビルドでは `Console.open()` で起動し、環境変数 `ASH2_RUN_TESTS` が設定されている場合は
ゲームループに入らず Catch2 のテストランナーを実行して終了する（`tools/run-tests.sh` 経由）。
未捕捉の `std::exception` は `crash.log` に追記してから再 throw する。

### 遷移は「値」として返す

`IPhase::update` は `PhaseCommand`（None / Pop / Push / Reset）を返し、スタックの実際の操作は
`PhaseStack` だけが行う。Phase が自分でスタックを触ると、`update` の途中で自分自身が破棄される
危険がある。戻り値で意図だけを伝え、操作は呼び出し元に任せることでこれを構造的に防ぐ。

同じ考えが Motion にも適用されている（後述）。

---

## ECS（EnTT）の使い方

**コンポーネントはデータのみ。** `Component/` の型はすべてデータ構造で、振る舞いを持たない。
例外は `Hierarchy` と `WorldPos` で、前者は不変条件（双方向連結リストの整合性）を守るため
更新を static メンバ関数に限定し、後者は座標変換という純粋関数だけを持つ。

**システムは状態を持たない静的関数。** 入力は `registry` と `dt`／`FrameData` のみ。
フレーム間で持ち越したい状態は必ずコンポーネントか `registry.ctx()` に置く。
呼び出し順以外の暗黙の前提を持たないため、単体テストしやすい。

**能力はコンポーネントの組み合わせで表現し、種別ごとの分岐を作らない。**

- `Attack` + `Collider` を持てば攻撃判定になる
- `Hp` + `Collider` を持てば被弾判定の対象になる
- `Invincible` を足すと被弾対象から外れる

`Player` / `Enemy` / `Projectile` は種別の識別、`Invincible` / `Hitstop` はシステムのビューから
除外するためのスイッチとして使う。「フラグを見て if する」のではなく「ビューに入らない」形に
することで、除外の意図がシステム側のクエリに現れる。

**派生データはシグナルで同期する。** `NameLookup`（名前 → エンティティ）は
`on_construct<Name>` / `on_destroy<Name>` で、`Hierarchy` の連結リスト整合は
`on_destroy<Hierarchy>` で自動更新される。呼び出し側に「登録を忘れない」責任を負わせない
ための選択であり、その代償として `Name::value` は `const`（構築後の変更を禁止）にしてある。

**`registry.ctx()` はグローバルな読み取り専用データの置き場。** シングルトンを作らず registry に
相乗りさせることで、システムの引数が `registry` 1つで済み、テスト時は registry を作り直すだけで
隔離できる。格納物は [REFERENCE.md](REFERENCE.md) の「`registry.ctx()` の内容一覧」を参照。

---

## Motion — variant による排他的な行動状態

プレイヤーと敵の行動状態は `Motion`（`std::variant`）1つのコンポーネントで表す。

状態は排他的（同時に2つは成立しない）で、種類は有限かつコンパイル時に確定している。
variant なら状態ごとに必要なデータ（`Melee::stage`、`Dash::lastDashDir` 等）をそのまま持てて、
動的確保も仮想関数も要らない。プレイヤーと敵で variant を共有しているため、ディスパッチを
`MotionSystem` 1つに集約できる。

**遷移は Tick の戻り値でのみ表現する。** 各状態型には ADL で解決される
`Tick(state, registry, entity, frameData)` があり、`Optional<Motion>` を返す。`none` なら継続、
値があれば遷移。`MotionSystem` だけが `registry.replace<Motion>` を呼ぶ。
`MotionState` コンセプトがこの契約をコンパイル時に強制するため、状態型を追加したら `Tick` を
書かないとビルドが通らない。Tick が自分で状態を書き換えないのは、処理中に自分自身
（`state` の参照先）が破壊されるのを避けるため。`PhaseCommand` と同じ理由・同じ形をとっている。

**例外：外部要因による強制遷移。** 被弾は「その状態が自分で決める遷移」ではないため、
`HitReactionSystem` が直接 `replace<Motion>` する唯一の例外になっている。このとき前の状態が
`Tick` の満了時に行うはずだった後始末（`Velocity` のクリア、縮んだ `RectDrawable::size` の復元）が
飛ばされるので、`HitReactionSystem` が上書き前に代わりに行う。この例外は増やさない。

---

## 座標系 — 疑似3D

`WorldPos { w, h, d }` の3軸を使う。`w` は横位置（右が正）、`h` は高さ（地面=0、上が正）、
`d` は奥行き（大きいほど奥 = 画面上方）。

- 画面座標は `toScreen()` で `Vec2{ w, -(d + h) }`。描画順は `d` の降順（奥から手前）
- 高さと奥行きは同じ画面軸に潰れるが、当たり判定は3軸を保ったカプセル（線分＋半径）どうしの
  最近接距離で行う。描画では潰れる `h` と `d` を、判定では区別する
- カメラは `Scene::Center()` の固定オフセットのみ（スクロールなし）
- 接地判定は `h <= 0`（`WorldPos::isOnGround()`）で、`GravitySystem` が沈み込みを 0 にクランプする

**基準点：** `WorldPos` は `Drawable`（`DrawAnchor`）と `Collider`（オフセット）の共通基準点だが、
「中心」か「接地点」かはエンティティごとに異なる。`DrawAnchor` のデフォルトは `Center`、
接地キャラクター（プレイヤー等）は生成時に `BottomCenter` を明示する。`Collider` の `Vec3` は
x=w、y=h、z=d に対応する。

**制約：** `WorldPos` は常に絶対座標。相対座標は `LocalOffset` に置き、`AttachmentSystem` が
毎フレーム親の絶対座標 + `LocalOffset` で子を上書きする。

---

## 1フレームのデータフロー

`PlayerTestPhase::update` が代表例。各システムの呼び出し順と処理内容は
[REFERENCE.md](REFERENCE.md) の「システム一覧」を参照。順序には以下の意味がある。

- `AttachmentSystem` は `MotionSystem`（珠の `LocalOffset` 更新）より後、`HitSystem` より前。
  子の絶対座標が確定していないと判定がずれる
- `ProjectileSystem` は `MovementSystem` と `HitSystem` の後。着弾判定を `Attack::hitTargets` の
  中身で行うため
- `GravitySystem` の「加速」と「地面クランプ」は時間軸が違う（次フレーム用／今フレーム確定）。
  分割すると跳ね方が変わるため1つの関数に留める

### ヒットストップの実現方法

`Hitstop` を持つエンティティは `MovementSystem` / `GravitySystem` / `AnimationSystem` のビューから
`entt::exclude` で除外される。一方 `MotionSystem` は除外せず、`dt = 0` の `FrameData` で `Tick` を
呼ぶ。除外すると停止中の入力が `Tick` に届かず、コンボの先行入力を取りこぼすため。
「時間だけを凍結し、入力の受付は続ける」という区別が必要だった。
時間依存のシステムを追加するときは、同様の対応が必要か検討すること。

---

## 設定の外部化

数値はコードに埋めず TOML に置き、`FromToml` で構造体に変換して `registry.ctx()` に載せる。
Debug ビルドでは F5（`InputState::reloadConfig`）で再読込でき、`PlayerTestPhase` は自分自身を
エンティティごと作り直して即座に反映する。

アセットは `Ash2/App/assets/asset_list` を単一の入り口として扱い、Debug はファイルから・Release は
埋め込みリソースから読む差異を `Asset.hpp` に閉じ込める。パス解決は必ず `AssetPath()` を通す。

---

## 設計上の原則

- **ビルドは `tools/build.sh`、実行は `tools/run.sh` で行う。** デバッガを使った調査は
  ユーザーが Visual Studio 2022 で行う
- **破棄は親から。** `Hierarchy` を持つエンティティは `Hierarchy::DestroyWithChildren` で破棄し、
  子（攻撃判定の珠）が孤児になるのを防ぐ。独立エンティティである弾はタグで検索して個別に破棄する
- **ビューの走査中に `destroy` しない。** 破棄対象は配列に集めてループの外でまとめて破棄する
  （`EnemySystem` / `ProjectileSystem`）
- **タイムラインは共通化する。** 攻撃・ダッシュ系はすべて `MotionTimeline`（構え／有効／
  後隙A＝キャンセル不可／後隙B＝キャンセル可）の4区間で表し、区間判定をアクションごとに書かない
- **入力はテストしやすい型に落とす。** `InputState` は `Key` や `TOMLValue` のような実行環境に
  依存する型を持ち込まない。デバイス差は `InputDeviceSelector` で吸収する
- **エラー処理は層で使い分ける。** ゲームループ内（`Tick` / System）では投げない。設定読み込みは
  `std::expected` で伝播させ、`GameSetup` が呼ぶ最上位境界でのみ致命として `throw` する
- **`Name` は構築後不変。** `NameLookup` が構築・破棄シグナルでのみ同期されるため
- **ファイル追加時は `Ash2.vcxproj` と `.filters` も更新する**
