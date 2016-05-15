#ifndef KPRINTF_H
#define KPRINTF_H

int klog_init(void);
int ksprintf(char *out, const char *format, ...);
int kprintf(const char *format, ...);


#endif
