#ifndef VERSION_H
#define VERSION_H

#pragma once

#include <string>

namespace warhawk
{
  struct Version
  {
    static const std::string GIT_HASH;
    static const std::string GIT_DATE;
  };
}

#endif // VERSION_H
