/* Copyright (C) 2017 Embecosm Limited and University of Bristol

   Contributor Graham Markall <graham.markall@embecosm.com>

   This file is part of Embench and was formerly part of the Bristol/Embecosm
   Embedded Benchmark Suite.

   SPDX-License-Identifier: GPL-3.0-or-later */


#include <stdint.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <limits.h>
#include <sys/signal.h>
//#include "util.h"

//#include <support.h>

#define SYS_write //64


extern volatile uint64_t tohost;
extern volatile uint64_t fromhost;


/*static uintptr_t syscall(uintptr_t which, uint64_t arg0, uint64_t arg1, uint64_t arg2)
{
  volatile uint64_t magic_mem[8] __attribute__((aligned(64)));
  magic_mem[0] = which;
  magic_mem[1] = arg0;
  magic_mem[2] = arg1;
  magic_mem[3] = arg2;
  __sync_synchronize();

  tohost = (uintptr_t)magic_mem;
  while (fromhost == 0)
    ;
  fromhost = 0;

  __sync_synchronize();
  return magic_mem[0];
}*/

#define NUM_COUNTERS 2
static uintptr_t counters[NUM_COUNTERS];
static char* counter_names[NUM_COUNTERS];

/*void setStats(int enable)
{
  int i = 0;
#define READ_CTR(name) do { \
    while (i >= NUM_COUNTERS) ; \
    uintptr_t csr = read_csr(name); \
    if (!enable) { csr -= counters[i]; counter_names[i] = #name; } \
    counters[i++] = csr; \
  } while (0)

  READ_CTR(mcycle);
  READ_CTR(minstret);

#undef READ_CTR
}*/
/*
void __attribute__((noreturn)) tohost_exit(uintptr_t code)
{
  tohost = (code << 1) | 1;
  while (1);
}

uintptr_t __attribute__((weak)) handle_trap(uintptr_t cause, uintptr_t epc, uintptr_t regs[32])
{
  tohost_exit(1337);
}
*/
void exit(int code)
{
  ;//tohost_exit(code);
  while(1);
}

void abort()
{
  exit(128 + SIGABRT);
}

uint32_t outp=0x07000000;
uint32_t incr=4;

//void printstr(const char* s)
/*{
  syscall(SYS_write, 1, (uintptr_t)s, strlen(s));
}*/
/*{	
	while (*s != 0){
		*((volatile uint32_t*)(outp)) = *(s++);
		outp+=incr;
	}
} */

/*void __attribute__((weak)) thread_entry(int cid, int nc)
{
  // multi-threaded programs override this function.
  // for the case of single-threaded programs, only let core 0 proceed.
  while (cid != 0);
}

int __attribute__((weak)) main(int argc, char** argv)
{
  // single-threaded programs override this function.
  printstr("Implement main(), foo!\n");
  return -1;
}*/

/*static void init_tls()
{
  register void* thread_pointer asm("tp");
  extern char _tls_data;
  extern __thread char _tdata_begin, _tdata_end, _tbss_end;
  size_t tdata_size = &_tdata_end - &_tdata_begin;
  memcpy(thread_pointer, &_tls_data, tdata_size);
  size_t tbss_size = &_tbss_end - &_tdata_end;
  memset(thread_pointer + tdata_size, 0, tbss_size);
}*/

/*
void _init(int cid, int nc)
{
  init_tls();
  thread_entry(cid, nc);

  // only single-threaded programs should ever get here.
  int ret = main(0, 0);

  char buf[NUM_COUNTERS * 32] __attribute__((aligned(64)));
  char* pbuf = buf;
  for (int i = 0; i < NUM_COUNTERS; i++)
    if (counters[i])
      pbuf += sprintf(pbuf, "%s = %d\n", counter_names[i], counters[i]);
  if (pbuf != buf)
    printstr(buf);

  exit(ret);
}*/

#undef putchar
inline int putchar(int ch)
/*{
  static __thread char buf[64] __attribute__((aligned(64)));
  static __thread int buflen = 0;

  buf[buflen++] = ch;

  if (ch == '\n' || buflen == sizeof(buf))
  {
    syscall(SYS_write, 1, (uintptr_t)buf, buflen);
    buflen = 0;
  }

  return 0;
}*/
{
	*((volatile uint32_t*)outp) = ch;
	outp+=4;
	return 0;
}


/*int
__sprint_r (struct _reent *ptr,
       FILE *fp,
       register struct __suio *uio)
{
	putchar('x');
	while (*ptr!=NULL){
		putchar(*((char)ptr));
		ptr++;
	}
}*/


