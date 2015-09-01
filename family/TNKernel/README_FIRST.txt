
  For TNKernel v.2.6 and later you should set (in an IDE or makefile)
the preprocessor defenition according to you port:

   - for Cortex M4F (optional FPU support) - TNKERNEL_PORT_CORTEXM4F

   (see file 'tn_port.h' for more details)

otherwise you'll get the preprocessor error "TNKernel port undefined".

