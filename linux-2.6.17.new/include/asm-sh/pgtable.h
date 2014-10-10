/*
 * This file contains the functions and defines necessary to modify and
 * use the SuperH page table tree.
 *
 * Copyright (C) 1999 Niibe Yutaka
 * Copyright (C) 2002 - 2005 Paul Mundt
 *
 * This file is subject to the terms and conditions of the GNU General
 * Public License.  See the file "COPYING" in the main directory of this
 * archive for more details.
 */
#ifndef __ASM_SH_PGTABLE_H
#define __ASM_SH_PGTABLE_H

#include <linux/config.h>
#include <asm-generic/pgtable-nopmd.h>
#include <asm/page.h>

#define PTRS_PER_PGD		1024

#ifndef __ASSEMBLY__
#include <asm/addrspace.h>
#include <asm/fixmap.h>

extern pgd_t swapper_pg_dir[PTRS_PER_PGD];
extern void paging_init(void);

/*
 * ZERO_PAGE is a global shared page that is always zero: used
 * for zero-mapped memory areas etc..
 */
extern unsigned long empty_zero_page[PAGE_SIZE / sizeof(unsigned long)];
#define ZERO_PAGE(vaddr) (virt_to_page(empty_zero_page))

#endif /* !__ASSEMBLY__ */

/* traditional two-level paging structure */
#define PGDIR_SHIFT	22
#define PTRS_PER_PMD	1
#define PTRS_PER_PTE	1024
#define PMD_SIZE	(1UL << PMD_SHIFT)
#define PMD_MASK	(~(PMD_SIZE-1))
#define PGDIR_SIZE	(1UL << PGDIR_SHIFT)
#define PGDIR_MASK	(~(PGDIR_SIZE-1))

#define USER_PTRS_PER_PGD	(TASK_SIZE/PGDIR_SIZE)
#define FIRST_USER_ADDRESS	0

#define PTE_PHYS_MASK	0x1ffff000

/*
 * First 1MB map is used by fixed purpose.
 * Currently only 4-enty (16kB) is used (see arch/sh/mm/cache.c)
 */
#define VMALLOC_START	(P3SEG+0x00100000)
#define VMALLOC_END	(FIXADDR_START-2*PAGE_SIZE)

/*
 * Linux PTEL encoding.
 *
 * Hardware and software bit definitions for the PTEL value:
 *
 * - Bits 0 and 7 are reserved on SH-3 (_PAGE_WT and _PAGE_SZ1 on SH-4).
 *
 * - Bit 1 is the SH-bit, but is unused on SH-3 due to an MMU bug (the
 *   hardware PTEL value can't have the SH-bit set when MMUCR.IX is set,
 *   which is the default in cpu-sh3/mmu_context.h:MMU_CONTROL_INIT).
 *
 *   In order to keep this relatively clean, do not use these for defining
 *   SH-3 specific flags until all of the other unused bits have been
 *   exhausted.
 *
 * - Bit 9 is reserved by everyone and used by _PAGE_PROTNONE.
 *
 * - Bits 10 and 11 are low bits of the PPN that are reserved on >= 4K pages.
 *   Bit 10 is used for _PAGE_ACCESSED, bit 11 remains unused.
 *
 * - Bits 31, 30, and 29 remain unused by everyone and can be used for future
 *   software flags, although care must be taken to update _PAGE_CLEAR_FLAGS.
 */
#define	_PAGE_WT	0x001		/* WT-bit on SH-4, 0 on SH-3 */
#define _PAGE_HW_SHARED	0x002		/* SH-bit  : shared among processes */
#define _PAGE_DIRTY	0x004		/* D-bit   : page changed */
#define _PAGE_CACHABLE	0x008		/* C-bit   : cachable */
#define _PAGE_SZ0	0x010		/* SZ0-bit : Size of page */
#define _PAGE_RW	0x020		/* PR0-bit : write access allowed */
#define _PAGE_USER	0x040		/* PR1-bit : user space access allowed */
#define _PAGE_SZ1	0x080		/* SZ1-bit : Size of page (on SH-4) */
#define _PAGE_PRESENT	0x100		/* V-bit   : page is valid */
#define _PAGE_PROTNONE	0x200		/* software: if not present  */
#define _PAGE_ACCESSED	0x400		/* software: page referenced */
#define _PAGE_FILE	_PAGE_WT	/* software: pagecache or swap? */

