#include "lo/lo.h"


int main()
{
  lo_address t = lo_address_new(NULL, "7770");
  lo_send(t, "/foo/bar", "ssf", "create", "invaders", 1.0f);

return 0;
}
