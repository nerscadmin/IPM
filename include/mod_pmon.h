#ifndef  PMON_H
#define  PMON_H

#include "ipm_modules.h"

typedef struct ipm_pmon
{
    double node_initial_energy;
    double node_final_energy;
    
    double cpu_initial_energy;
    double cpu_final_energy;

    double mem_initial_energy;
    double mem_final_energy;
} ipm_pmon_t;

int mod_pmi_xml(ipm_mod_t* mod, void* ptr, struct region* reg);
int mod_pmon_finalize(ipm_mod_t* mod, int flags);
int mod_pmon_region(ipm_mod_t* mod, int op, struct region* reg);
int mod_pmon_init(ipm_mod_t* mod, int flags);
double ipm_pmon_get_region_energy(int id);


#endif //PMON_H