/* software: moves to PTEA.TC (Timing Control) */
#define _PAGE_PCC_AREA5	0x00000000	/* use BSC registers for area5 */
#define _PAGE_PCC_AREA6	0x80000000	/* use BSC registers for area6 */

/* software: moves to PTEA.SA[2:0] (Space Attributes) */
#define _PAGE_PCC_IODYN 0x00000001	/* IO space, dynamically sized bus */
#define _PAGE_PCC_IO8	0x20000000	/* IO space, 8 bit bus */
#define _PAGE_PCC_IO16	0x20000001	/* IO space, 16 bit bus */
#define _PAGE_PCC_COM8	0x40000000	/* Common Memory space, 8 bit bus */
#define _PAGE_PCC_COM16	0x40000001	/* Common Memory space, 16 bit bus */
#define _PAGE_PCC_ATR8	0x60000000	/* Attribute Memory space, 8 bit bus */
#define _PAGE_PCC_ATR16	0x60000001	/* Attribute Memory space, 6 bit bus */

/* Mask which drops unused bits from the PTEL value */
#ifdef CONFIG_CPU_SH3
#define _PAGE_CLEAR_FLAGS	(_PAGE_PROTNONE | _PAGE_ACCESSED| \
				 _PAGE_FILE	| _PAGE_SZ1	| \
				 _PAGE_HW_SHARED)
#else
#define _PAGE_CLEAR_FLAGS	(_PAGE_PROTNONE | _PAGE_ACCESSED | _PAGE_FILE)
#endif

#define _PAGE_FLAGS_HARDWARE_MASK	(0x1fffffff & ~(_PAGE_CLEAR_FLAGS))

/* Hardware flags: SZ0=1 (4k-byte) */
#define _PAGE_FLAGS_HARD	_PAGE_SZ0

#if defined(CONFIG_HUGETLB_PAGE_SIZE_64K)
#define _PAGE_SZHUGE	(_PAGE_SZ1)
#elif defined(CONFIG_HUGETLB_PAGE_SIZE_1MB)
#define _PAGE_SZHUGE	(_PAGE_SZ0 | _PAGE_SZ1)
#endif

#define _PAGE_TABLE	(_PAGE_PRESENT | _PAGE_RW | _PAGE_USER | _PAGE_ACCESSED | _PAGE_DIRTY)
#define _KERNPG_TABLE	(_PAGE_PRESENT | _PAGE_RW | _PAGE_ACCESSED | _PAGE_DIRTY)
#define _PAGE_CHG_MASK	(PTE_MASK | _PAGE_ACCESSED | _PAGE_CACHABLE | _PAGE_DIRTY)

#ifndef __ASSEMBLY__

#ifdef CONFIG_MMU
#define PAGE_NONE	__pgprot(_PAGE_PROTNONE | _PAGE_CACHABLE |_PAGE_ACCESSED | _PAGE_FLAGS_HARD)
#define PAGE_SHARED	__pgprot(_PAGE_PRESENT | _PAGE_RW | _PAGE_USER | _PAGE_CACHABLE |_PAGE_ACCESSED | _PAGE_FLAGS_HARD)
#define PAGE_COPY	__pgprot(_PAGE_PRESENT | _PAGE_USER | _PAGE_CACHABLE | _PAGE_ACCESSED | _PAGE_FLAGS_HARD)
#define PAGE_READONLY	__pgprot(_PAGE_PRESENT | _PAGE_USER | _PAGE_CACHABLE | _PAGE_ACCESSED | _PAGE_FLAGS_HARD)
#define PAGE_KERNEL	__pgprot(_PAGE_PRESENT | _PAGE_RW | _PAGE_CACHABLE | _PAGE_DIRTY | _PAGE_ACCESSED | _PAGE_HW_SHARED | _PAGE_FLAGS_HARD)
#define PAGE_KERNEL_NOCACHE \
			__pgprot(_PAGE_PRESENT | _PAGE_RW | _PAGE_DIRTY | _PAGE_ACCESSED | _PAGE_HW_SHARED | _PAGE_FLAGS_HARD)
