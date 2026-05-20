#include <linux/init.h>
#include <linux/module.h>
#include <linux/version.h>

#include "trdm/printk.h"

#if 6 != LINUX_VERSION_MAJOR && 1 != LINUX_VERSION_PATCHLEVEL
  #error TRDM has only been tested on Linux version 6.1.0.
#endif

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Titan 0x546974616E");
MODULE_DESCRIPTION("A simple device mapper for educational purposes");

// The __init macro causes init functions to be freed once invoked for built-in
// drivers (in-tree build + built-in module), but not loadable modules (in-tree
// loadable module or out-of-tree build).
//
// For in-tree build module the kernel build system arranges all init functions
// in the same block of memory and when the kernel boots it frees that that one
// block all at once (at this moment a message like "Freeing unused kernel
// memory: 236k freed" is logged).
//
// Loadable modules, at the other end, cannot shared their code space between
// each other, therefore freeing indivual .init section is probably more trouble
// than it's worth because these sections are probably smaller than the page
// size.
//
// The __exit macro causes the omission of the function when the module is built
// into the kernel and like __init has no effect for loadable modules. Built-in
// drivers do not need a cleanup function because they cannot be unloaded.
//
// However, in our case, init functions call exit functions whenever an error
// occurs. Because these functions are stored in different sections (namely
// .init.text and .exit.text) they have a different lifetimes (functions may be
// released before use). Therefore, __init and __exit macros are not used (see
// modpost).
//
// https://sysprog21.github.io/lkmpg/#the-init-and-exit-macros
// https://stackoverflow.com/questions/11680641/init-and-exit-macros-usage-for-built-in-and-loadable-modules
// https://medium.com/@adityapatnaik27/linux-kernel-module-in-tree-vs-out-of-tree-build-77596fc35891
// https://stackoverflow.com/questions/8563978/what-is-kernel-section-mismatch

#define TRDM_SEPARATOR "================"

// Temporary log to have a better global view in dmesg.
#define TRDM_PRINT_INIT() TRDM_INFOLN(TRDM_SEPARATOR " Init " TRDM_SEPARATOR)
#define TRDM_PRINT_EXIT() TRDM_INFOLN(TRDM_SEPARATOR " Exit " TRDM_SEPARATOR)

static int trdm_init(void) {
  TRDM_PRINT_INIT();

  int error = 0;
  return TRDM_SUCCESS;

  TRDM_PRINT_EXIT();
  return error;
}

static void trdm_exit(void) {
  TRDM_PRINT_EXIT();
}

module_init(trdm_init);
module_exit(trdm_exit);
