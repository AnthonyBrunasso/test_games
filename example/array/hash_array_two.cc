#include <cstdio>

#include "common\hash_array.cc"

struct Unit {
  HASH_STRUCT()
};

DECLARE_HASH_ARRAY(Unit, 10);

void
DebugPrint()
{
  printf("Unit - ");

  for (int i = 0; i < kMaxUnit; ++i) {
    printf("%i/%i/%i ", i, kUnit[i].id, kUnit[i].hash_idx);
  }

  printf("\nHash - ");

  for (int i = 0; i < kMaxHashUnit; ++i) {
    printf("%i/%i/%i ", i, kHashEntryUnit[i].id, kHashEntryUnit[i].array_idx);
  }

  printf("\n\n");
}
  
int
main()
{
  UseUnit();
  UseUnit(); 

  DebugPrint();

  return 0;
}