#define PAGE_KERNEL_RO	__pgprot(_PAGE_PRESENT | _PAGE_CACHABLE | _PAGE_DIRTY | _PAGE_ACCESSED | _PAGE_HW_SHARED | _PAGE_FLAGS_HARD)
#define PAGE_KERNEL_PCC(slot, type) \
			__pgprot(_PAGE_PRESENT | _PAGE_RW | _PAGE_DIRTY | _PAGE_ACCESSED | _PAGE_FLAGS_HARD | (slot ? _PAGE_PCC_AREA5 : _PAGE_PCC_AREA6) | (type))
#else /* no mmu */
#define PAGE_NONE		__pgprot(0)
#define PAGE_SHARED		__pgprot(0)
#define PAGE_COPY		__pgprot(0)
#define PAGE_READONLY		__pgprot(0)
#define PAGE_KERNEL		__pgprot(0)
#define PAGE_KERNEL_NOCACHE	__pgprot(0)
#define PAGE_KERNEL_RO		__pgprot(0)
#define PAGE_KERNEL_PCC		__pgprot(0)
#endif

#endif /* __ASSEMBLY__ */

/*
 * As i386 and MIPS, SuperH can't do page protection for execute, and
 * considers that the same as a read.  Also, write permissions imply
 * read permissions. This is the closest we can get..
 */
#define __P000	PAGE_NONE
#define __P001	PAGE_READONLY
#define __P010	PAGE_COPY
#define __P011	PAGE_COPY
#define __P100	PAGE_READONLY
#define __P101	PAGE_READONLY
#define __P110	PAGE_COPY
#define __P111	PAGE_COPY

#define __S000	PAGE_NONE
#define __S001	PAGE_READONLY
#define __S010	PAGE_SHARED
#define __S011	PAGE_SHARED
#define __S100	PAGE_READONLY
#define __S101	PAGE_READONLY
#define __S110	PAGE_SHARED
#define __S111	PAGE_SHARED

#ifndef __ASSEMBLY__

/*
 * Certain architectures need to do special things when PTEs
 * within a page table are directly modified.  Thus, the following
 * hook is made available.
 */
#define set_pte(pteptr, pteval) (*(pteptr) = pteval)
#define set_pte_at(mm,addr,ptep,pteval) set_pte(ptep,pteval)

/*
 * (pmds are folded into pgds so this doesn't get actually called,
 * but the define is needed for a generic inline function.)
 */
#define set_pmd(pmdptr, pmdval) (*(pmdptr) = pmdval)

#define pte_pfn(x)		((unsigned long)(((x).pte >> PAGE_SHIFT)))
#define pfn_pte(pfn, prot)	__pte(((pfn) << PAGE_SHIFT) | pgprot_val(prot))
#define pfn_pmd(pfn, prot)	__pmd(((pfn) << PAGE_SHIFT) | pgprot_val(prot))

#define pte_none(x)	(!pte_val(x))
#define pte_present(x)	(pte_val(x) & (_PAGE_PRESENT | _PAGE_PROTNONE))
#define pte_clear(mm,addr,xp)	do { set_pte_at(mm, addr, xp, __pte(0)); } while (0)

