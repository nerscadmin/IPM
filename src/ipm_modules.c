
#include "ipm_core.h"
#include "ipm_sizes.h"
#include "ipm_modules.h"


ipm_mod_t modules[MAXNUM_MODULES];

void ipm_module_init(struct ipm_module *mod)
{   
  mod->state=STATE_NOTINIT;
  mod->init=0;
  mod->output=0;
  mod->finalize=0;
  mod->xml=0;
  mod->regfunc=0;
  mod->name=0;
  mod->ct_offs=0;
  mod->ct_range=0;
}
