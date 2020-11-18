#pragma once

#include <streambuf>

namespace BiosSimpleMqttClient {

  namespace Logging {

    class NullBuffer : public std::streambuf {

      public:
        // Fake that buffer is emptied
        int overflow(int c) {
          return c;
        }

    };

  }

};
