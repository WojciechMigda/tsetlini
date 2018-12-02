#pragma once

#include <utility>
#include <string>

namespace Tsetlin
{

enum StatusCode
{
    S_OK = 0,

    S_BAD_JSON,
    S_BAD_LABELS,
};

using status_message_t = std::pair<StatusCode, std::string>;


}
