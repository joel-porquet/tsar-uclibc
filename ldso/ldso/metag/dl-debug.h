/*
 * Meta ELF shared library loader support.
 *
 * Program to load an elf binary on a linux system, and run it.
 * References to symbols in sharable libraries can be resolved
 * by either an ELF sharable library or a linux style of shared
 * library.
 *
 * Copyright (C) 2013, Imagination Technologies Ltd.
 *
 * Licensed under LGPL v2.1 or later, see the file COPYING.LIB in this tarball.
 */

static const char *_dl_reltypes_tab[] = {
	[0]  "R_METAG_HIADDR16", "R_METAG_LOADDR16", "R_METAG_ADDR32",
	[3]  "R_METAG_NONE", "R_METAG_RELBRANCH", "R_METAG_GETSETOFF",
	[6]  "R_METAG_REG32OP1", "R_METAG_REG32OP2", "R_METAG_REG32OP3",
	[9]  "R_METAG_REG16OP1", "R_METAG_REG16OP2", "R_METAG_REG16OP3",
	[12] "R_METAG_REG32OP4", "R_METAG_HIOG", "R_METAG_LOOG",
	[30] "R_METAG_VTINHERIT", "R_METAG_VTENTRY",
	[32] "R_METAG_HI16_GOTOFF", "R_METAG_LO16_GOTOFF",
	[34] "R_METAG_GETSET_GOTOFF", "R_METAG_GETSET_GOT",
	[36] "R_METAG_HI16_GOTPC", "R_METAG_LO16_GOTPC",
	[38] "R_METAG_HI16_PLT", "R_METAG_LO16_PLT",
	[40] "R_METAG_RELBRANCH_PLT", "R_METAG_GOTOFF",
	[42] "R_METAG_PLT", "R_METAG_COPY", "R_METAG_JMP_SLOT",
};
