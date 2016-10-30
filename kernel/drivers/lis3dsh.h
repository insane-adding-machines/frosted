#ifndef LIS3DSH_INC
#define LIS3DSH_INC
#   ifdef CONFIG_DEVLIS3DSH
        int lis3dsh_init(uint8_t bus, const struct gpio_config *lis3dsh_cs);
#   else
#       define lis3dsh_init(...) (0)
#   endif
#endif