//#undef _stdout_r(x) 
//#define _stdout_r(x)    (outp++)

// https://stackoverflow.com/questions/55014043/how-do-you-implement-printf-in-gcc-from-newlib
#include <unistd.h>
int _write(int handle, char *data, int size ) 
{//putchar('a');
if ((handle != STDOUT_FILENO) && (handle != STDERR_FILENO))
      return -1;

	for (int i=0; i<size; i++){
		*((volatile int*)(outp)) = data[i];
		outp+=4;
	}	
	//putchar('b');
	//putchar('\n');
	return size;//1
}

/*#include <_ansi.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/unistd.h>
*/
/*
void
__smakebuf_r (struct _reent *ptr,
       register FILE *fp)
{
  register void *p;
  int flags;
  size_t size;
  int couldbetty;

 // if (fp->_flags & __SNBF)
 //   {

      fp->_bf._base = fp->_p = fp->_nbuf;
      fp->_bf._size = 1;
      return;*/
 //   }
  /*flags = __swhatbuf_r (ptr, fp, &size, &couldbetty);
  if ((p = _malloc_r (ptr, size)) == NULL)
    {
      if (!(fp->_flags & __SSTR))
        {
          fp->_flags = (fp->_flags & ~__SLBF) | __SNBF;
          fp->_bf._base = fp->_p = fp->_nbuf;
          fp->_bf._size = 1;
        }
    }
  else
    {
      ptr->__cleanup = _cleanup_r;
      fp->_flags |= __SMBF;
      fp->_bf._base = fp->_p = (unsigned char *) p;
      fp->_bf._size = size;
      if (couldbetty && _isatty_r (ptr, fp->_file))
        fp->_flags = (fp->_flags & ~__SNBF) | __SLBF;
      fp->_flags |= flags;
    }*/
//}

/*extern int _open (const char* c, int k, ...){
putchar('A');putchar('\n');
	return 0;
}
int _close (int __fildes){
putchar('B');putchar('\n');
    return 0;
}
int _read (int fd, char *buf,size_t nbytes){
putchar('C');putchar('\n');
    return 0;
}
int _fstat (int fd, struct stat *buf){putchar('d');
	return 0;
}
extern int _fstat_r (struct _reent *, int, struct stat *){putchar('e');
	return 0;
}*/

/*
int
_link (char *old,
       char *new)
{

  //errno = EMLINK;
  //*new=*old;putchar('G');putchar('\n');
  return -1;             
}  

int
_kill (int  pid,
       int  sig)
{
  putchar('H');putchar('\n');//errno = EINVAL;
  return -1;                   // Always fails

}       // _kill ()      

int
_lseek (int   file,
        int   offset,
        int   whence)
{
putchar('D');putchar('\n');
  return  0;

}       // _lseek () 
	
	
//#undef errno
//extern int  errno;

//#define STACK_BUFFER  65536      Reserved stack space in bytes. 

void *
_sbrk (int nbytes)
{
  // Symbol defined by linker map 
  extern int  end;              //start of free memory (as symbol) 

  // Value set by crt0.S
  extern void *stack;           // end of free memory 

  // The statically held previous end of the heap, with its initialization. 
  static void *heap_ptr = (void *)&end;        // Previous end 
//putchar('E');putchar('\n');
  //if ((stack - (heap_ptr + nbytes)) > STACK_BUFFER )
    {
      void *base  = heap_ptr;
      heap_ptr   += nbytes;
                
      return  base;
    }
  //else
    //{
     // errno = ENOMEM;
     // return  (void *) -1;
    //}
}

#include <sys/stat.h>

int
_stat (char        *file,
       struct stat *st)
{
  putchar('F');putchar('\n');
  st->st_mode = S_IFCHR;
  return 0;

}      
int
_wait (int *status)
{
  putchar('H');putchar('\n');
  //errno = ECHILD;
  return -1;                    //Always fails 

} 

int
_fork ()
{
  putchar('I');putchar('\n');
  //errno = EAGAIN;
  return -1;                //Always fails

}      
int
_isatty (int   file)
{
  putchar('J');putchar('\n');
  return  1;

} 

int
_unlink (char *name)
{
 // errno = ENOENT;
  return -1;              //Always fails

} 

int
fseek (register FILE *fp,
       long offset,
       int whence)
{
  return 0;
}
int
_fork_r ( struct _reent *ptr )
{
// return "not supported" 
return -1;
}*/



