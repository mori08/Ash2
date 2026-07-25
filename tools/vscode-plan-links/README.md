# Plan Links

`tmp/plan-*.md`（resolve-issue フローが作る plan ファイル）の中で、
バッククォートで囲んだリポジトリルート相対パスを Ctrl+Click で開けるようにする
VS Code 拡張機能。

## 記法

バッククォートで囲んだ内容が「ルート相対パス（+ 任意の行指定）」のとき、
そのファイルが実在すればリンクになる。

```
`src/System/PlayerMotion/Helper.cpp`        ← ファイルを開く
`src/System/PlayerMotion/Helper.cpp:18`     ← 18 行目へジャンプ
`src/System/PlayerMotion/Helper.cpp:18-19`  ← 18 行目へジャンプ（範囲は開始行を使う）
```

- ラベルは不要。パスだけを書く
- `../` は不要。ワークスペースのルート基準で解決する
- 実在しないパスや、パス形式でないインラインコードはリンクにならない

## 動作の仕組み

`DocumentLinkProvider` を `**/tmp/plan-*.md` にだけ登録し、
インラインコード span を走査してルート相対で存在確認する。
plan ファイル以外の Markdown には影響しない。

## インストール

ビルド不要（素の JavaScript）。このフォルダを VS Code の拡張ディレクトリへ置く。

Windows:

```sh
# シンボリックリンク（管理者権限が必要な場合あり）
cmd //c mklink //D "%USERPROFILE%\.vscode\extensions\plan-links" "%CD%\tools\vscode-plan-links"
# もしくはフォルダごとコピー
cp -r tools/vscode-plan-links "$USERPROFILE/.vscode/extensions/plan-links"
```

置いたあと VS Code を再読み込みする（コマンドパレット → Developer: Reload Window）。

## 開発・動作確認

このフォルダを VS Code で開いて F5 を押すと、拡張機能を読み込んだ
Extension Development Host が起動する。そこで `tmp/plan-*.md` を開いて挙動を確認する。
