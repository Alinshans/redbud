// ============================================================================
// Copyright (c) 2017 Alinshans. All rights reserved.
// Licensed under the MIT License. See LICENSE for details.
// 
// Header File : redbud/__undef_minmax.h 
//
// Cancels macro definition for min/max.
// ============================================================================

#ifdef min
#pragma message("macro min is incompatible with C++. #undefing min")
#undef min
#endif // min

#ifdef max
#pragma message("macro max is incompatible with C++. #undefing max")
#undef max
#endif // max
