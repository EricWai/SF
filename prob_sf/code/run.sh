#sleep 3
#python3 test.py

#在java目录下运行mvn clean package -f pom.xml即可生成jar包
#java -jar ./java/target/player-1.0-SNAPSHOT.jar

# 以下是针对 mac-m1 的处理
## 如果是 c++, 需要重新编译
#```sh
#    # run.sh 参考 run-template-arm64-cpp.sh
#	g++ test.cpp `pkg-config --libs --cflags libcurl` -o test
#	./test

	 g++ final.cpp `pkg-config --libs --cflags libcurl` -o final
	 ./final
	#  g++ final_ac8.cpp `pkg-config --libs --cflags libcurl` -o final_ac8
	#  ./final_ac8
#```
#
### 如果是 golang，可以选择重新编译，提交前改成编译成 x86架构
#	* 提交前先本地编译成 amd64版本， `GOOS=linux GOARCH=amd64 go build`
#	* run.sh只填写可运行程序
#```
#./code
#```