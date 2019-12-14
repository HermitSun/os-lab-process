
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                               proc.h
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                                                    Forrest Yu, 2005
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

// 避免重复引入
#ifndef PROC_H_
#define PROC_H_

typedef struct s_stackframe
{					/* proc_ptr points here				↑ Low			*/
	u32 gs;			/* ┓						│			*/
	u32 fs;			/* ┃						│			*/
	u32 es;			/* ┃						│			*/
	u32 ds;			/* ┃						│			*/
	u32 edi;		/* ┃						│			*/
	u32 esi;		/* ┣ pushed by save()				│			*/
	u32 ebp;		/* ┃						│			*/
	u32 kernel_esp; /* <- 'popad' will ignore it			│			*/
	u32 ebx;		/* ┃						↑栈从高地址往低地址增长*/
	u32 edx;		/* ┃						│			*/
	u32 ecx;		/* ┃						│			*/
	u32 eax;		/* ┛						│			*/
	u32 retaddr;	/* return address for assembly code save()	│			*/
	u32 eip;		/*  ┓						│			*/
	u32 cs;			/*  ┃						│			*/
	u32 eflags;		/*  ┣ these are pushed by CPU during interrupt	│			*/
	u32 esp;		/*  ┃						│			*/
	u32 ss;			/*  ┛						┷High			*/
} STACK_FRAME;

typedef struct s_proc
{
	STACK_FRAME regs; /* process registers saved in stack frame */

	u16 ldt_sel;			   /* gdt selector giving ldt base and limit */
	DESCRIPTOR ldts[LDT_SIZE]; /* local descriptors for code and data */

	int ticks; /* remained ticks */
	int priority;
	// 进程睡眠时间
	int sleep_time;
	// 进程是否等待
	int is_wait;
	// 进程当前的颜色
	int color;

	u32 pid;		 /* process id passed in from MM */
	char p_name[16]; /* name of the process */
} PROCESS;

typedef struct s_task
{
	task_f initial_eip;
	int stacksize;
	char name[32];
} TASK;

// 信号量
// 结构体居然不能定义函数？
typedef struct s_semaphore
{
	// 值
	int value;
	// 进程队列（用数组模拟）
	PROCESS *queue[50];
	// 队列中进程的数量
	int size;
} SEMAPHORE;

/* Number of tasks */
#define NR_TASKS 7

/* stacks of tasks */
#define STACK_SIZE_INIT 0x8000
#define STACK_SIZE_A 0x8000
#define STACK_SIZE_B 0x8000
#define STACK_SIZE_C 0x8000
#define STACK_SIZE_D 0x8000
#define STACK_SIZE_E 0x8000
#define STACK_SIZE_F 0x8000

#define STACK_SIZE_TOTAL (STACK_SIZE_INIT + \
						  STACK_SIZE_A +    \
						  STACK_SIZE_B +    \
						  STACK_SIZE_C +    \
						  STACK_SIZE_D +    \
						  STACK_SIZE_E +    \
						  STACK_SIZE_F)

#endif