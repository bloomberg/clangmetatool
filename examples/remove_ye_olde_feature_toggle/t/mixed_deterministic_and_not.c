#include <ye_olde_feature_toggle.h>
int test_function() {
  if (ye_olde_feature_toggle_is_enabled(15) ||
      ye_olde_feature_toggle_is_enabled(3)) {
    return 1;
  } else {
    return 0;
  }
}
