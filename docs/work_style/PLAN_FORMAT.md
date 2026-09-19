# PLAN_FORMAT.md

plan ファイル（`tmp/plan-<番号>.md`）の書き方を定める。

## 対象

- 対象は resolve-issue フローが作成・追記する plan ファイルのみ
  - `tmp/plan-<番号>.md`
- 他の文書（会話の応答・issue / PR・コードコメント等）には適用しない

## 節で分ける

- `###` の見出しを使って、段落に該当する節を作る
- 見出しは短く端的に、節の内容を要約したものにする
- 節が巨大化しすぎないように気をつける
  - 推奨は4~10行
  - 20行超える場合は分割した方がよいが、節としてのまとまりと行の短さの方が優先

## 箇条書きで書く

- 節内は箇条書きで記載する
  - リストマーカーは `-` を使う
  - 手順を表現するときは、`1.` `2.` のリストマーカーを使う
  - 地の文やリード文は置かない（区切りを作りたい場合は `###` の見出しにする）
- 一行一文で書く
  - 一行に複数の文をいれない
  - 一文の途中で改行をいれない
- 箇条書き内では端的に書く
  - このファイルのような文体を使う
  - 敬語などは使わず、句点は必要ない

## 長文を書かない

- 一行の幅が全角50(半角100文字)を超える文章を長文とする
- 短く端的に書き、一文で全てを表現しない
  - 補足や条件、理由は文に混ぜず、インデントを上げ次行へ逃がす
- 括弧内に文を書かない
  - OK）`reactionLevel == 1` (怯み) かを確認する
  - NG）`reactionLevel == 1` (このとき敵Enttiyが怯んでいる)かを確認する
    - このような補足は括弧内には書かず、次行へ逃がす

## インラインコードで長文にさせない

- 長文になる場合は次行に逃がす
- 悪例：`AbcdComponent.attack` (0のときは処理をスキップ) / `XyzComponent.hitpoint`  を取得して比較する
- 良例：以下を取得して比較する
  - `AbcdComponent.attack`
    - 0のときは比較をスキップ
  - `XyzComponent.hitpoint`

## ファイルパスはリンクにする

- ファイルパスは次行に逃がして、節の末尾にまとめる
- パスはバッククォートで囲む
  - 行番号はオプション（不要 / `:行番号` / `:開始行-終了行`）
    - 例） `Ash2/src/System/GravitySystem.cpp:8-21`
  - `file:1,5,10` のように複数の行を指定しない
- パスの行は行幅の制限に含めない

## コードブロックを使う

- 文章で説明するより、実例を示した方が分かりやすい場合はコードブロックを使う
- コードブロックの行数、文字数は他の制限を無視して良い
  - 行の幅/行数の制限にコードブロックを含めない
- 一つのコードブロックが巨大化しすぎないように注意
  - 巨大な関数を全て抜き出すことはせず、該当箇所を抽出して記載する
  - 論点内で重要ではない箇所は省略してよい
    - `// 略（要約）` のように省略したと分かるようにはする
- 参照元があるときは、コードブロック直前の行に `ファイルパス:行番号` を書く

`Ash2/src/System/GravitySystem.cpp:8-21`

```cpp
void GravitySystem::Update(entt::registry& registry, double dt) {
  auto view =
      registry.view<WorldPos, Velocity, Gravity>(entt::exclude<Hitstop>);
  for (auto&& [entity, pos, vel, gravity] : view.each()) {
    // 次フレーム用の重力加速
    vel.h -= gravity.accel * dt;
    // 略（地面クランプ）
  }
}
```

- 変更を強調したいときはdiffを使う

```diff
 void GravitySystem::Update(entt::registry& registry, double dt) {
-  auto view = registry.view<WorldPos, Velocity, Gravity>();
+  auto view =
+      registry.view<WorldPos, Velocity, Gravity>(entt::exclude<Hitstop>);
```
