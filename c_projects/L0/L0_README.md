# L0
## 调试环境准备
### 编译构建项目代码
编译前需要写一个Makefile，Makefile中需要包含
```Makefile
NAME := hello # 构建出的程序/镜像名称
SRCS := hello.c # 包含你所有的代码列表
#DEPS   := hello.h # 包含你的头文件，可选项
include $(AM_HOME)/Makefile # 需要定义上述变量后方可编译
$(shell xxd -i $(GRAPH_PATH) > graph_data.c)
```
编译过程可能失败，可以通过`make -nB`查看具体编译命令（insight：通过trace将未知变为已知）
在$(AM_HOME)/Makefile中设定了默认构建的目标
```Makefile
### Default to create a bare-metal kernel image
ifeq ($(MAKECMDGOALS),)
  MAKECMDGOALS  = image
  .DEFAULT_GOAL = image
endif
···
### Build order control
image: image-dep
archive: $(ARCHIVE)
image-dep: $(OBJS) am $(LIBS)
	@echo \# Creating image [$(ARCH)]
```
其中LIBS会编译实验框架中的am和klib的代码（对于.S文件用ASFLAGS控制编译选项）
正常情况下，make run最后会启动qemu
```bash
qemu-system-x86_64 -s -S -serial mon:stdio -machine accel=tcg -smp "1,cores=1,sockets=1" -drive format=raw,file=/home/yzh/NJU_os/c_projects/L0/build/hello-x86_64-qemu -vnc 0.0.0.0:12345
```
-serial mon:stdio: 表示将串口输出重定向。mon 表示将串口重定向到监控控制台
-machine accel=tcg: 指定使用的加速器。accel=tcg 表示使用 TCG（Tiny Code Generator）作为虚拟机的加速器。
-smp "1,cores=1,sockets=1": 设置处理器配置。
-drive format=raw,file=~/L0/build/hello-x86_64-qemu: 指定硬盘驱动器配置。
--nographic：如果不需要图形界面或在因为系统没有图形库而失败，可以加上
-vnc 0.0.0.0:12345：开启一个vnc服务来展示系统界面
-s -S：The -s option will make QEMU listen for an incoming connection from gdb on TCP port 1234, and -S will make QEMU not start the guest until you tell it to from gdb. (If you want to specify which TCP port to use or to use something other than TCP for the gdbstub connection, use the -gdb dev option instead of -s)
## 系统启动过程
以x86平台为例
1. 上电后80x86芯片进入16位实模式（可以通过CR0寄存器查看是否为实模式（CR0.PG和CR0.PE）），内存寻址方式为`base($cs << 4) + $ip`(可访问的空间仅有1M（16+4=20bit）)。cpu从内存0xFFFF0(qemu -s -S启动后通过gdb查看$pc=0xfff0,$cs=0xf000)开始执行指令，这个地址通常是ROM BIOS/UEFI程序的地址
2. BIOS会将位于启动设备的MBR（Master Boot Record，主引导扇区）的bootloader程序读入内存0x7c00处（在gdb中watch *0x7c00后可以观测到这一过程）。
3. 跳转（jmp）0x7c00处开始执行刚刚搬运的bootloader程序（GRUB就是其中一种），在本实验中的bootloader是abstract-machine/am/src/x86/qemu/boot/目录下的代码实现的（因为bootloader的大小限制，编译时没法带调试符号）由start.S开启保护模式，并进入main.c中加载磁盘中的elf程序。
```asm
  lgdt    gdtdesc
  movl    %cr0, %eax
  orl     $CR0_PE, %eax // 使能保护模式，从16位实模式切换到32位保护模式
  movl    %eax, %cr0
  ljmp    $GDT_ENTRY(1), $start32

.code32
start32:
  movw    $GDT_ENTRY(2), %ax
  movw    %ax, %ds
  movw    %ax, %es
  movw    %ax, %ss

  movl    $0xa000, %esp
  call    load_kernel # 跳转到main.c中加载内核
```
4. 跳转elf的Entry point执行。elf的Entry point，一般会在链接生成elf的时候默认将其设置为_start的地址（在实验框架中是由$(AM_HOME)/am/src/x86/qemu/start64.S）在_start中从保护模式进入了64位模式
```asm
.long_mode_init:
  movl  $PML4_ADDR,  %eax
  movl  %eax,        %cr3     // %cr3 = PML4 base
  movl  $CR4_PAE,    %eax
  movl  %eax,        %cr4     // %cr4.PAE = 1
  movl  $0xc0000080, %ecx
  rdmsr
  orl   $0x100,      %eax
  wrmsr                       // %EFER.LME = 1
  movl  %cr0,        %eax
  orl   $CR0_PG,     %eax
  movl  %eax,        %cr0     // %cr0.PG = 1 开启64位运行模式并开启内存分页机制
  lgdt  gdt_ptr               // bootstrap GDT
  ljmp  $8, $_start64         // should not return
```
## 调试
用python替换原有的gdb脚本
```python
# gdb包不是python中的包，无法在gdb外面使用，只有在gdb程序内部的python才能使用
import gdb

# Kill process (QEMU) on gdb exits
def quit_handler(event):
    print("Killing process (QEMU)")
    gdb.execute("kill")
    
gdb.events.exited.connect(quit_handler)

# Define hook-stop
def stop_handler(event):
    print("Program Counter:")
    pc = int(gdb.parse_and_eval("$rip")) + (int(gdb.parse_and_eval("$cs")) * 16)
    gdb.execute("x/i " + str(pc))
    print("---- Memory around 0x7c00 ----")
    gdb.execute("x/16b 0x7c00")

gdb.events.stop.connect(stop_handler)

# Connect to remote
gdb.execute("target remote localhost:1234")
gdb.execute("watch *0x7c00")
gdb.execute("break *0x7c00")

# add symbol
with open(binary_path, 'rb') as file:
    elf_file = ELFFile(file)
    symbol_offset = 0
    # get offset of symbol from readelf 
    for section in elf_file.iter_sections():
        if section.name == '.text':
            symbol_offset = section['sh_addr']
            break
    gdb.execute("add-symbol-file "+binary_path+" "+str(symbol_offset))
    gdb.execute("break main")
```
发现一个问题L0启动qemu后gdb连上后python里设置的kill on quit无法kill
（目前发现是gdb没通过socket（sendto）发送kill给qemu）改用gdb.init
## 参考
清华大学操作系统课程4.1启动顺序
64-ia-32-architectures-software-developer-manual,Volume 3,2.5 Control Registers
https://www.qemu.org/docs/master/system/gdb.html
https://wiki.gentoo.org/wiki/QEMU/Options