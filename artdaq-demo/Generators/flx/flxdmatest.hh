#ifndef artdaq_flx_dma_test_Generators_HTG710FixedDMA_hh
#define artdaq_flx_dma_test_Generators_HTG710FixedDMA_hh

/*                                                                 */
/* This is the C++ source code of the flxdmatest application     */
/*                                                                 */
/* Author: Markus Joos, CERN                                       */
/*                                                                 */
/**C 2015 Ecosoft - Made from at least 80% recycled source code*****/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#include "/home/echurch/felix/software.r3222/drivers_rcc/DFDebug/DFDebug.h"
#include "/home/echurch/felix/software.r3222/drivers_rcc/cmem_rcc/cmem_rcc.h" // CMEM_* declarations
//#include "cmem_common.h" // EC, 20-Apr-2016
#include "/home/echurch/felix/software.r3222/flxcard/flxcard/FlxCard.h"
#include "/home/echurch/felix/software.r3222/flxcard/flxcard/FlxException.h"


#define APPLICATION_NAME    "flxdmatest"
#define APPLICATION_VERSION "1.0.0"
#define BUFSIZE (1024)
#define DMA_ID (0)



namespace flx {

class flxdmatest
{

public:
  flxdmatest() {};
  ~flxdmatest() {};
  FlxCard flxCard;

  // EC making get_data,vaddr public, the second of which is totally against the spirit of the original DMA'ing program.
  int get_data(int, char**);
  u_long vaddr;

private:

  int i, loop, max_tlp, ret, device_number = 0, opt, handle, debuglevel;
  u_long baraddr0, /*vaddr,*/ paddr, board_id, bsize, opt_emu_ena_to_host, opt_emu_ena_to_frontend, opt_dnlnk_fo, opt_uplnk_fo;

  flxcard_bar0_regs_t *bar0;


  void dump_buffer(u_long);
  void display_help();

};

}

#endif
