
#include <stdio.h>

#include "write_html.h"
#include "ipm_parse.h"

#define PROTOVIS      "http://dl.dropbox.com/u/408013/protovis.js"
#define STYLESHEET    "http://dl.dropbox.com/u/408013/style.css"

char buf1[128];
char buf2[128];

void write_head(FILE*f, job_t* job);
void write_nav(FILE*f, job_t* job);
void write_banner(FILE*f, job_t* job, banner_t *b);
void write_mpiloadbalance(FILE *f, job_t *job) ;
void write_module_piechart(FILE *f, job_t *job, char* mname);


void write_html( FILE *f, job_t* job, banner_t *b )
{
  fprintf(f, "<html>\n"); 
  {
    write_head(f,job);

    fprintf(f, "<body>\n"); 
    {
      write_nav(f,job);

      fprintf(f, "<div id=\"spacecontainer\">\n");
      fprintf(f, "<div id=\"container\">\n");

      write_banner(f, job, b);
      
      write_mpiloadbalance(f, job);

      write_module_piechart(f, job, "MPI");
      //      write_module_piechart(f, job, "POSIXIO");
    } 
    fprintf(f, "</body>\n");
  } 
  fprintf(f, "</html>\n");
}
 

void write_head(FILE*f, job_t* job)
{
  fprintf(f, 
	  "<head>\n"
	  "<LINK href=\"%s\" rel=\"stylesheet\" type=\"text/css\">\n"
	  "<title>IPM Job Profile - %s</title>\n"
	  "<script type=\"text/javascript\" src=\"%s\"></script>\n"
	  "</head>\n",
	  STYLESHEET,
	  JOBNAME(job),
	  PROTOVIS );
}

void write_nav(FILE *f, job_t *job)
{
  fprintf(f, 
	  "<div class=\"floating-menu\">\n"
	  "<h3>Quick Links</h3>\n"
	  "<a href=\"#\">Load Balance</a>\n"
	  "<a href=\"#\">Communication Balance</a>\n"
	  "<a href=\"#\">Message Buffer Sizes</a>\n"
	  "<a href=\"#\">Communication Topology</a>\n"
	  "<a href=\"#\">Switch Traffic</a>\n"
	  "</div>\n");
}

void write_banner(FILE *f, job_t *job, banner_t *data)
{
  char tmpstr[128];

  fprintf(f, 
	  "<div id=\"banner\">IPM Profile for <strong>%s</strong></div>\n"       
	  "<div id=\"commandPath\"><div id=\"cmdBg\">command: </div>%s</div>\n",
	  JOBNAME(job), CMDPATH(job));
  
  fprintf(f, "<table width=\"660\">\n");

  fprintf(f, "<tr><td class=\"odd\">host</td><td>%s</td>"
	  "<td class=\"odd\">walltime [sec]</td><td>%.2f</td></tr>\n",
          data->hostname, data->app.wallt.dmax);

  sprintf(tmpstr, "%d on %d nodes",  data->ntasks, data->nhosts);
  fprintf(f, "<tr><td class=\"odd\">mpi_tasks</td><td>%s</td>"
	  "<td class=\"odd\">%%comm</td><td>%.2f</td></tr>\n",
          tmpstr, 100.0*data->app.mpi.dsum/data->app.wallt.dsum);
  
  if( data->flags&BANNER_HAVE_OMP ) {
    sprintf(tmpstr, "%d", data->nthreads);
    fprintf(f, "<tr><td class=\"odd\">omp_threads</td><td>%s</td>"
	    "<td class=\"odd\">%%omp</td><td>%.2f</td></tr>\n",
	    tmpstr, 100.0*data->app.omp.dsum/data->app.wallt.dsum);
  }
  
  if( data->flags&BANNER_HAVE_POSIXIO ) {
    sprintf(tmpstr, "");
    fprintf(f, "<tr><td class=\"odd\">files</td><td>%s</td>"
	    "<td class=\"odd\">%%i/o</td><td>%.2f</td></tr>\n",
	    tmpstr, 100.0*data->app.pio.dsum/data->app.wallt.dsum);
  }

  sprintf(tmpstr, "%d on %d nodes",  data->ntasks, data->nhosts);
  fprintf(f, "<tr><td class=\"odd\">mem [GB]</td><td>%.2f</td>"
	  "<td class=\"odd\">%%gflop/sec</td><td>%.2f</td></tr>\n",
          data->procmem.dsum, data->app.gflops.dsum);
  fprintf(f, "</table>\n");
  

  fprintf(f, 
	  "<table width=\"660\">\n"
	  "<tr><td class=\"odd\">start</td><td>%s</td><td class=\"odd\">stop</td><td>%s</td></tr>\n"
	  "</table>\n",
	  START(job), STOP(job));
  

}


char* print_time(char* buf, int len, struct timeval tv)
{							
  const struct tm *nowstruct;

  nowstruct = localtime( &(tv.tv_sec) );	
  strftime(buf, len, "%a %b %d %T %Y", nowstruct);
  return buf;
}


