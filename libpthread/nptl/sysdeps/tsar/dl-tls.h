/* Thread-local storage handling in the ELF dynamic linker */

/* Type used for the representation of TLS information in the GOT.  */
typedef struct
{
	unsigned long int ti_module;
	unsigned long int ti_offset;
} tls_index;

/* The thread pointer points 0x7000 past the first static TLS block.  */
#define TLS_TP_OFFSET 0x7000

/* Dynamic thread vector pointers point 0x8000 past the start of each
   TLS block.  */
#define TLS_DTV_OFFSET 0x8000

/* Compute the value for a GOTTPREL reloc.  */
#define TLS_TPREL_VALUE(sym_map, sym_val) \
	((sym_map)->l_tls_offset + sym_val - TLS_TP_OFFSET)

/* Compute the value for a DTPREL reloc.  */
#define TLS_DTPREL_VALUE(sym_val) \
	(sym_val - TLS_DTV_OFFSET)

extern void *__tls_get_addr (tls_index *ti);

#define GET_ADDR_OFFSET	(ti->ti_offset + TLS_DTV_OFFSET)
#define __TLS_GET_ADDR(__ti) (__tls_get_addr(__ti) - TLS_DTV_OFFSET)
