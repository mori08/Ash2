# .vcxproj 操作メモ

## ファイルの構造

`Ash2.vcxproj` は400行超の XML。ファイル全体を読まず、Grep で対象箇所だけ探すこと。

## ヘッダーファイル（.hpp）を追加するとき

2つのファイルを編集する必要がある。

### Ash2.vcxproj

`ClInclude` で検索して追記する。

```xml
<ItemGroup>
  <ClInclude Include="stdafx.h" />
  <ClInclude Include="NewFile.hpp" />  <!-- ここに追加 -->
</ItemGroup>
```

### Ash2.vcxproj.filters

`ClInclude` で検索して追記する。

```xml
<ClInclude Include="NewFile.hpp">
  <Filter>Header Files</Filter>
</ClInclude>
```

## ソースファイル（.cpp）を追加するとき

`ClCompile` で検索して同様に追記する。

## コンパイラフラグを変更するとき

`AdditionalOptions` で検索する。Debug / Release の2箇所にある。
