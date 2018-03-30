#ifndef PTY_H_INCLUDED
#define PTY_H_INCLUDED

#ifndef CONFIG_PTY_UNIX
#   define ptmx_init() do{}while(0)
#else
    int ptmx_init(void);
#endif

#endif
