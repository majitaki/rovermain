## クロスコンパイラ環境の構築方法 ##
1. Ubuntu環境を整える．
2. ターミナルで  
sudo apt-get update  
sudo apt-get upgrade  
sudo apt-get install ia32-libs  
sudo apt-get install libc6-i386 lib32stdc++6 zlib1g:i386  
3. https://bitbucket.org/hogefugabar/minerva/downloads/compiler.tar.gz  
を落としてきて，解凍し，ソースコードのあるディレクトリに放り込む．
4. ソースコードのあるディレクトリでmakeコマンド実行．