1. 整体介绍
该文件夹包含两个基本文件
update_fta30：文件夹为镜像文件，将其烧录到板子里
flashavl.vbs：此文件为脚本文件，修改此文件达到测试目的

2. 烧录
将update_fta30中的文件全部复制到U盘根目录
打开SCom软件，连接对应串口
先将板子置于power off状态，连接到SCom软件后长按‘m’键，然后将板子置于power on
如果出现‘R’则成功，否则重复以上操作
在SCom软件中点击File...打开update_fta30目录下的boot.img文件，然后点击Send File
带发送完成后如果出现“Please send the dte-boot.”则成功，否则重复以上操作
然后在SCom软件中点击File...打开update_fta30目录下的dte_boot.img文件，然后点击Send File
待文件将要传送完成时一直点回车，进入boot模式
关闭SCom软件，打开SecureCRT软件，连接串口，点击软件上方菜单栏Script，选择Run
选择update_fta30目录下的update_spinand_encrypt.vbs脚本文件，再点击Run，开始烧写
等待出现“snf burn successfully!”则烧写成功，否则重复以上操作
重启板子，成功运行mt_sample，烧录成功

3. 使用
打开SecureCRT软件，连接串口
运行程序mt_sample
点击软件上方菜单栏Script，选择Run
选择flashavl目录下的flashavl.vbs脚本文件，再点击Run
flashavl测试开始
等待弹出方框，测试结果在方框中
测试结束

4.脚本文件修改
Function RunImg(a, b)函数中找到“flashavl -c /dev/mtd7 /media/casetest/usrfs.img ”，以下为参数解释
/dev/mtd7：                指定的block区域
/media/casetest/usrfs.img：要写进去的镜像文件（镜像文件得对应block区域，否则程序无法运行）
函数传参a：                循环次数，填 0 表示跳过执行
函数传参b：                程序内部执行次数

Function RunData(a, b)函数中找到“flashavl -n /dev/mtd8 ”，以下为参数解释
/dev/mtd8：                指定的block区域
函数传参a：                循环次数，填 0 表示跳过执行
函数传参b：                程序内部执行次数

Function RunBlock(a, b)函数中找到“flashavl -f /dev/mtd8 ”，以下为参数解释
/dev/mtd8：                指定的block区域
函数传参a：                循环次数，填 0 表示跳过执行
函数传参b：                程序内部执行次数

Function RunInterrupt(a)函数中找到“flashavl -i /dev/mtd8 ”，以下为参数解释
/dev/mtd8：                指定的block区域
函数传参a：                循环次数，填 0 表示跳过执行

如果会出现板子有时候识别不到U盘路径的情况找到脚本46行，根据提示修改脚本

Sub Main函数中：
RunStatu = RunImg(1)        param: 传入的参数‘1’为case循环次数
如果有不想测试case则把该case对应的函数和判断屏蔽掉
如果不想因为某条case测试失败而终止整个测试则把对应case的判断屏蔽掉
判断语句范围为If...Then到End If