#define pmd_none(x)	(!pmd_val(x))
#define pmd_present(x)	(pmd_val(x) & _PAGE_PRESENT)
#define pmd_clear(xp)	do { set_pmd(xp, __pmd(0)); } while (0)
#define	pmd_bad(x)	((pmd_val(x) & (~PAGE_MASK & ~_PAGE_USER)) != _KERNPG_TABLE)

#define pages_to_mb(x)	((x) >> (20-PAGE_SHIFT))
#define pte_page(x)	phys_to_page(pte_val(x)&PTE_PHYS_MASK)

/*
 * The following only work if pte_present() is true.
 * Undefined behaviour if not..
 */
static inline int pte_read(pte_t pte) { return pte_val(pte) & _PAGE_USER; }
static inline int pte_exec(pte_t pte) { return pte_val(pte) & _PAGE_USER; }
static inline int pte_dirty(pte_t pte){ return pte_val(pte) & _PAGE_DIRTY; }
static inline int pte_young(pte_t pte){ return pte_val(pte) & _PAGE_ACCESSED; }
static inline int pte_file(pte_t pte) { return pte_val(pte) & _PAGE_FILE; }
static inline int pte_write(pte_t pte){ return pte_val(pte) & _PAGE_RW; }
static inline int pte_not_present(pte_t pte){ return !(pte_val(pte) & _PAGE_PRESENT); }

static inline pte_t pte_rdprotect(pte_t pte)	{ set_pte(&pte, __pte(pte_val(pte) & ~_PAGE_USER)); return pte; }
static inline pte_t pte_exprotect(pte_t pte)	{ set_pte(&pte, __pte(pte_val(pte) & ~_PAGE_USER)); return pte; }
static inline pte_t pte_mkclean(pte_t pte)	{ set_pte(&pte, __pte(pte_val(pte) & ~_PAGE_DIRTY)); return pte; }
static inline pte_t pte_mkold(pte_t pte)	{ set_pte(&pte, __pte(pte_val(pte) & ~_PAGE_ACCESSED)); return pte; }
static inline pte_t pte_wrprotect(pte_t pte)	{ set_pte(&pte, __pte(pte_val(pte) & ~_PAGE_RW)); return pte; }
static inline pte_t pte_mkread(pte_t pte)	{ set_pte(&pte, __pte(pte_val(pte) | _PAGE_USER)); return pte; }
static inline pte_t pte_mkexec(pte_t pte)	{ set_pte(&pte, __pte(pte_val(pte) | _PAGE_USER)); return pte; }
static inline pte_t pte_mkdirty(pte_t pte)	{ set_pte(&pte, __pte(pte_val(pte) | _PAGE_DIRTY)); return pte; }
static inline pte_t pte_mkyoung(pte_t pte)	{ set_pte(&pte, __pte(pte_val(pte) | _PAGE_ACCESSED)); return pte; }
static inline pte_t pte_mkwrite(pte_t pte)	{ set_pte(&pte, __pte(pte_val(pte) | _PAGE_RW)); return pte; }
#ifdef CONFIG_HUGETLB_PAGE
static inline pte_t pte_mkhuge(pte_t pte)	{ set_pte(&pte, __pte(pte_val(pte) | _PAGE_SZHUGE)); return pte; }
#endif

/*
 * Macro and implementation to make a page protection as uncachable.
 */
#define pgprot_noncached pgprot_noncached

static inline pgprot_t pgprot_noncached(pgprot_t _prot)
{
	unsigned long prot = pgprot_val(_prot);

	prot &= ~_PAGE_CACHABLE;
	return __pgprot(prot);
}

#define pgprot_writecombine(prot) __pgprot(pgprot_val(prot) & ~_PAGE_CACHABLE)

/*
 * Conversion functions: convert a page and protection to a page entry,
 * and a page entry and page directory to the page they refer to.
 *
 * extern pte_t mk_pte(struct page *page, pgprot_t pgprot)
 */
