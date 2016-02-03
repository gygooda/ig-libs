#include "log.h"

static LibSys::Logger g_logger;

LibSys::Logger& get_logger()
{
    return g_logger;
}
