# 开发日志

## 测试方法

* 提前设定好环境变量`QUERY_STRING`方便读取表单参数
* 执行`./hw3.cgi > log.html` 用浏览器打开查看输出结果

环境变量设置方法：

```
source setenv.sh
``````

## 架构

### 文件

* cig.cpp 主程序
* client.cpp client.h 客户端类
* setenv.sh 设置环境变量进行调试

