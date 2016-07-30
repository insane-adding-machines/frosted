#ifndef FROSTED_MPU_H
#define FROSTED_MPU_H

#ifdef CONFIG_MPU
void mpu_init(void);
void mpu_task_on(void *stack);
#else
#   define mpu_init() do{}while(0)
#   define mpu_task_on(x) do{}while(0)
#endif




#endif

