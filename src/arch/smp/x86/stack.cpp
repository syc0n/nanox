#include "smpprocessor.hpp"

using namespace nanos;

void SMPWD::initStackDep ( void *userfuction, void *cleanup )
{
   state = stack;
   state += stackSize;
   *state = (intptr_t)this; state--;
   *state = (intptr_t)getData(); state--;
   *state = (intptr_t)cleanup; state--;
   *state = (intptr_t)userfuction; state--;

   // skip first state
   state -= 3;
}
