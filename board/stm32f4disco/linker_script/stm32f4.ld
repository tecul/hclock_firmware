OUTPUT_FORMAT("elf32-littlearm", "elf32-bigarm",
	      "elf32-littlearm")
OUTPUT_ARCH(arm)
ENTRY(_entry)

MEMORY
{
 flash (rx)         : ORIGIN = 0x08000000, LENGTH = 2M
 sram (rw)          : ORIGIN = 0x20000000, LENGTH = 192K
 ccm  (rw)          : ORIGIN = 0x10000000, LENGTH = 64K
}

_stack = ORIGIN(sram) + LENGTH(sram);
__stack = ORIGIN(sram) + LENGTH(sram);

SECTIONS
{
  .vectors          :
    {
        __boot_vector_start__ = .;
        KEEP(*(.vectors))
        . = 0x40;
        __boot_vector_end__ = .;
    } > flash = 0
  .text             :
    {
        *(.text)
    } > flash
  .rodata           :
    {
        *(.rodata)
        KEEP (*(SORT_NONE(.init)))
        KEEP (*(SORT_NONE(.fini)))
        KEEP (*(.eh_frame))
    } > flash
  __exidx_start = .;
  .ARM.exidx        :
    {
        *(.ARM.exidx*)
    } > flash
  __exidx_end = .;
  __data_load_start__ = .;
  .runtime_vectors :
    {
        *(.runtime_vectors)
    } > sram
  .data             :
    {
        __data_start__ = .;
        KEEP (*(SORT(.init_array.*)))
        KEEP (*(.init_array ))
        KEEP (*(SORT(.fini_array.*)))
        KEEP (*(.fini_array ))
        KEEP (*(.jcr))
        *(.data)
        __data_end__ = .;
    } > sram AT > flash
  __bss_start__ = .;
  .bss             :
    {
    } > sram
  __bss_end__ = . ;
  __end__ = . ;
  PROVIDE (end = .);
}

