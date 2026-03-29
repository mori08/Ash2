# テスト

## 方針

OpenSiv3D に同梱されている **Catch2** を使用する。
テストは Siv3D 環境内で動作するため、すべての Siv3D 型をそのままテスト可能。

## ファイル構成

```
Ash2/
├── src/        ← ゲームのソースコード
└── tests/      ← テストコード（*Test*.cpp）
```

## テストの実行

Debug ビルドで実行すると自動的にテストが走る。

- **成功時**: コンソールがすぐ閉じ、そのままゲームが起動する
- **失敗時**: コンソールが残り、失敗内容を確認できるまで待機する

## テストの書き方

`tests/` に `.cpp` ファイルを追加し、`USE_TEST` ガードで囲む。

テスト名は英語で書き、日本語はコメントで補足する（コンソール出力の文字化け対策）。

```cpp
# if USE_TEST
# include <ThirdParty/Catch2/catch.hpp>
# include "WorldPos.hpp"

TEST_CASE("WorldPos::ToScreen - far objects have smaller y")
{
    // 奥にあるものほどy座標が小さい
    REQUIRE(...);
}

# endif
```


## USE_TEST プリプロセッサ

Debug 構成のみに定義されている。Release ビルドにはテストコードが含まれない。

## ビジュアルテスト

描画・座標感覚の確認はシーン管理が整備されてから行う。
シーン管理の基盤ができたら、デバッグ用シーンを追加してビジュアルテストを組み込む。
