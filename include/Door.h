#pragma once

#if defined(MK2)
#include "McpwmDoor.h"
#define Door McpwnDoor
#else
#include "SimpleDoor.h"
#define Door SimpleDoor
#endif
