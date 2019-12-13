
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                               proc.c
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

PRIVATE void push(SEMAPHORE *s, PROCESS *p);
PRIVATE PROCESS *pop(SEMAPHORE *s);
PUBLIC void clear_screen();

/*======================================================================*
                              schedule
 *======================================================================*/
PUBLIC void schedule()
{
	PROCESS *p;
	int greatest_ticks = 0;

	// 如果进程在睡眠，每次调度时减去睡眠时间
	for (p = proc_table; p < proc_table + NR_TASKS; p++)
	{
		if (p->sleep_time)
		{
			--p->sleep_time;
			// disp_int(p->sleep_time);
		}
	}
	while (!greatest_ticks)
	{
		for (p = proc_table; p < proc_table + NR_TASKS; p++)
		{
			if (p->sleep_time || p->is_wait)
			{
				// 睡眠或等待时不分配时间片
				continue;
			}
			if (p->ticks > greatest_ticks)
			{
				greatest_ticks = p->ticks;
				p_proc_ready = p;
			}
		}

		if (!greatest_ticks)
		{
			for (p = proc_table; p < proc_table + NR_TASKS; p++)
			{
				if (p->sleep_time || p->is_wait)
				{
					// 睡眠时不分配时间片
					continue;
				}
				p->ticks = p->priority;
			}
		}
	}
}

/*======================================================================*
                           sys_get_ticks
 *======================================================================*/
PUBLIC int sys_get_ticks()
{
	return ticks;
}

// 进程睡眠，不分配时间片
// 事实上睡眠的总时间要考虑其他进程，因为每次调度时才会检查一次
PUBLIC int sys_sleep(int milli_sec)
{
	// ms转换成tick
	p_proc_ready->sleep_time = milli_sec / 10;
	schedule();
	return 0;
}

// 打印字符串
PUBLIC int sys_print(char *str)
{
	disp_str(str);
	return 0;
}

// 信号量P操作
PUBLIC int sys_P(SEMAPHORE *s)
{
	--s->value;
	// 没有可用资源，等待
	if (s->value < 0)
	{
		// 当前进程等待并进入队列
		p_proc_ready->is_wait = 1;
		push(s, p_proc_ready);
		schedule();
	}
	return 0;
}

// 信号量V操作
PUBLIC int sys_V(SEMAPHORE *s)
{
	++s->value;
	// 有等待进程
	if (s->value <= 0)
	{
		// 退出队列并唤醒
		PROCESS *wait_process = pop(s);
		wait_process->is_wait = 0;
	}
	return 0;
}

// 信号量和控制变量
SEMAPHORE s_reader;
SEMAPHORE s_writer;
SEMAPHORE s_mutex;
SEMAPHORE *p_s_reader;
SEMAPHORE *p_s_writer;
SEMAPHORE *p_s_mutex;
int num_readers;

// 首先说明，因为屏幕不够大，通过颜色来区分状态
// 红色开始读写，蓝色正在读写，绿色读写完成
//----------------------------------------------------------------
// 以下为读者优先
//----------------------------------------------------------------

// // 初始化信号量
// PUBLIC int sem_init()
// {
// 	// 允许同时读的读者数量
// 	s_reader.value = 3;
// 	s_reader.size = 0;
// 	s_writer.value = 1;
// 	s_writer.size = 0;
// 	num_readers = 0;
// 	p_s_reader = &s_reader;
// 	p_s_writer = &s_writer;
// 	return 0;
// }

// PUBLIC int reader(char *name, int cost)
// {
// 	P(p_s_reader);
// 	if (num_readers == 0)
// 	{
// 		P(p_s_writer);
// 	}
// 	++num_readers;
// 	// V(p_s_reader);

// 	// 读开始
// 	disp_color_str(name, COLOR_RED);
// 	// disp_color_str(" starts. ", COLOR_BLUE);
// 	sleep(cost);
// 	// 正在读
// 	disp_color_str(name, COLOR_BLUE);
// 	// disp_color_str(" reading. ", COLOR_GREEN);

