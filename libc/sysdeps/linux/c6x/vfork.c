/*
 * Port of uClibc for TMS320C6000 DSP architecture
 * Copyright (C) 2005 Texas Instruments Incorporated
 * Author of TMS320C6000 port: Aurelien Jacquiot
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Library General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU Library General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 */
#include <features.h>
__asm__(" .text\n"
            "vfork: \n"
            " MVKL .S1 ___libc_vfork,A0\n"
            " MVKH .S1 ___libc_vfork,A0\n"
            " BNOP .S2X A0,5\n");
