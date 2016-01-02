#ifndef INC_STM32F4EXTI
#define INC_STM32F4EXTI

struct exti_addr {
    uint32_t base;
    uint32_t pin;
    uint8_t trigger;
    const char* name;
};

typedef void (* exti_callback)(void * arg);

void exti_init(struct fnode * dev,  const struct exti_addr exti_addrs[], int num_extis);
void exti_register_callback(struct fnode *fno, exti_callback callback_fn, void * callback_arg);

int exti_enable(struct fnode * fno, int enable);

#endif
