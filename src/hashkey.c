#include <hashkey.h>




#ifdef UTEST_HASHKEY

#define TEST_ACTIVITY   0
#define TEST_REGION     1
#define TEST_TID        2
#define TEST_CALLSITE   3
#define TEST_DATATYPE   4
#define TEST_OPERATION  5
#define TEST_SELECT     6
#define TEST_RANK       7
#define TEST_BYTES      8


int test_setget(int what,
		unsigned long long val, 
		unsigned long long expect)
{
  IPM_KEY_TYPE key;
  unsigned long long u;
  
  KEY_CLEAR(key);
  switch( what )
    {
    case TEST_ACTIVITY:
      KEY_SET_ACTIVITY(key, val);
      u=KEY_GET_ACTIVITY(key);
      break;

    case TEST_REGION:
      KEY_SET_REGION(key, val);
      u=KEY_GET_REGION(key);
      break;

    case TEST_TID:
      KEY_SET_TID(key, val);
      u=KEY_GET_TID(key);
      break;

    case TEST_CALLSITE:
      KEY_SET_CALLSITE(key, val);
      u=KEY_GET_CALLSITE(key);
      break;

    case TEST_DATATYPE:
      KEY_SET_DATATYPE(key, val);
      u=KEY_GET_DATATYPE(key);
      break;

    case TEST_OPERATION:
      KEY_SET_OPERATION(key, val);
      u=KEY_GET_OPERATION(key);
      break;

    case TEST_SELECT:
      KEY_SET_SELECT(key, val);
      u=KEY_GET_SELECT(key);
      break;

    case TEST_RANK:
      KEY_SET_RANK(key, val);
      u=KEY_GET_RANK(key);
      break;

    case TEST_BYTES:
      KEY_SET_BYTES(key, val);
      u=KEY_GET_BYTES(key);
      break;
    }

  if( u!=expect )
    return 0;
  
  return 1;
}


int test_range(char* name, int what, unsigned long long min, 
	       unsigned long long max, unsigned long long wrap)
{
  int res;
  
  fprintf(stderr, "Testing set/get range for %12s ... ", name);
  res = test_setget(what, min, min);
  res = res && test_setget(what, max-1, max-1);
  res = res && test_setget(what, max, max);
  res = res && test_setget(what, max+1, wrap);
  fprintf(stderr, "%s\n", res?"PASS":"FAIL");
  
  return res;
}

int main(int argc, char* argv[] )
{
  int res;

  fprintf(stderr, "IPM_KEY_TYPE is %d bytes long\n", sizeof(IPM_KEY_TYPE));

  test_range("ACTIVITY", TEST_ACTIVITY, 0, KEY_MAX_ACTIVITY, 0);
  test_range("REGION",   TEST_REGION,   0, KEY_MAX_REGION,   0);
  test_range("TID",      TEST_TID,      0, KEY_MAX_TID,      0);
  test_range("CALLSITE", TEST_CALLSITE, 0, KEY_MAX_CALLSITE, 0);
  test_range("DATATYPE", TEST_DATATYPE,  0, KEY_MAX_DATATYPE, 0);
  test_range("OPERATION",TEST_OPERATION, 0, KEY_MAX_OPERATION, 0);
  test_range("SELECT",   TEST_SELECT,    0, KEY_MAX_SELECT, 0);
  test_range("RANK",     TEST_RANK,     0, KEY_MAX_RANK,     0);
  test_range("BYTES",    TEST_BYTES,    0, KEY_MAX_BYTES,    0);
}

#endif /* UTEST_HASHKEY */


