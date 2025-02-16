
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                            main.c
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                                                    Forrest Yu, 2005
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

#include "type.h"
#include "const.h"
#include "protect.h"
#include "proto.h"
#include "string.h"
#include "proc.h"
#include "global.h"

// 来自进程的读者数量
extern int num_readers;

/*======================================================================*
                            kernel_main
 *======================================================================*/
PUBLIC int kernel_main()
{
	disp_str("-----\"kernel_main\" begins-----\n");

	TASK *p_task = task_table;
	PROCESS *p_proc = proc_table;
	char *p_task_stack = task_stack + STACK_SIZE_TOTAL;
	u16 selector_ldt = SELECTOR_LDT_FIRST;
	int i;
	for (i = 0; i < NR_TASKS; i++)
	{
		strcpy(p_proc->p_name, p_task->name); // name of the process
		p_proc->pid = i;					  // pid

		p_proc->ldt_sel = selector_ldt;

		memcpy(&p_proc->ldts[0], &gdt[SELECTOR_KERNEL_CS >> 3],
			   sizeof(DESCRIPTOR));
		p_proc->ldts[0].attr1 = DA_C | PRIVILEGE_TASK << 5;
		memcpy(&p_proc->ldts[1], &gdt[SELECTOR_KERNEL_DS >> 3],
			   sizeof(DESCRIPTOR));
		p_proc->ldts[1].attr1 = DA_DRW | PRIVILEGE_TASK << 5;
		p_proc->regs.cs = ((8 * 0) & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | RPL_TASK;
		p_proc->regs.ds = ((8 * 1) & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | RPL_TASK;
		p_proc->regs.es = ((8 * 1) & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | RPL_TASK;
		p_proc->regs.fs = ((8 * 1) & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | RPL_TASK;
		p_proc->regs.ss = ((8 * 1) & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | RPL_TASK;
		p_proc->regs.gs = (SELECTOR_KERNEL_GS & SA_RPL_MASK) | RPL_TASK;

		p_proc->regs.eip = (u32)p_task->initial_eip;
		p_proc->regs.esp = (u32)p_task_stack;
		p_proc->regs.eflags = 0x1202; /* IF=1, IOPL=1 */

		p_task_stack -= p_task->stacksize;
		p_proc++;
		p_task++;
		selector_ldt += 1 << 3;
	}

	// 单位：tick，即10ms
	// 此处指第一个进程先运行1tick（10ms）后再运行第二个进程
	// 通过这种方式来使睡眠效果接近预期
	proc_table[0].ticks = proc_table[0].priority = 1;
	proc_table[1].ticks = proc_table[1].priority = 1;
	proc_table[2].ticks = proc_table[2].priority = 1;
	proc_table[3].ticks = proc_table[3].priority = 1;
	proc_table[4].ticks = proc_table[4].priority = 1;
	proc_table[5].ticks = proc_table[5].priority = 1;
	proc_table[6].ticks = proc_table[6].priority = 1;

	k_reenter = 0;
	ticks = 0;

	p_proc_ready = proc_table;

	/* 初始化 8253 PIT */
	out_byte(TIMER_MODE, RATE_GENERATOR);
	out_byte(TIMER0, (u8)(TIMER_FREQ / HZ));
	out_byte(TIMER0, (u8)((TIMER_FREQ / HZ) >> 8));

	put_irq_handler(CLOCK_IRQ, clock_handler); /* 设定时钟中断处理程序 */
	enable_irq(CLOCK_IRQ);					   /* 让8259A可以接收时钟中断 */

	// 清空屏幕
	disp_pos = 0;
	for (i = 0; i < 80 * 25; i++)
	{
		disp_str(" ");
	}
	disp_pos = 0;
	// 初始化信号量和控制变量
	sem_init();

	restart();

	while (1)
	{
	}
}

// 用于占位和协调的进程
// 如果没有这个进程，别的进程全部sleep，会导致死锁
void init()
{
	int i = 0;
	while (1)
	{
		++i;
		if (i > MAGIC_NUMBER)
		{
			i = 0;
		}
	}
}

// 读者进程，消耗2时间片
void A()
{
	int i = 0;
	while (1)
	{
		reader("A", 20000);

		++i;
		if (i > MAGIC_NUMBER)
		{
			i = 0;
		}
	}
}

// 读者进程，消耗3时间片
void B()
{
	int i = 0;
	while (1)
	{
		reader("B", 30000);

		++i;
		if (i > MAGIC_NUMBER)
		{
			i = 0;
		}
	}
}

// 读者进程，消耗3时间片
void C()
{
	int i = 0;
	while (1)
	{
		reader("C", 30000);

		++i;
		if (i > MAGIC_NUMBER)
		{
			i = 0;
		}
	}
}

// 写者进程，消耗3时间片
void D()
{
	int i = 0;
	while (1)
	{
		writer("D", 30000);

		++i;
		if (i > MAGIC_NUMBER)
		{
			i = 0;
		}
	}
}

// 写者进程，消耗4时间片
void E()
{
	int i = 0;
	while (1)
	{
		writer("E", 40000);

		++i;
		if (i > MAGIC_NUMBER)
		{
			i = 0;
		}
	}
}

// 普通进程，每隔1个时间片打印
void F()
{
	int i = 0;
	while (1)
	{
		// 如果在if-else里声明局部变量，会打印乱码；但是为什么？？？
		// 好像是disp_str本身实现的bug
		char readers_print[] = "Reading. Readers: ";
		char readers_count_print[32] = {'\0'};
		char writers_print[] = "Writing. \n";
		// 正在读
		if (num_readers)
		{
			print(readers_print);
			print(c_itoa(num_readers, readers_count_print, 10));
			print("\n");
		}
		// 正在写
		else
		{
			print(writers_print);
		}
		milli_delay(10000);

		++i;
		if (i > MAGIC_NUMBER)
		{
			i = 0;
		}
	}
}

/*======================================================================*
                               TestA
 *======================================================================*/
// void TestA()
// {
// 	int i = 0;
// 	while (1)
// 	{
// 		print("A.");
// 		milli_delay(1000);
// 	}
// }

/*======================================================================*
                               TestB
 *======================================================================*/
// void TestB()
// {
// 	int i = 0x1000;
// 	// 虽然预期睡眠时间是1000ms（100ticks），但是事实上要考虑其他进程花费的时间
// 	sleep(1000);
// 	while (1)
// 	{
// 		print("B.");
// 		milli_delay(1000);
// 	}
// }

/*======================================================================*
                               TestC
 *======================================================================*/
// void TestC()
// {
// 	int i = 0x2000;
// 	while (1)
// 	{
// 		print("C.");
// 		milli_delay(1000);
// 	}
// }
