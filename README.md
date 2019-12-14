## 操作系统第四次实验 - 进程调度

### 问题清单

#### 1.  进程是什么 

从宏观来看，它有自己的目标（或者说功能），同时又能受控于进程调度模块；

从微观来看，它可以利用系统的资源，有代码块、数据块、堆栈块；

进程可以（也必须）被调度。

“唤醒”一个进程：将esp指向进程表（PCB）的开始处，然后lldt加载局部描述符表，然后pop恢复各个寄存器的值。

#### 2.  进程表是什么 

即进程控制块PCB，存储进程状态信息的数据结构，**独立于进程**。事实上，会存在很多个进程表。

当将寄存器值压入进程表时，已经处在进程管理模块中了。

#### 3.  进程栈是什么 

进程运行时自身的堆栈。

#### 4.  当寄存器的值已经被保存到进程表内，esp 应指向何处来避免破坏进程表的值

一定要清楚自己使用的是哪个栈，否则可能会意外破坏进程表。

此时esp应指向内核栈，即进程调度模块运行时使用的堆栈。按照Orange's上的代码：

   ```assembly
[SECTION .bss]
StackSpace		resb	2 * 1024
StackTop		; 栈顶
   ```

#### 5. tty 是什么

TTY设备包括虚拟控制台，串口以及伪终端设备。可以简单理解成终端。

TTY是TeleTYpe的一个老缩写。

#### 6. 不同的 tty 为什么输出不同的画面在同一个显示器上

因为使用了显存的不同位置。

#### 7. 解释 tty 任务执行的过程

轮询tty，然后处理输入（查看是不是当前tty，并且tty需要使用console，如果是则从键盘缓冲区读取输入）输出（显示需要显示的内容）。

#### 8. tty 结构体中大概包括哪些内容

按照Orange's中的写法：

```c
typedef struct s_tty
{
	u32	in_buf[TTY_IN_BYTES];	/* TTY 输入缓冲区 */
	u32*	p_inbuf_head;		/* 指向缓冲区中下一个空闲位置 */
	u32*	p_inbuf_tail;		/* 指向键盘任务应处理的键值 */
	int	inbuf_count;			/* 缓冲区中已经填充了多少 */

	struct s_console *	p_console;
}TTY;
```

#### 9. console 结构体中大概包括哪些内容

按照Orange's中的写法：

```c
typedef struct s_console
{
	unsigned int	current_start_addr;	/* 当前显示到了什么位置*/
	unsigned int	original_addr;		/* 当前控制台对应显存位置 */
	unsigned int	v_mem_limit;		/* 当前控制台占的显存大小 */
	unsigned int	cursor;				/* 当前光标位置 */
}CONSOLE;
```

#### 主要修改的文件

include/const.h

include/proc.h

include/proto.h

kernel/global.c

kernel/kernel.asm

kernel/main.c

kernel/proc.c

kernel/syscall.asm

lib/klib.c

Makefile

#### Orange's添加进程的过程

main.c添加一个进程

global.c任务表task_table里添加任务

proc.h里添加新进程使用的栈空间

proto.h里添加原型声明

#### 中断重入

不为0说明发生了嵌套中断