/*
void printhex(uint64_t x)
{
  char str[17];
  int i;
  for (i = 0; i < 16; i++)
  {
    str[15-i] = (x & 0xF) + ((x & 0xF) < 10 ? '0' : 'a'-10);
    x >>= 4;
  }
  str[16] = 0;

  printstr(str);
}*/
/*
static inline void printnum(void (*putch)(int, void**), void **putdat,
                    unsigned long long num, unsigned base, int width, int padc)
{
  unsigned digs[sizeof(num)*CHAR_BIT];
  int pos = 0;

  while (1)
  {
    digs[pos++] = num % base;
    if (num < base)
      break;
    num /= (unsigned long long)base;
  }

  while (width-- > pos)
    putch(padc, putdat);

  while (pos-- > 0)
    putch(digs[pos] + (digs[pos] >= 10 ? 'a' - 10 : '0'), putdat);
}

static unsigned long long getuint(va_list *ap, int lflag)
{
  if (lflag >= 2){
    unsigned long long x = //va_arg(*ap, unsigned int);
    va_arg(*ap, unsigned long long);
    //x<<=32;
    //x|=va_arg(*ap, unsigned int)<<32;
    return x;//(x>>32L) + (x<<32L);
  }else if (lflag)
    return va_arg(*ap, unsigned long);
  else
    return va_arg(*ap, unsigned int);
}

static long long getint(va_list *ap, int lflag)
{
  if (lflag >= 2)
    return va_arg(*ap, long long);
  else if (lflag)
    return va_arg(*ap, long);
  else
    return va_arg(*ap, int);
}

static void vprintfmt(void (*putch)(int, void**), void **putdat, const char *fmt, va_list ap)
{
  register const char* p;
  const char* last_fmt;
  register int ch, err;
  unsigned long long num;
  int base, lflag, width, precision, altflag;
  char padc;

  while (1) {
    while ((ch = *(unsigned char *) fmt) != '%') {
      if (ch == '\0')
        return;
      fmt++;
      putch(ch, putdat);
    }
    fmt++;

    // Process a %-escape sequence
    last_fmt = fmt;
    padc = ' ';
    width = -1;
    precision = -1;
    lflag = 0;
    altflag = 0;
  reswitch:
    switch (ch = *(unsigned char *) fmt++) {

    // flag to pad on the right
    case '-':
      padc = '-';
      goto reswitch;
      
    // flag to pad with 0's instead of spaces
    case '0':
      padc = '0';
      goto reswitch;

    // width field
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
      for (precision = 0; ; ++fmt) {
        precision = precision * 10 + ch - '0';
        ch = *fmt;
        if (ch < '0' || ch > '9')
          break;
      }
      goto process_precision;

    case '*':
      precision = va_arg(ap, int);
      goto process_precision;

    case '.':
      if (width < 0)
        width = 0;
      goto reswitch;

    case '#':
      altflag = 1;
      goto reswitch;

    process_precision:
      if (width < 0)
        width = precision, precision = -1;
      goto reswitch;

    // long flag (doubled for long long)
    case 'l':
      lflag+=1;
      goto reswitch;

    // character
    case 'c':
      putch(va_arg(ap, int), putdat);
      break;

    // string
    case 's':
      if ((p = va_arg(ap, char *)) == NULL)
        p = "(null)";
      if (width > 0 && padc != '-')
        for (width -= strnlen(p, precision); width > 0; width--)
          putch(padc, putdat);
      for (; (ch = *p) != '\0' && (precision < 0 || --precision >= 0); width--) {
        putch(ch, putdat);
        p++;
      }
      for (; width > 0; width--)
        putch(' ', putdat);
      break;

    // (signed) decimal
    case 'd':
      num = getint(&ap, lflag);
      if ((long long) num < 0) {
        putch('-', putdat);
        num = -(long long) num;
      }
      base = 10;
      goto signed_number;

    // unsigned decimal
    case 'u':
      base = 10;
      goto unsigned_number;

    // (unsigned) octal
    case 'o':
      // should do something with padding so it's always 3 octits
      base = 8;
      goto unsigned_number;

    // pointer
    case 'p':
      //static_assert(sizeof(long) == sizeof(void*));
      lflag = 1;
      putch('0', putdat);
      putch('x', putdat);
      // fall through to 'x' 

    // (unsigned) hexadecimal
    case 'x':
      base = 16;
    unsigned_number:
      num = getuint(&ap, lflag);
    signed_number:
      //if (width==1)
      //putch('a'+lflag, putdat);
      printnum(putch, putdat, num, base, width, padc);
      break;

    // escaped '%' character
    case '%':
      putch(ch, putdat);
      //lflag = 0;
      break;
      
    // unrecognized escape sequence - just print it literally
    default:      
      putch('%', putdat);
      fmt = last_fmt;
      break;
    }
  }
}

int printf2(const char* fmt, ...)
{
  va_list ap;
  va_start(ap, fmt);

  vprintfmt((void*)putchar, 0, fmt, ap);

  va_end(ap);
  return 0; // incorrect return value, but who cares, anyway?
}*/
/*
int sprintf(char* str, const char* fmt, ...)
{
  va_list ap;
  char* str0 = str;
  va_start(ap, fmt);

  void sprintf_putch(int ch, void** data)
  {
    char** pstr = (char**)data;
    **pstr = ch;
    (*pstr)++;
  }

  vprintfmt(sprintf_putch, (void**)&str, fmt, ap);
  *str = 0;

  va_end(ap);
  return str - str0;
}
*//*
void* memcpy(void* dest, const void* src, size_t len)
{
  if ((((uintptr_t)dest | (uintptr_t)src | len) & (sizeof(uintptr_t)-1)) == 0) {
    const uintptr_t* s = src;
    uintptr_t *d = dest;
    while (d < (uintptr_t*)(dest + len))
      *d++ = *s++;
  } else {
    const char* s = src;
    char *d = dest;
    while (d < (char*)(dest + len))
      *d++ = *s++;
  }
  return dest;
}*/
/*
void* memset(void* dest, int byte, size_t len)
{
  if ((((uintptr_t)dest | len) & (sizeof(uintptr_t)-1)) == 0) {
    uintptr_t word = byte & 0xFF;
    word |= word << 8;
    word |= word << 16;
    word |= word << 16 << 16;

    uintptr_t *d = dest;
    while (d < (uintptr_t*)(dest + len))
      *d++ = word;
  } else {
    char *d = dest;
    while (d < (char*)(dest + len))
      *d++ = byte;
  }
  return dest;
}

size_t strlen(const char *s)
{
  const char *p = s;
  while (*p)
    p++;
  return p - s;
}

size_t strnlen(const char *s, size_t n)
{
  const char *p = s;
  while (n-- && *p)
    p++;
  return p - s;
}*/

