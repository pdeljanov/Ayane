#pragma once

#include <cpplog/cpplog.hpp>

using namespace cpplog;

namespace Ayane
{

  class Logger
  {
    public:

      static StdErrLogger &logger()
      {
        static StdErrLogger log;
        return log;
      }

    private:
      Logger();
      Logger ( Logger const& );

      void operator= ( Logger const& );
  };

}