void write_mpiloadbalance(FILE *f, job_t *job) 
{
  int i, ntasks;
  region_t *reg;
  module_t *mod;
  double time, max;

  ntasks = job->ntasks;
  mod = job->modulemap["MPI"];
  reg = &(job->ipm_main);
  max = 0.0;

  fprintf(f, 
	  "<div id=\"fig\">\n"
	  "<center>Time in MPI  <div id=\"subtle\">with respect to</div> MPI Rank\n"
	  "<script type=\"text/javascript+protovis\">\n"
	  "\n");

  fprintf(f, "var data = [[\n");

  for( i=0; i<ntasks; i++ ) 
    {
      time = (job->taskdata[i]).funcdata[std::make_pair(reg->id, mod->id)].time;
      if( max<time ) max=time;
      fprintf(f, "{\"x\": %d, \"y\": %.2f}%s", 
	      i, time, (i+1)<ntasks?", ":"");
    }

  fprintf(f, "]];\n");

  fprintf(f, 
	  "/* Sizing and scales. */\n"
	  "var w = 600,\n"
	  "h = 300,\n"
	  "x = pv.Scale.linear(0, %d).range(0, w),\n"
	  "y = pv.Scale.linear(0, %.2f).range(0, h);\n"
	  "\n"
	  "/* The root panel. */\n"
	  "var vis = new pv.Panel()\n"
	  ".width(w)\n"
	  ".height(h)\n"
	  ".bottom(20)\n"
	  ".left(20)\n"
	  ".right(10)\n"
	  ".top(5);\n"
	  "\n"
	  "/* X-axis and ticks. */\n"
	  "vis.add(pv.Rule)\n"
	  ".data(x.ticks())\n"
	  ".visible(function(d) d)\n"
	  ".left(x)\n"
	  ".bottom(-5)\n"
	  ".height(5)\n"
	  ".anchor(\"bottom\").add(pv.Label)\n"
	  ".text(x.tickFormat);\n"
	  "\n"
	  "/* The stack layout. */\n"
	  "vis.add(pv.Layout.Stack)\n"
	  ".layers(data)\n"
	  ".x(function(d) x(d.x))\n"
	  ".y(function(d) y(d.y))\n"
	  ".layer.add(pv.Area);\n"
	  "\n"
	  "/* Y-axis and ticks. */\n"
	  "vis.add(pv.Rule)\n"
	  ".data(y.ticks(3))\n"
	  ".bottom(y)\n"
	  ".strokeStyle(function(d) d ? \"rgba(128,128,128,.2)\" : \"#000\")\n"
	  ".anchor(\"left\").add(pv.Label)\n"
	  ".text(y.tickFormat);\n"
	  "\n"
	  "vis.render();\n",
	  ntasks-1, max*1.05 );

  fprintf(f, "</script>\n");
}

void write_module_piechart(FILE *f, job_t *job, char* mname) 
{
  int i, ntasks;
  region_t *reg;
  module_t *mod;
  func_t *func;
  double time;
  double sum;
  
  ntasks = job->ntasks;
  mod = job->modulemap[mname];
  reg = &(job->ipm_main);

  std::map<std::string, double> funcsum;
  std::vector<std::pair<double, std::string> > funcvec;

  sum=0.0;
  // go through all functions and see if they belong to
  // the module we are interested in
  for( std::map<std::string, func_t*>::iterator fit=job->funcmap.begin();
       fit!=job->funcmap.end(); ++fit )
    {
      if( fit->second->parent!=mod )
	continue;

      for( i=0; i<ntasks; i++ ) 
	{
	  // for each task, sum time spent in each region
	  for( std::map<region_t *, regdata_t>::iterator rit=(job->taskdata[i]).regdata.begin();
	       rit!=(job->taskdata[i]).regdata.end(); ++rit) 
	    {
	      time = (job->taskdata[i]).funcdata[std::make_pair(rit->first->id, fit->second->id)].time;
	      funcsum[fit->first]+=time;
	      sum += time;
	    }
	}
    }

  

  fprintf(f, 
	  "<div id=\"fig\">\n"
	  "<center> Fraction of time spent in %s functions.\n"
	  "<script type=\"text/javascript+protovis\">\n"
	  "\n", mname);

  fprintf(f, "var data = [\n");
  for( std::map<std::string, double>::iterator it=funcsum.begin();
       it!=funcsum.end(); ++it ) 
    {
      fprintf(f, "%.2f, ", 100.0*funcsum[it->first]/sum);
    }
  fprintf(f, "];\n");


  fprintf(f, 
	  "\n"
	  "/* Sizing and scales. */\n"
	  "var w = 200,\n"
	  "h = 200,\n"
	  "r = w / 2,\n"
	  "a = pv.Scale.linear(0, pv.sum(data)).range(0, 2 * Math.PI);\n"
	  "\n\n"
	  "/* The root panel. */\n"
	  "var vis = new pv.Panel()\n"
	  ".width(w)\n"
	  ".height(h);\n"
	  "\n"
	  "/* The wedge, with centered label. */\n"
	  "vis.add(pv.Wedge)\n"
	  ".data(data.sort(pv.reverseOrder))\n"
	  ".bottom(w / 2)\n"
	  ".left(w / 2)\n"
	  ".innerRadius(r - 40)\n"
	  ".outerRadius(r)\n"
	  ".angle(a)\n"
	  ".event(\"mouseover\", function() this.innerRadius(0))\n"
	  ".event(\"mouseout\", function() this.innerRadius(r - 40))\n"
	  ".anchor(\"center\").add(pv.Label)\n"
	  ".visible(function(d) d > .15)\n"
	  ".textAngle(0)\n"
	  ".text(function(d) d.toFixed(2));\n"
	  "\n"
	  "vis.render();\n"
	  "</script>\n");

  
}
