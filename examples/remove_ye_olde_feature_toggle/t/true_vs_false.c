#include <ye_olde_feature_toggle.h>
int test_function() {
  if (ye_olde_feature_toggle_is_enabled(3) ||
      ye_olde_feature_toggle_is_enabled(5) ||
      ye_olde_feature_toggle_is_enabled(7)) {
    return 1;
  } else {
    return 0;
  }
}
