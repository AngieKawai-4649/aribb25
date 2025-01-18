ツクミジマ氏が配布しているaribb25に以下の修正を実施

１．EMG機能追加

	EMMをカードに投げる設定になっている時にEMGもカードに投げるようにした
 
２．カードリーダーを指定する機能

	複数のカードリーダーを接続している環境でカードリーダーを指定する機能が追加されているが
 
	プログラムがイマイチなので修正
 
３．カードリーダー名称の取得

	複数のカードリーダーを接続している環境でカードリーダー名称を取得するプログラムを追加
 
４．windowsに特化

	linux用には別のリポジトリで公開することにした
 
５．VS2022でビルド、バグ・ワーニング除去

[output]

B1.exe         B1デコードを行うロードモジュール

B25.exe        B25デコードを行うロードモジュール

getcrn.exe     カードリーダー名称を取得するロードモジュール

上記3つはDOSコマンドプロンプトで実行する

libaribb1.dll  B1デコードを行うdll

libaribb25.dll B25デコードを行うdll


windows版とlinux版の違い

オリジナル版はマルモ氏作成のもの

B25解除する本体はC言語で作成されている

windows用デコーダーdllはwindowsアプリ(TVTest EDCB等)から動的リンク、コールする為に

C++でオブジェクトを作成し、それをエクスポートし、そこからC言語本体の関数を呼び出している

各アプリはC言語本体の関数を直接呼び出していない

よってC++オブジェクトで被せてあるdllでないと動作しない

Linux用はC言語本体のみで共有ライブラリを作成して、各チューナーアプリはC言語本体の関数を直接呼び出している

詳細はMulti2Dec/B25Decoder/Readme.txt(マルモ氏作成)を参照
