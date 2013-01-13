/*
 *  This file is part of the Xen Crashdump Analyser.
 *
 *  The Xen Crashdump Analyser is free software: you can redistribute
 *  it and/or modify it under the terms of the GNU General Public
 *  License as published by the Free Software Foundation, either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  The Xen Crashdump Analyser is distributed in the hope that it will
 *  be useful, but WITHOUT ANY WARRANTY; without even the implied
 *  warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *  See the GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with the Xen Crashdump Analyser.  If not, see
 *  <http://www.gnu.org/licenses/>.
 *
 *  Copyright (c) 2012 Citrix Inc.
 */

#ifndef __X86_64_STRUCTURES_HPP__
#define __X86_64_STRUCTURES_HPP__

/**
 * @file include/arch/x86_64/structures.hpp
 * @author Andrew Cooper
 */

/**
 * 64bit CPU registers.
 *
 * Contains all general purpose REX registers, unions to contain 32bit registers
 * as well, all segment registers, flags and CR3.
 */
struct x86_64regs
{
    /// @cond
    uint64_t r15, r14, r13, r12, r11, r10, r9, r8;
    union { uint64_t rax; uint32_t eax; };
    union { uint64_t rbx; uint32_t ebx; };
    union { uint64_t rcx; uint32_t ecx; };
    union { uint64_t rdx; uint32_t edx; };
    union { uint64_t rsi; uint32_t esi; };
    union { uint64_t rdi; uint32_t edi; };
    union { uint64_t rbp; uint32_t ebp; };
    union { uint64_t rsp; uint32_t esp; };
    union { uint64_t rip; uint32_t eip; };
    uint16_t cs, ds, es, fs, gs, ss;
    union { uint64_t orig_rax; uint32_t orig_eax; };
    union { uint64_t rflags; uint32_t eflags; };
    uint64_t cr0, cr2, cr3, cr4;
    /// @endcond
};


/**
 * 64bit Xen CPU user regs structure.  Transcribed from Xen
 */
struct x86_64_cpu_user_regs {
    /// @cond
#define __DECL_REG(reg) uint64_t r##reg
    uint64_t r15;
    uint64_t r14;
    uint64_t r13;
    uint64_t r12;
    __DECL_REG(bp);
    __DECL_REG(bx);
    uint64_t r11;
    uint64_t r10;
    uint64_t r9;
    uint64_t r8;
    __DECL_REG(ax);
    __DECL_REG(cx);
    __DECL_REG(dx);
    __DECL_REG(si);
    __DECL_REG(di);
    uint32_t error_code;    /* private */
    uint32_t entry_vector;  /* private */
    __DECL_REG(ip);
    uint16_t cs, _pad0[1];
    uint8_t  saved_upcall_mask;
    uint8_t  _pad1[3];
    __DECL_REG(flags);      /* rflags.IF == !saved_upcall_mask */
    __DECL_REG(sp);
    uint16_t ss, _pad2[3];
    uint16_t es, _pad3[3];
    uint16_t ds, _pad4[3];
    uint16_t fs, _pad5[3]; /* Non-zero => takes precedence over fs_base.     */
    uint16_t gs, _pad6[3]; /* Non-zero => takes precedence over gs_base_usr. */
#undef __DECL_REG
    /// @endcond
};

/// x86_64 crash xen core note
typedef struct {
    /// @cond
    uint64_t cr0, cr2, cr3, cr4;
    /// @endcond
} x86_64_crash_xen_core_t;

/// @cond
typedef struct
{
	int32_t signo;			/* signal number */
	int32_t code;			/* extra code */
	int32_t err_no;			/* errno */
} ELF_Signifo;

typedef int32_t ELF_Pid;

typedef struct {
	int64_t tv_sec;
	int64_t tv_usec;
} ELF_Timeval;
/// @endcond


/**
 * PR_STATUS note.  Transcribed from Xen.
 */
typedef struct __attribute__ ((aligned (16)))
{
/// @cond
	ELF_Signifo pr_info;		/* Info associated with signal */
	int16_t pr_cursig;		/* Current signal */
	uint64_t pr_sigpend;		/* Set of pending signals */
	uint64_t pr_sighold;		/* Set of held signals */
	ELF_Pid pr_pid;
	ELF_Pid pr_ppid;
	ELF_Pid pr_pgrp;
	ELF_Pid pr_sid;
	ELF_Timeval pr_utime;		/* User time */
	ELF_Timeval pr_stime;		/* System time */
	ELF_Timeval pr_cutime;		/* Cumulative user time */
	ELF_Timeval pr_cstime;		/* Cumulative system time */
	uint64_t pr_reg[27];		/* GP registers */
	int32_t pr_fpvalid;		/* True if math co-processor being used.  */
/// @endcond
} ELF_Prstatus;

/// @cond
#define PR_REG_r15		0
#define PR_REG_r14		1
#define PR_REG_r13		2
#define PR_REG_r12		3
#define PR_REG_rbp		4
#define PR_REG_rbx		5
#define PR_REG_r11		6
#define PR_REG_r10		7
#define PR_REG_r9		8
#define PR_REG_r8		9
#define PR_REG_rax		10
#define PR_REG_rcx		11
#define PR_REG_rdx		12
#define PR_REG_rsi		13
#define PR_REG_rdi		14
#define PR_REG_orig_rax		15
#define PR_REG_rip		16
#define PR_REG_cs		17
#define PR_REG_rflags		18
#define PR_REG_rsp		19
#define PR_REG_ss		20
#define PR_REG_thread_fs	21
#define PR_REG_thread_gs	22
#define PR_REG_ds		23
#define PR_REG_es		24
#define PR_REG_fs		25
#define PR_REG_gs		26
/// @endcond


/** x86 exception information
 */
struct x86_64exception
{
    /// @cond
    uint64_t rip;
    uint16_t cs, _pad1[3];
    uint64_t rflags;
    uint64_t rsp;
    uint16_t ss, _pad2[3];
    /// @endcond
};

/** Xen crash_xen_info structure
 */
typedef struct {
    /// @cond
    uint64_t xen_major_version;
    uint64_t xen_minor_version;
    maddr_t xen_extra_version;
    maddr_t xen_changeset;
    maddr_t xen_compiler;
    maddr_t xen_compile_date;
    maddr_t xen_compile_time;
    uint64_t tainted;
    maddr_t xen_phys_start;
    uint64_t dom0_pfn_to_mfn_frame_list_list;
    /// @endcond
} x86_64_crash_xen_info_t;


#endif

/*
 * Local variables:
 * mode: C++
 * c-file-style: "BSD"
 * c-basic-offset: 4
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 */
