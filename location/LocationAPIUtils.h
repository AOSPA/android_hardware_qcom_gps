/*
 * Copyright (C) 2020 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
/*
Copyright (c) 2023 Qualcomm Innovation Center, Inc. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause-Clear
*/

#ifndef LOCATION_API_UTILS_H
#define LOCATION_API_UTILS_H

#include "LocationDataTypes.h"

/*===========================================================================
FUNCTION locUtilCreateCoarseLocation

DESCRIPTION
This function will fudge fine location to coarse

DEPENDENCIES
N/A

RETURN VALUE
N/A

SIDE EFFECTS
N/A
===========================================================================*/
void locUtilCreateCoarseLocation(const Location& in, Location& out);


#endif /* LOCATION_API_UTILS_H */