/*#undef strcmp
int strcmp(const char* s1, const char* s2)
{
  unsigned char c1, c2;

  do {
    c1 = *s1++;
    c2 = *s2++;
  } while (c1 != 0 && c1 == c2);

  return c1 - c2;
}
*/
//#undef strcmp
/*
char* strcpy(char* dest, const char* src)
{
  char* d = dest;
  while ((*d++ = *src++))
    ;
  return dest;
}

long atol(const char* str)
{
  long res = 0;
  int sign = 0;

  while (*str == ' ')
    str++;

  if (*str == '-' || *str == '+') {
    sign = *str == '-';
    str++;
  }

  while (*str) {
    res *= 10;
    res += *str++ - '0';
  }

  return sign ? -res : res;
}

*/
/*
uint64_t time()
{
	uint32_t cycles_low;
	asm volatile ("rdcycle %0" : "=r"(cycles_low));
	uint32_t cycles_high;	
	asm volatile ("rdcycleh %0" : "=r"(cycles_high));
	return ((uint64_t)cycles_high<<32)|cycles_low;
}

uint64_t insn()
{
	uint32_t instructions_low;
	asm volatile ("rdinstret %0" : "=r"(instructions_low));
	uint32_t instructions_high;
	asm volatile ("rdinstreth %0" : "=r"(instructions_high));
	return ((uint64_t)instructions_high<<32)|instructions_low;
}*/




/*



uint64_t start_time = 0;
uint64_t start_insn = 0;


void __attribute__ ((noinline)) __attribute__ ((externally_visible))
start_trigger ()
{
  //__asm__ volatile ("li a0, 0" : : : "memory");
  //printf (" cycles %llu \n", time());
  start_time = (volatile) time(); 
  start_insn = (volatile) insn(); 
}

void __attribute__ ((noinline)) __attribute__ ((externally_visible))
stop_trigger ()
{
  //__asm__ volatile ("li a0, 0" : : : "memory");
  volatile uint64_t end_time = time();
  volatile uint64_t end_insn = insn();
  printf ("cycles %llu instructions %llu ", end_time-start_time, end_insn-start_insn);
}*/


