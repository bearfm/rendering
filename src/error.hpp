#pragma once
#include <iostream>

#define THROW() \
    std::cerr << "ERROR AT LINE " << __LINE__ << " IN " << __FILE__ << std::endl; \
    std::exit(EXIT_FAILURE);