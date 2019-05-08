#include <ye_olde_feature_toggle.h>
int test_function() {
  int feature_id = 3;
  if (ye_olde_feature_toggle_is_enabled(feature_id))
    return 1;
  feature_id = 5;
  if (ye_olde_feature_toggle_is_enabled(feature_id))
    return 0;
  return 42;
}
