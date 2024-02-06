#include "dongshang/chrome/util/assert_utils.h"
#include <assert.h>


void AssertUtils::IsTrue(bool result, std::string message) {
  assert(result && message.c_str());
}

void AssertUtils::NotNull(void* obj, std::string message) {
  if (!message.empty()) {
    assert(obj && message.c_str());
  } else {
    assert(obj);
  }
}