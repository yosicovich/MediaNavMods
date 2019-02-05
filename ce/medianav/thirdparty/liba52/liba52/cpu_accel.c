/*
 * cpu_accel.c
 * Copyright (C) 2000-2004 Michel Lespinasse <walken@zoy.org>
 * Copyright (C) 1999-2000 Aaron Holtzman <aholtzma@ess.engr.uvic.ca>
 *
 * This file is part of mpeg2dec, a free MPEG-2 video stream decoder.
 * See http://libmpeg2.sourceforge.net/ for updates.
 *
 * mpeg2dec is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * mpeg2dec is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <a52_config.h>

#include <inttypes.h>

#include "a52.h"
#include "attributes.h"
#include "a52_internal.h"

#ifdef ARCH_X86
static inline uint32_t arch_accel (uint32_t accel)
{
    if (accel & (A52_ACCEL_X86_3DNOW | A52_ACCEL_X86_MMXEXT))
	accel |= A52_ACCEL_X86_MMX;
	
    if (accel & (A52_ACCEL_X86_SSE2 | A52_ACCEL_X86_SSE3))
	accel |= A52_ACCEL_X86_MMXEXT;
	
    if (accel & (A52_ACCEL_X86_SSE3))
	accel |= A52_ACCEL_X86_SSE2;

#ifdef ACCEL_DETECT
    if (accel & A52_ACCEL_DETECT) {
	uint32_t eax, ebx, ecx, edx;
	int AMD;

#if defined(__x86_64__) || (!defined(PIC) && !defined(__PIC__))
#define cpuid(op,eax,ebx,ecx,edx)	\
    __asm__ ("cpuid"			\
	     : "=a" (eax),		\
	       "=b" (ebx),		\
	       "=c" (ecx),		\
	       "=d" (edx)		\
	     : "a" (op)			\
	     : "cc")
#else	/* PIC version : save ebx (not needed on x86_64) */
#define cpuid(op,eax,ebx,ecx,edx)	\
    __asm__ ("pushl %%ebx\n\t"		\
	     "cpuid\n\t"		\
	     "movl %%ebx,%1\n\t"	\
	     "popl %%ebx"		\
	     : "=a" (eax),		\
	       "=r" (ebx),		\
	       "=c" (ecx),		\
	       "=d" (edx)		\
	     : "a" (op)			\
	     : "cc")
#endif

#ifndef __x86_64__ /* x86_64 supports the cpuid op */
	__asm__ ("pushf\n\t"
		 "pushf\n\t"
		 "pop %0\n\t"
		 "movl %0,%1\n\t"
		 "xorl $0x200000,%0\n\t"
		 "push %0\n\t"
		 "popf\n\t"
		 "pushf\n\t"
		 "pop %0\n\t"
		 "popf"
		 : "=r" (eax),
		 "=r" (ebx)
		 :
		 : "cc");

	if (eax == ebx)			/* no cpuid */
	    return accel;
#endif

	cpuid (0x00000000, eax, ebx, ecx, edx);
	if (!eax)			/* vendor string only */
	    return accel;

	AMD = (ebx == 0x68747541 && ecx == 0x444d4163 && edx == 0x69746e65);

	cpuid (0x00000001, eax, ebx, ecx, edx);
	if (! (edx & 0x00800000))	/* no MMX */
	    return accel;

	accel |= A52_ACCEL_X86_MMX;
	if (edx & 0x02000000)		/* SSE - identical to AMD MMX ext. */
	    accel |= A52_ACCEL_X86_MMXEXT;

	if (edx & 0x04000000)		/* SSE2 */
	    accel |= A52_ACCEL_X86_SSE2;

	if (ecx & 0x00000001)		/* SSE3 */
	    accel |= A52_ACCEL_X86_SSE3;

	cpuid (0x80000000, eax, ebx, ecx, edx);
	if (eax < 0x80000001)		/* no extended capabilities */
	    return accel;

	cpuid (0x80000001, eax, ebx, ecx, edx);

	if (edx & 0x80000000)
	    accel |= A52_ACCEL_X86_3DNOW;

	if (AMD && (edx & 0x00400000))	/* AMD MMX extensions */
	    accel |= A52_ACCEL_X86_MMXEXT;
    }
