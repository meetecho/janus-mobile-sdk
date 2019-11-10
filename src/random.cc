#include "janus/random.h"

#include <functional>
#include <algorithm>

namespace Janus {

  std::string RandomImpl::generate() {
    const char charset[] = "0123456789" "abcdefghijklmnopqrstuvwxyz" "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    const size_t size = sizeof(charset) - 1;

    auto randomChar = [&]() {
      return charset[ rand() % size ];
    };

    auto resultSize = 16;
    std::string result(resultSize, 0);
    std::generate_n(result.begin(), resultSize, randomChar);

    return result;
  }

}
