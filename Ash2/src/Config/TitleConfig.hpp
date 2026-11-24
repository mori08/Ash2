#pragma once
#include <Siv3D.hpp>

/// @brief 植物の棒1本分の形（全株で共通）
struct PlantStemConfig {
  /// 株の中心からの基準の横間隔（px。負が左）
  double dx;
  /// 棒の高さ（px）
  double height;
};

/// @brief 植物1株分の配置と揺れ方
struct PlantConfig {
  /// 根元の x 座標（論理座標、px）
  double x;
  /// 1往復の秒数（0 より大きい）
  double period;
  /// 出だしのずれ。1周期を 1.0 とした割合
  double offset;
};

/// @brief タイトル画面の配置・色の設定値
struct TitleConfig {
  /// ロゴの中心の y 座標（論理座標、px）
  double logoY;
  /// 1つ目のボタンの中心の y 座標（論理座標、px）
  double menuY;
  /// ボタンの中心同士の縦の間隔（px）
  double menuSpacing;
  /// ボタンの当たり範囲の幅（px）
  double buttonWidth;
  /// ボタンの当たり範囲の高さ（px）
  double buttonHeight;
  /// 背景（画面全体）の色
  ColorF backgroundColor;
  /// 水面線の色
  ColorF surfaceLineColor;
  /// ロゴに乗算する色
  ColorF logoColor;
  /// 選択されていないボタンの文字色
  ColorF buttonNormalColor;
  /// 選択中のボタンの文字色
  ColorF buttonFocusedColor;
  /// 水面線の中心の y 座標（論理座標、px）
  double surfaceY;
  /// 水面線の太さ（px）
  double surfaceLineWidth;
  /// 植物の色
  ColorF plantColor;
  /// 植物の棒の幅（px）
  double plantStemWidth;
  /// 棒の間隔の振れ幅。基準の間隔に対する割合（0.45 なら 0.55〜1.45 倍）
  double plantSpreadAmplitude;
  /// 株の形。棒ごとの基準の間隔と高さ。空なら植物は出ない
  Array<PlantStemConfig> plantStems;
  /// 株の配置と揺れ方。空なら植物は出ない
  Array<PlantConfig> plants;

  /// @brief TOML からタイトル画面の設定を生成する
  /// @throw Error 必須キーの欠落・型違い、`period` が 0 以下のとき
  [[nodiscard]] static TitleConfig FromToml(const TOMLValue& toml);
};
