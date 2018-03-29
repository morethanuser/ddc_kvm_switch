#ifndef _H_MAIN_
#define _H_MAIN_

#define __AVR_LIBC_DEPRECATED_ENABLE__

#define  F_CPU 8000000L


#define H(a,b) a |=(1<<(b))
#define L(a,b) a &=~(1<<(b))
#define IS(a,b) bit_is_set(a,b)

#endif


#ifndef F_CPU
   #error CPU speed unknown
#endif


