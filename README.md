
# PE_COFF

最小限の UEFI アプリの COFF オブジェクトファイルの  
不要なセクションを無効化し、MS 製 Linker に入力することで、    
MS 製 Linker の要求する最小限の COFF オブジェクトファイルを  
確認するためのツールです。使用した MS 製 Linker は以下の通りです。

```
Microsoft (R) Incremental Linker Version 14.23.28105.4
```

C 言語で書かれていて、Visual Studio の標準 C ライブラリと Windows API に依存しています。

## ファイルのダウンロード時の注意事項

コマンドプロンプトで以下のコマンドを投入し、ゾーン情報を削除します。

```
C:\>echo.>PE_COFF.zip:Zone.Identifier
```

## 開発環境

* Visual Studio Community 2019
https://docs.microsoft.com/en-us/windows-hardware/drivers/download-the-wdk
* 64 ビット版 Windows 10

## bin フォルダのコマンドの実行方法

Visual Studio 2019 Developer Command Prompt で作業します。  
上述したように、ゾーン情報を削除しておくことが前提条件です。

* PE_COFF.exe を実行すると、以下のファイルが出力されます。
```
PE_COFF.obj : UEFI アプリの efi_main.obj の不要なセクションを無効化したものです。
PE_COFF.txt : UEFI アプリの efi_main.obj の不要なセクション以外をテキスト形式で出力したものです。
PE_COFF.bin : UEFI アプリの efi_main.obj の .text セクションの Raw Data です。
```

* PE_COFF_obj_dump.cmd を実行すると、以下のファイルが出力されます。
```
PE_COFF_obj.txt : (PE_COFF.obj を dumpbin した結果)
```

* PE_COFF_link.cmd を実行すると、以下のファイルが出力されます。
```
bin\efi\boot\bootx64.efi : UEFI アプリ(PE_COFF.obj を link した結果)
```

## ビルド方法

* PE_COFF.sln ファイルをダブルクリックします。
* ゾーン情報を削除していない場合は、Visual Studio のセキュリティ警告を回避してプロジェクトを開きます。  
![warning](https://raw.githubusercontent.com/tenpoku1000/PE_COFF/master/images/MSVC.PNG)
* F7 キーを押下します。

ビルド後に tools フォルダに存在するファイルが、bin フォルダにコピーされます。

## ライセンス

[MIT license](https://raw.githubusercontent.com/tenpoku1000/PE_COFF/master/LICENSE)

## 作者

市川 真一 <tenpoku1000@outlook.com>

