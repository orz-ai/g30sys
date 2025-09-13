# 更新的文件
1. make.bat
2. Makefile

**注意**： Makefile 每行命令的开头要使用了 tab 键，而不是空格

# 启动
## 第一阶段
1. 先执行`make -r ipl.bin`，会编译出一个ipl.bin和ipl.lst
2. 执行`make -r helloos.img`，生成helloos.img
3. 在执行`run.bat`

## 第二阶段
1. 直接执行 `make run`, 会编译出一个ipl.bin和ipl.lst, 并生成helloos.img