// 	// 读完成
// 	disp_color_str(name, COLOR_GREEN);
// 	// disp_color_str(" ends. ", COLOR_RED);

// 	// P(p_s_reader);
// 	--num_readers;
// 	if (num_readers == 0)
// 	{
// 		V(p_s_writer);
// 	}
// 	V(p_s_reader);
// 	return 0;
// }

// PUBLIC int writer(char *name, int cost)
// {
// 	P(p_s_writer);

// 	// 写开始
// 	disp_color_str(name, COLOR_RED);
// 	// disp_color_str(" starts. ", COLOR_BLUE);
// 	sleep(cost);
// 	// 正在写
// 	disp_color_str(name, COLOR_BLUE);
// 	// disp_color_str(" writing. ", COLOR_GREEN);
// 	// 写完成
// 	disp_color_str(name, COLOR_GREEN);
// 	// disp_color_str(" ends. ", COLOR_RED);

// 	V(p_s_writer);
// 	return 0;
// }

//----------------------------------------------------------------
// 以下为写者优先
//----------------------------------------------------------------

// 初始化信号量
PUBLIC int sem_init()
{
	// 允许同时读的读者数量
	s_reader.value = 3;
	s_reader.size = 0;
	// 只允许1个写者同时写
	s_writer.value = 1;
	s_writer.size = 0;
	s_mutex.value = 1;
	s_mutex.size = 0;
	num_readers = 0;
	p_s_reader = &s_reader;
	p_s_writer = &s_writer;
	p_s_mutex = &s_mutex;
	return 0;
}

PUBLIC int reader(char *name, int cost)
{
	// 请求互斥
	P(p_s_mutex);
	P(p_s_reader);
	if (num_readers == 0)
	{
		P(p_s_writer);
	}
	++num_readers;
	// V(p_s_reader);
	// 释放互斥
	V(p_s_mutex);

	// 读开始
	clear_screen();
	disp_color_str(name, COLOR_RED);
	disp_color_str(" starts. ", COLOR_RED);
	sleep(cost);
	// 正在读
	clear_screen();
	disp_color_str(name, COLOR_BLUE);
	disp_color_str(" readed. ", COLOR_BLUE);
	// 读完成
	clear_screen();
	disp_color_str(name, COLOR_GREEN);
	disp_color_str(" ends. ", COLOR_GREEN);

	// P(p_s_reader);
	--num_readers;
	if (num_readers == 0)
	{
		V(p_s_writer);
	}
	V(p_s_reader);
	return 0;
}

PUBLIC int writer(char *name, int cost)
{
	// 请求互斥
	P(p_s_mutex);
	P(p_s_writer);

	// 写开始
	clear_screen();
	disp_color_str(name, COLOR_RED);
	disp_color_str(" starts. ", COLOR_RED);
	sleep(cost);
	// 正在写
	clear_screen();
	disp_color_str(name, COLOR_BLUE);
	disp_color_str(" writed. ", COLOR_BLUE);
	// 写完成
	clear_screen();
	disp_color_str(name, COLOR_GREEN);
	disp_color_str(" ends. ", COLOR_GREEN);

	V(p_s_writer);
	// 释放互斥
	V(p_s_mutex);

	return 0;
}

// 因为结构体不能直接定义函数，所以拿出来了
// 入队
PRIVATE void push(SEMAPHORE *s, PROCESS *p)
{
	if (s->size < 50)
	{
		s->queue[s->size] = p;
		++s->size;
	}
}
// 出队
PRIVATE PROCESS *pop(SEMAPHORE *s)
{
	PROCESS *p = s->queue[0];
	for (int i = 0; i < s->size - 1; ++i)
	{
		s->queue[i] = s->queue[i + 1];
	}
	--s->size;
	return p;
}

// （在有必要的情况下）清空屏幕
PUBLIC void clear_screen()
{
	if (disp_pos >= 80 * 25 * 2 - 80 * 4)
	{
		disp_pos = 0;
		for (int i = 0; i < 80 * 25 * 2; i++)
		{
			print(" ");
		}
		disp_pos = 0;
	}
}