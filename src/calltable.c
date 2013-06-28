
#include "ipm_sizes.h"
#include "calltable.h"


ipm_call_t ipm_calltable[MAXSIZE_CALLTABLE];

void init_calltable()
{
  int i;
  for( i=0; i<MAXSIZE_CALLTABLE; i++ ) {
    ipm_calltable[i].name=0;
    ipm_calltable[i].attr=0;
  }
}
