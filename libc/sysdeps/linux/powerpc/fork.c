/*
 * uC-libc/sysdeps/linux/powerpc/fork.c
 * fork() implementation for powerpc
 *
 * Copyright (C) 2001 by Lineo, Inc.
 * Author: David A. Schleef <ds@schleef.org>
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

#include <errno.h>
#include <features.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>


_syscall0(pid_t,fork);


