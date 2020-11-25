// Copyright (c) 2010-2020, Lawrence Livermore National Security, LLC. Produced
// at the Lawrence Livermore National Laboratory. All Rights reserved. See files
// LICENSE and NOTICE for details. LLNL-CODE-806117.
//
// This file is part of the MFEM library. For more information and source code
// availability visit https://mfem.org.
//
// MFEM is free software; you can redistribute it and/or modify it under the
// terms of the BSD-3 license. We welcome feedback and contributions, see file
// CONTRIBUTING.md for details.

#ifndef MFEM_CEEDSOLVERS_QCOARSEN_H
#define MFEM_CEEDSOLVERS_QCOARSEN_H

#include "../../config/config.hpp"

#ifdef MFEM_USE_CEED

#include <ceed.h>

// include linear.h here or in the .cpp?
#include "linear.h"

CEED_EXTERN int CeedOperatorQCoarsen(CeedOperator oper, int qorder_reduction,
                                     CeedOperator* out, CeedVector* coarse_assembledqf,
                                     struct LinearQFunctionContext** context_ptr);

#endif

#endif