#endif /* ACCEL_DETECT */

    return accel;
}
#endif /* ARCH_X86 */

#if defined(ACCEL_DETECT) && (defined(ARCH_PPC) || defined(ARCH_SPARC))
#include <signal.h>
#include <setjmp.h>

static sigjmp_buf jmpbuf;
static volatile sig_atomic_t canjump = 0;

static RETSIGTYPE sigill_handler (int sig)
{
    if (!canjump) {
	signal (sig, SIG_DFL);
	raise (sig);
    }

    canjump = 0;
    siglongjmp (jmpbuf, 1);
}
#endif /* ACCEL_DETECT && (ARCH_PPC || ARCH_SPARC) */

#ifdef ARCH_PPC
static uint32_t arch_accel (uint32_t accel)
{
#ifdef ACCEL_DETECT
    if ((accel & (A52_ACCEL_PPC_ALTIVEC | A52_ACCEL_DETECT)) ==
	A52_ACCEL_DETECT) {
	static RETSIGTYPE (* oldsig) (int);

	oldsig = signal (SIGILL, sigill_handler);
	if (sigsetjmp (jmpbuf, 1)) {
	    signal (SIGILL, oldsig);
	    return accel;
	}

	canjump = 1;

#ifdef HAVE_ALTIVEC_H	/* gnu */
#define VAND(a,b,c) "vand " #a "," #b "," #c "\n\t"
#else			/* apple */
#define VAND(a,b,c) "vand v" #a ",v" #b ",v" #c "\n\t"
#endif
	asm volatile ("mtspr 256, %0\n\t"
		      VAND (0, 0, 0)
		      :
		      : "r" (-1));

	canjump = 0;
	accel |= A52_ACCEL_PPC_ALTIVEC;

	signal (SIGILL, oldsig);
    }
#endif /* ACCEL_DETECT */

    return accel;
}
#endif /* ARCH_PPC */

#ifdef ARCH_SPARC
static uint32_t arch_accel (uint32_t accel)
{
    if (accel & A52_ACCEL_SPARC_VIS2)
	accel |= A52_ACCEL_SPARC_VIS;

#ifdef ACCEL_DETECT
    if (accel & (A52_ACCEL_SPARC_VIS2 | A52_ACCEL_DETECT) ==
	A52_ACCEL_DETECT) {
	static RETSIGTYPE (* oldsig) (int);

	oldsig = signal (SIGILL, sigill_handler);
	if (sigsetjmp (jmpbuf, 1)) {
	    signal (SIGILL, oldsig);
	    return accel;
	}

	canjump = 1;

	/* pdist %f0, %f0, %f0 */
	__asm__ __volatile__(".word\t0x81b007c0");

	canjump = 0;
	accel |= A52_ACCEL_SPARC_VIS;

	if (sigsetjmp (jmpbuf, 1)) {
	    signal (SIGILL, oldsig);
	    return accel;
	}

	canjump = 1;

	/* edge8n %g0, %g0, %g0 */
	__asm__ __volatile__(".word\t0x81b00020");

	canjump = 0;
	accel |= A52_ACCEL_SPARC_VIS2;

	signal (SIGILL, oldsig);
    }
#endif /* ACCEL_DETECT */

    return accel;
}
#endif /* ARCH_SPARC */

#ifdef ARCH_ALPHA
static inline uint32_t arch_accel (uint32_t accel)
{
    if (accel & A52_ACCEL_ALPHA_MVI)
	accel |= A52_ACCEL_ALPHA;

#ifdef ACCEL_DETECT
    if (accel & A52_ACCEL_DETECT) {
	uint64_t no_mvi;

	asm volatile ("amask %1, %0"
		      : "=r" (no_mvi)
		      : "rI" (256));	/* AMASK_MVI */
	accel |= no_mvi ? A52_ACCEL_ALPHA : (A52_ACCEL_ALPHA |
					       A52_ACCEL_ALPHA_MVI);
    }
#endif /* ACCEL_DETECT */

    return accel;
}
#endif /* ARCH_ALPHA */

uint32_t a52_detect_accel (uint32_t accel)
{
#if defined (ARCH_X86) || defined (ARCH_PPC) || defined (ARCH_ALPHA) || defined (ARCH_SPARC)
    accel = arch_accel (accel);
#endif
    return accel;
}
