# hackfonForM5Stack

「hackfon」とは「アナログ電話をスマートリモコンに」というコンセプトで「FutuRocket株式会社」 (https://www.futurocket.co/) が現在製品化に向けて進めているIoTプロダクトです。

![hackfon](/techbookfesta8/img/01_hackfon.jpg)

以下に簡単な製品説明があります。<br>
(技術書典#8の"Madamada M5Stack Moku Moku Mook β" ( https://booth.pm/ja/items/1887481 )からの抜粋版です)<br>

/techbookfesta8/tomorrow56_tech8_03.md

### 2020年4月13日更新

"exsample"にIFTTT対応版を追加しました。<br>

#### /example/M5_hackfon_IFTTT

同梱の "hackfon.cfg" を編集して "makerKey"と"Event trigger"を設定に応じて書き換えてSDカードのルートに置けば設定をこちらから読みますので、ソースを修正する必要はありません。今回のバージョンでは各キーに１つのIFTTTのイベントキーが割り当てることができます。<br><br>

また、コンパイル済のバイナリー(M5-SD-Updater対応版, M5_hackfon_IFTTT.bin) も準備しました。<br>
こちらをSDカードのルートに置いて、以下のM5Burnerをダウンロードして、LovyanLauncher-v0.2.3 を本体に書き込めばSDカードに保存したバイナリーを実行することができます。<br>
https://m5stack.oss-cn-shenzhen.aliyuncs.com/resource/software/M5Burner.zip
