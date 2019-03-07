lua的实时gc补丁
----
## 修改要点
为将lua用于实时系统中，修改了lua的gc算法，主要修改内容：
- lua增量gc的参数进行了调整，使gc动作更加频繁
- lua增量gc的单个步骤中，增加了计时，如果未能在限定时间内完成，则自动让出控制权
## 使用方法
在lua脚本中，使用如下函数启用实时gc功能
```
> set_rtgc(0.7, 0)
> seg_rtgc(0, 0)
```
- 参数0.7表示将每步增量gc的时间限制在0.7ms内
- 参数0表示不记录gc调试信息，改为1,则运行结果会记录在rtlog.txt文件中
- 两个参数均设为0, 则关闭实时gc 
