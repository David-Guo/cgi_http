#开发日志

## 测试

1. 执行 `./http [port]` 绑定端口
2. 浏览器打开 `127.0.0.1:port` 

* 如果绑定端口为80则浏览器中无需输入port *

学习点：使用`strerror(error)` 打印错误信息。

使用如下函数封装错误输出函数：

```c
error(const char *eroMsg) {
    cerr << eroMsg << ":" << strerror(errno) << endl;
    exit(1);
}
``````

那么在调用`execvp(requestDoc.c_str())` 时，如果返回 fail ，就可以从 `error("execvp cgi failed");` 中得到错误消息，然后进一步处理问题。
