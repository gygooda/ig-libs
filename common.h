#pragma once

// disallow copy and assign constructors
#define DISALLOW_COPY_AND_ASSIGN(type_name) \
        type_name(const type_name&); \
    type_name& operator = (const type_name&)

