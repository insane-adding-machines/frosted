
ifeq ($(USERSPACE_MINI),y)
  USERSPACE=frosted-mini-userspace
endif

ifeq ($(USERSPACE_BFLT),y)
  USERSPACE=frosted-mini-userspace-bflt
endif