#define mk_pte(page, pgprot)	pfn_pte(page_to_pfn(page), (pgprot))

static inline pte_t pte_modify(pte_t pte, pgprot_t newprot)
{ set_pte(&pte, __pte((pte_val(pte) & _PAGE_CHG_MASK) | pgprot_val(newprot))); return pte; }

#define pmd_page_kernel(pmd) \
((unsigned long) __va(pmd_val(pmd) & PAGE_MASK))

#define pmd_page(pmd) \
	(phys_to_page(pmd_val(pmd)))

/* to find an entry in a page-table-directory. */
#define pgd_index(address) (((address) >> PGDIR_SHIFT) & (PTRS_PER_PGD-1))
#define pgd_offset(mm, address) ((mm)->pgd+pgd_index(address))

/* to find an entry in a kernel page-table-directory */
#define pgd_offset_k(address) pgd_offset(&init_mm, address)

/* Find an entry in the third-level page table.. */
#define pte_index(address) \
		((address >> PAGE_SHIFT) & (PTRS_PER_PTE - 1))
#define pte_offset_kernel(dir, address) \
	((pte_t *) pmd_page_kernel(*(dir)) + pte_index(address))
#define pte_offset_map(dir, address) pte_offset_kernel(dir, address)
#define pte_offset_map_nested(dir, address) pte_offset_kernel(dir, address)
#define pte_unmap(pte)		do { } while (0)
#define pte_unmap_nested(pte)	do { } while (0)

#define pte_ERROR(e) \
	printk("%s:%d: bad pte %08lx.\n", __FILE__, __LINE__, pte_val(e))
#define pgd_ERROR(e) \
	printk("%s:%d: bad pgd %08lx.\n", __FILE__, __LINE__, pgd_val(e))

struct vm_area_struct;
extern void update_mmu_cache(struct vm_area_struct * vma,
			     unsigned long address, pte_t pte);

/* Encode and de-code a swap entry */
/*
 * NOTE: We should set ZEROs at the position of _PAGE_PRESENT
 *       and _PAGE_PROTNONE bits
 */
#define __swp_type(x)		((x).val & 0xff)
#define __swp_offset(x)		((x).val >> 10)
#define __swp_entry(type, offset) ((swp_entry_t) { (type) | ((offset) << 10) })
#define __pte_to_swp_entry(pte)	((swp_entry_t) { pte_val(pte) >> 1 })
#define __swp_entry_to_pte(x)	((pte_t) { (x).val << 1 })

/*
 * Encode and decode a nonlinear file mapping entry
 */
#define PTE_FILE_MAX_BITS	29
#define pte_to_pgoff(pte)	(pte_val(pte) >> 1)
#define pgoff_to_pte(off)	((pte_t) { ((off) << 1) | _PAGE_FILE })

typedef pte_t *pte_addr_t;

#define kern_addr_valid(addr)	(1)

#define io_remap_pfn_range(vma, vaddr, pfn, size, prot)		\
		remap_pfn_range(vma, vaddr, pfn, size, prot)

#define MK_IOSPACE_PFN(space, pfn)	(pfn)
#define GET_IOSPACE(pfn)		0
#define GET_PFN(pfn)			(pfn)

struct mm_struct;

/*
 * No page table caches to initialise
 */
#define pgtable_cache_init()	do { } while (0)

#ifndef CONFIG_MMU
extern unsigned int kobjsize(const void *objp);
#endif /* !CONFIG_MMU */

#if defined(CONFIG_CPU_SH4) || defined(CONFIG_SH7705_CACHE_32KB)
#define __HAVE_ARCH_PTEP_GET_AND_CLEAR
extern pte_t ptep_get_and_clear(struct mm_struct *mm, unsigned long addr, pte_t *ptep);
#endif

#include <asm-generic/pgtable.h>

#endif /* !__ASSEMBLY__ */

#endif /* __ASM_SH_PAGE_H */
