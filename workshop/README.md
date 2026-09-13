# 工房

作者が手で作成した一次資料の置き場です。ビルドにも配布物にも含まれません。

| ディレクトリ | 内容 | 現行の実装との関係 |
|---|---|---|
| `images/` | EDGE の作業ファイル（`.edg`） | [`Ash2/App/assets/images/`](../Ash2/App/assets/images/) へ書き出した PNG と一致する |
| `sketches/` | 手書きの初期案 | **一致しているとは限らない。** アイデア出しと初期設計の記録 |

現行の仕様は実装および [docs/](../docs) を参照してください。

## 画像を更新したら

`images/` の作業ファイルから `Ash2/App/assets/images/` へ PNG を書き出したあと、
[`tools/sync-assets.sh`](../tools/sync-assets.sh) を実行して `asset_list` と
`Resource.rc` を再生成します。

## 置かないもの

作業中の一時ファイルや使い捨てのファイル。ここに入るのは
[README.md](../README.md) で著作権を留保している原本と原案だけです。
一時ファイルは git 管理外の `tmp/` を使ってください。
