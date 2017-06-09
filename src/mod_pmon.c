#include <stdio.h>
#include <string.h>
#include <mpi.h>
#include <stdbool.h>
#include <assert.h>

#include <unistd.h>
#include "ipm_modules.h"
#include "ipm_time.h"
#include "mod_pmon.h"
#include "report.h"


int    pmon_running = 0;


ipm_pmon_t pmondata[MAXNUM_REGIONS];

static void ipm_pmon_nullify(){
  pmon_running = 0;
  return;
}

static void parse_pm_counter(const char* file, double* val)
{
    FILE *F = fopen(file,"r");
    if (F)
    {
        fscanf(F, "%lf", val);
        fclose(F);
    }
    else
    {
        *val = 0.0;
    }
}
static double ipm_pmon_sample_energy_mem(){
  double energy;
  parse_pm_counter("/sys/cray/pm_counters/memory_energy", &energy);
  return energy;
}//PMON_sample_energy_node

static double ipm_pmon_sample_energy_cpu(){
  double energy;
  parse_pm_counter("/sys/cray/pm_counters/cpu_energy", &energy);
  return energy;
}//PMON_sample_energy_node

static double ipm_pmon_sample_energy_node(){
  double energy;
  parse_pm_counter("/sys/cray/pm_counters/energy", &energy);
  return energy;
}//PMON_sample_energy_node

static void ipm_pmon_resume(struct region* reg){
    pmon_running = 1;
    pmondata[reg->id].node_initial_energy = ipm_pmon_sample_energy_node();
    pmondata[reg->id].cpu_initial_energy = ipm_pmon_sample_energy_cpu();
    pmondata[reg->id].mem_initial_energy = ipm_pmon_sample_energy_mem();
    return;
}//PMON_Resume

static void ipm_pmon_pause(struct region* reg){

    pmon_running = 0;

    pmondata[reg->id].node_final_energy =  ipm_pmon_sample_energy_node();
    reg->energy = pmondata[reg->id].node_final_energy - pmondata[reg->id].node_initial_energy;

    pmondata[reg->id].cpu_final_energy =  ipm_pmon_sample_energy_cpu();
    reg->cpu_energy = pmondata[reg->id].cpu_final_energy - pmondata[reg->id].cpu_initial_energy;

    pmondata[reg->id].mem_final_energy =  ipm_pmon_sample_energy_mem();
    reg->mem_energy = pmondata[reg->id].mem_final_energy - pmondata[reg->id].mem_initial_energy;

    reg->other_energy = reg->energy - reg->cpu_energy - reg->mem_energy;
}//PMON_Pause

int mod_pmon_xml(ipm_mod_t* mod, void* ptr, struct region* reg)
{
    int res = 0;
    double hz = 0;
    double version = 0;
    // timestamp of last startup
    double startup = 0;
    parse_pm_counter("/sys/cray/pm_counters/raw_scan_hz", &hz);
    parse_pm_counter("/sys/cray/pm_counters/version", &version);
    parse_pm_counter("/sys/cray/pm_counters/startup", &startup);

    res+=ipm_printf(ptr, "<module name=\"%s\" scan_hz=\"%lf\" version=\"%lf\"\
 startup=\"%lf\">\n", "PMON", hz, version, startup);

    return res;
}


int mod_pmon_finalize(ipm_mod_t* mod, int flags)
{
  // for future use
  return 0;
}//PMON_finalize

int mod_pmon_region(ipm_mod_t* mod, int op, struct region* reg)
{
    if (reg)
    {
        switch(op)
        {
            case -1:
                ipm_pmon_pause(reg);
                break;

            case 1:
                ipm_pmon_resume(reg);
                break;
        }
    }
    return 0;
}

int mod_pmon_init(ipm_mod_t* mod, int flags)
{
  int i, comp, rv;

  mod->state    = STATE_IN_INIT;
  mod->init     = mod_pmon_init;
  mod->output   = 0;//mod_pmon_output;
  mod->xml      = mod_pmon_xml;
  mod->regfunc  = mod_pmon_region;
  mod->finalize = 0; //mod_pmon_finalize;
  mod->name     = "PMON";

  for( i=0; i<MAXNUM_REGIONS; i++ ) {
        pmondata[i].node_initial_energy = 0;
        pmondata[i].node_final_energy = 0;
        pmondata[i].cpu_initial_energy = 0;
        pmondata[i].cpu_final_energy = 0;
        pmondata[i].mem_initial_energy = 0;
        pmondata[i].mem_final_energy = 0;
  }

  ipm_pmon_nullify();
  mod->state    = STATE_ACTIVE;
  return IPM_OK;
}
