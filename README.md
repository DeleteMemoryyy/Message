# Message

Implementation to a simple message system by `DeleteMemory`.

This project is a part of Introduction to  Computer Network lesson, Peking University.

### 编译和安装

**Linux**

- Terminal进入到Message目录下输入 “make” 进行编译
- 编译过程中会自动检测并安装绘制Client UI需要的glfw3库

**Windows**

- 需要预安装mingw（最好是mingw-w64）环境，并正确配置环境变量中的环境变量

  > 环境变量配置
  >
  > - Path：{mingw-w64-path}\bin
  >
  > - C_INCLUDE_PATH：{mingw-w64-path}\include
  >
  > - CPLUS_INCLUDE_PATH：
  >
  >   {mingw-w64-path}\lib\gcc\x86_64-w64-mingw32\7.3.0\include\c++;{mingw-w64-path}\lib\gcc\x86_64-w64-mingw32\7.3.0\include\c++\backward
  >
  > - LIBRARY_PATH：{mingw-w64-path}lib

  如果需下载，可自官网下载选择安装包

  http://mingw-w64.org/doku.php/download/mingw-builds

  或下载我编译好的文件

  https://pan.baidu.com/s/1sXyMC8ellJp6B0FDo_3RfQ

- 命令行或PowerShell进入到Message目录下输入“mingw32-make”

- bin文件夹中是我在本地静态编译好的Server端和Client端exe文件，但由于即使是使用加上-static参数的mingw编译，也还是会出现一些链接问题。我在除开发机之外的Windows 10 Home系统中测试并没有成功，显示“此应用无法在这台电脑上运行”。如果有同学能指出我在编译和配置中哪些步骤不正确，我将不胜感激

### 使用

**Client**

- 配置host地址和使用的端口

  ![img1](http://otl4n2fe9.bkt.clouddn.com/Message/img1.png)

  进入程序后将出现配置页面，正确配置后点击Connect按钮即可连接Server

  需要在Server启动后再进行连接

  > 默认给出的host地址47.106.157.25是我的VPS开放的外网地址，目前上面一直运行着Linux端的Server程序，远程连接测试时可随时连接该服务器端

- 消息和聊天

  - 连接成功后打开消息页面。在底部输入框输入消息，按回车发送

    ![img1](http://otl4n2fe9.bkt.clouddn.com/Message/img2.png)

    服务器端回复的消息将以红字在[Server]字段后显示

  - 上部四个按钮`Clear` `Copy` `Scroll to bottom` `Disconnect`可以点击使用

    断开连接后将回到配置界面，等待下一次连接的创建

  - 在Filter框中输入字符串后，只会显示包含该字符串的信息

    ![img3](http://otl4n2fe9.bkt.clouddn.com/Message/img3.png)

  - 可使用命令：

    - HELP——显示可用命令

    - HISTORY——逐条显示历史消息

      ![img4](http://otl4n2fe9.bkt.clouddn.com/Message/img4.png)

    - CLEAR——同`Clear`按钮，清屏（不会删除历史消息）

**Server**

- 直接运行Message_Server或Message_Server.exe，目前支持最多存在一个连接情况下的无限次连接

  需要在Client发送连接请求前启动