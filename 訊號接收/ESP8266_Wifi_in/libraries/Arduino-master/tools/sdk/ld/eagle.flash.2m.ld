/* Flash Split for 2M chips */
/* sketch @0x40200000 (~1019KB) (1044464B) */
/* eeprom @0x403FB000 (=4KB) */
/* rfcal  @0x403FC000 (=4KB) */
/* wifi   @0x403FD000 (=12KB) */

MEMORY
{
  dport0_0_seg :                        org = 0x3FF00000, len = 0x10
  dram0_0_seg :                         org = 0x3FFE8000, len = 0x14000
  iram1_0_seg :                         org = 0x40100000, len = 0x8000
  irom0_0_seg :                         org = 0x40201010, len = 0xfeff0
}


INCLUDE "eagle.app.v6.common.ld"
