#ifndef KPRINTF_H
#define KPRINTF_H

#ifdef CONFIG_KLOG
int klog_init(void);
int ksprintf(char *out, const char *format, ...);
int kprintf(const char *format, ...);
#else
#   define klog_init() (0)
#   define kprintf
#endif


#endif
