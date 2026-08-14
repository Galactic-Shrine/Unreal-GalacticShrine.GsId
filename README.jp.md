# Unreal Engine 向け GalacticShrine GsId

**言語:** [Français](./README.md) · [English](./README.en.md) · [Español](./README.es.md) · [Italiano](./README.it.md) · **日本語**

[![バージョン](https://img.shields.io/badge/version-1.0.1-4c9aff)](https://github.com/Galactic-Shrine/Unreal-GalacticShrine.GsId/releases)
[![Unreal Engine](https://img.shields.io/badge/Unreal%20Engine-5.8-0e1128?logo=unrealengine)](https://www.unrealengine.com/)
[![ライセンス](https://img.shields.io/badge/license-MIT-blue)](./LICENSE)
[![対応環境](https://img.shields.io/badge/platforms-Windows%20%7C%20Linux%20%7C%20macOS-6c757d)](#対応環境)

`GsId` は、**256 ビット**の Galactic-Shrine 識別子を C++、Blueprint、シリアライズ、ネットワークレプリケーションで利用できる Unreal Engine 用の独立したプラグインです。

## 概要

| 項目 | 内容 |
| --- | --- |
| プラグイン | `GsId` |
| バージョン | `1.0.1` |
| 対象エンジン | Unreal Engine `5.8` |
| Runtime モジュール | `GsId` |
| Editor モジュール | `GsIdEditor` |
| 識別子サイズ | 256 ビット / 32 バイト |
| テキスト形式 | `N`（64 文字）および `D`（69 文字） |
| GitHub ライセンス | `MIT` |
| Fab ライセンス | `Fab Standard License` |

## 主な機能

- C++ と Blueprint で利用できる Unreal 型 `FGsId`；
- 組み込みのネイティブ C++20 API `GalacticShrine::GsId`；
- 暗号学的に安全なランダム生成と SHA-256 による決定論的生成；
- 厳密な解析、検証、大文字・小文字のフォーマット；
- 32 バイトの生データとの相互変換；
- `FArchive` バイナリ、テキスト、`NetSerialize` の各シリアライズ；
- レプリケーション、比較、並び替え、`GetTypeHash` 対応；
- 13 言語にローカライズされた Blueprint メタデータ；
- コア機能と Unreal 統合を検証する 14 件の自動テスト。

## インストール

`GsId` ディレクトリをプロジェクトのプラグインフォルダーへコピーします：

```text
<Project>/Plugins/GsId/
```

エンジン単位でインストールすることもできます：

```text
<UnrealEngine>/Engine/Plugins/Marketplace/GsId/
```

Plugin Browser で **GsId** を有効にし、プロジェクトを再ビルドしてください。

## C++ 統合

`.Build.cs` ファイルにモジュールを追加します：

```csharp
PublicDependencyModuleNames.Add("GsId");
```

```cpp
#include "GsId.h"

const FGsId Id = FGsId::NewGsId();
const FString Text = Id.ToString(EGsIdFormat::D);

FGsId Parsed;
if (FGsId::Parse(Text, Parsed))
{
    check(Id == Parsed);
}
```

ネイティブ C++20 API も利用できます：

```cpp
#include <galactic_shrine/gsid/gsid.hpp>

const GalacticShrine::GsId Id = GalacticShrine::GsId::NewGsId();
```

## Blueprint 統合

Blueprint ライブラリには `New GsId`、`Try New GsId`、`Parse`、`Parse Exact`、`To String`、`To Bytes`、`From Bytes`、`Create Deterministic GsId`、`Get Stable Hash` が含まれます。

安全な乱数生成器の失敗をアプリケーション側で明示的に処理する場合は、`Try New GsId` を使用してください。

## 対応フォーマット

### `N` 形式

64 文字の 16 進数によるコンパクト形式：

```text
4ee77ca611ab7ebc9fabdc58c9689676f0763af9eff3c0df575f94ab230c0c66
```

### `D` 形式

`16-8-8-8-8-16` で区切られた 69 文字の可読形式：

```text
4EE77CA611AB7EBC-9FABDC58-C9689676-F0763AF9-EFF3C0DF-575F94AB230C0C66
```

## 対応環境

| プラットフォーム | 安全な乱数ソース |
| --- | --- |
| Windows 64 ビット | `BCryptGenRandom` |
| Linux | `getrandom` |
| macOS | `SecRandomCopyBytes` |

未対応のプラットフォーム、または組み込み C++ ソースが存在しない場合は、明確なビルドエラーが発生します。

## ドキュメント

完全なドキュメント、API リファレンス、統合ガイドは [docs.galactic-shrine.com](https://docs.galactic-shrine.com/) で公開しています。

## 分散ビルド

各コントリビューターは、自分の fork と Unreal runner でプラグインをビルドできます。[個人 runner ガイド](./.github/SELF_HOSTED_RUNNERS.md)を参照してください。不足しているプラットフォームは、既存のタグを移動せずに [pull request で提案](./PlatformBuilds/README.md)できます。

## テスト

Session Frontend または AutomationTool から、次のフィルターでテストを実行します：

```text
GsId.*
```

## ライセンス

GitHub で公開されるソースコードとリリース（組み込みの `GalacticShrine.GsId.Cpp` を含む）は、[MIT License](./LICENSE) の下で配布されます。

Fab から無料で取得するパッケージは、[Fab Standard License](https://www.fab.com/eula) に基づく別の配布物です。各パッケージに含まれるライセンスファイルが、そのコピーに適用される条件を示します。[サードパーティーに関する通知](./THIRD_PARTY_NOTICES.jp.md) も参照してください。
