# Makefile for uClibc
#
# Copyright (C) 2000 by Lineo, inc.
#
# This program is free software; you can redistribute it and/or modify it under
# the terms of the GNU Library General Public License as published by the Free
# Software Foundation; either version 2 of the License, or (at your option) any
# later version.
#
# This program is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE. See the GNU Library General Public License for more
# details.
#
# You should have received a copy of the GNU Library General Public License
# along with this program; if not, write to the Free Software Foundation, Inc.,
# 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
#
# Derived in part from the Linux-8086 C library, the GNU C Library, and several
# other sundry sources.  Files within this library are copyright by their
# respective copyright holders.

#--------------------------------------------------------
#
#There are a number of configurable options in Rules.mak
#
#--------------------------------------------------------

include Rules.mak

DIRS = misc pwd_grp stdio string termios unistd net signal stdlib sysdeps

all: libc.a

libc.a: halfclean headers subdirs
	@echo
	@echo Finally finished compiling...
	@echo
	$(CROSS)ranlib libc.a

halfclean:
	@rm -f libc.a

headers: dummy
	@rm -f include/asm include/net include/linux include/bits
	@ln -s $(KERNEL_SOURCE)/include/asm-$(ARCH_DIR) include/asm
	@if [ ! -f include/asm/unistd.h ] ; then \
	    echo "You didn't set KERNEL_SOURCE, TARGET_ARCH or HAS_MMU correctly in Config"; \
	    echo "The path '$(KERNEL_SOURCE)/include/asm-$(ARCH_DIR)' doesn't exist."; \
	    /bin/false; \
	fi;
	@ln -s $(KERNEL_SOURCE)/include/net include/net
	@ln -s $(KERNEL_SOURCE)/include/linux include/linux
	@ln -s ../sysdeps/linux/$(TARGET_ARCH)/bits include/bits

tags:
	ctags -R
	
clean: subdirs_clean
	rm -f libc.a
	rm -f include/asm include/net include/linux include/bits

subdirs: $(patsubst %, _dir_%, $(DIRS))
subdirs_clean: $(patsubst %, _dirclean_%, $(DIRS))

$(patsubst %, _dir_%, $(DIRS)) : dummy
	$(MAKE) -C $(patsubst _dir_%, %, $@)

$(patsubst %, _dirclean_%, $(DIRS)) : dummy
	$(MAKE) -C $(patsubst _dirclean_%, %, $@) clean

.PHONY: dummy

