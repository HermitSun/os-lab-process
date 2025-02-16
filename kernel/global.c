
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                            global.c
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                                                    Forrest Yu, 2005
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

#define GLOBAL_VARIABLES_HERE

#include "type.h"
#include "const.h"
#include "protect.h"
#include "proto.h"
#include "proc.h"
#include "global.h"

PUBLIC PROCESS proc_table[NR_TASKS];

PUBLIC char task_stack[STACK_SIZE_TOTAL];

// 因为系统采用时间片轮转，所以放在前面的进程“优先级”较高
// 通过调整顺序即可实现读者/写者优先
// 读者优先
PUBLIC TASK task_table[NR_TASKS] = {
	{init, STACK_SIZE_INIT, "init"},
	{A, STACK_SIZE_A, "A"},
	{B, STACK_SIZE_B, "B"},
	{C, STACK_SIZE_C, "C"},
	{D, STACK_SIZE_D, "D"},
	{E, STACK_SIZE_E, "E"},
	{F, STACK_SIZE_F, "F"},
};

// 写者优先
// PUBLIC TASK task_table[NR_TASKS] = {
// 	{init, STACK_SIZE_INIT, "init"},
// 	{D, STACK_SIZE_D, "D"},
// 	{E, STACK_SIZE_E, "E"},
// 	{A, STACK_SIZE_A, "A"},
// 	{B, STACK_SIZE_B, "B"},
// 	{C, STACK_SIZE_C, "C"},
// 	{F, STACK_SIZE_F, "F"},
// };

PUBLIC irq_handler irq_table[NR_IRQ];

PUBLIC system_call sys_call_table[NR_SYS_CALL] = {
	sys_get_ticks,
	sys_sleep,
	sys_print,
	sys_color_print,
	sys_P,
	sys_V,
};
