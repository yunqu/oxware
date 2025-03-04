/*
*	OXWARE developed by oxiKKK
*	Copyright (c) 2023
* 
*	This program is licensed under the MIT license. By downloading, copying, 
*	installing or using this software you agree to this license.
*
*	License Agreement
*
*	Permission is hereby granted, free of charge, to any person obtaining a 
*	copy of this software and associated documentation files (the "Software"), 
*	to deal in the Software without restriction, including without limitation 
*	the rights to use, copy, modify, merge, publish, distribute, sublicense, 
*	and/or sell copies of the Software, and to permit persons to whom the 
*	Software is furnished to do so, subject to the following conditions:
*
*	The above copyright notice and this permission notice shall be included 
*	in all copies or substantial portions of the Software. 
*
*	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS 
*	OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
*	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL 
*	THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
*	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING 
*	FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS 
*	IN THE SOFTWARE.
*/

#include "precompiled.h"

VarBoolean custom_fov("custom_fov", "Forces custom field of view", false);
VarFloat custom_fov_value("custom_fov_value", "Scales original field of view value by this amount", 1.0f, 1.0f, 1.5f);
VarFloat custom_fov_based_speed("custom_fov_based_speed", "", 1.0f, 0.0f, 3.0f);

void CFieldOfViewChanger::scale_fov()
{
	if (!custom_fov.get_value())
	{
		return;
	}

	float *p_original_fov = CMemoryHookMgr::the().scr_fov_value().get();
	*p_original_fov *= custom_fov_value.get_value();

	const float scale = custom_fov_based_speed.get_value();

	if (scale)
	{
		const float velocity = CLocalState::the().get_local_velocity_2d();
		*p_original_fov += (velocity / *p_original_fov) * scale;
	}
}
