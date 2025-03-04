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

InCommand CThirdPerson::thirdperson = InCommand("thirdperson", NULL, true, IN_ACTCOND_Alive);

VarInteger thirdperson_dist("thirdperson_dist", "Distance from the player when in 3rd person view", 0, 0, 1000);
VarFloat thirdperson_smooth("thirdperson_smooth", "Uses linear interpolation to smooth the movement", 1.0f, 0.0f, 2.0f);
VarBoolean thirdperson_block_wall("thirdperson_block_wall", "3rd person camera gets blocked by a solid object, doesn't clip through", true);

void CThirdPerson::update(hl::ref_params_t* pparams)
{
	float dist = (float)thirdperson_dist.get_value();
	float smooth = (float)thirdperson_smooth.get_value();
	smooth = smooth != 0 ? (10.0f / smooth) * pparams->frametime : 1.0f;

	auto local = CLocalState::the().local_player();
	if (!local)
	{
		return;
	}

	if (thirdperson.is_active())
	{
		// Start from clients view pos.
		Vector start_eye = local->get_eye_pos();

		auto event_api = CMemoryHookMgr::the().cl_enginefuncs()->pEventAPI;

		// Trace for solid wall.
		hl::pmtrace_t tr;
		event_api->EV_SetTraceHull(2);
		event_api->EV_PlayerTrace(
			start_eye,
			start_eye - pparams->forward * 4096.0f,
			PM_GLASS_IGNORE,
			-1,
			&tr);

		// Multiply the forward vector by the distance amount.
		Vector dir = pparams->forward * -dist;

		if (dist > 0.0f)
		{
			// Check for wall collision.
			if (dist > start_eye.Distance(tr.endpos) && thirdperson_block_wall.get_value())
			{
				m_offset.Lerp(m_offset, tr.endpos - start_eye + pparams->forward * 2.0f, smooth);
			}
			else
			{
				m_offset.Lerp(m_offset, dir, smooth);
			}
		}
		else
		{
			m_offset.Lerp(m_offset, (tr.endpos - start_eye) * 0.45f, smooth);
		}

		// if CL_IsThirdPerson() returns true, game fucks up our view angles. Restore.
		pparams->viewangles = CMemoryHookMgr::the().cl().get()->viewangles;
	}
	else
	{
		static Vector zero(0, 0, 0);
		m_offset.Lerp(m_offset, zero, smooth);
	}
	
	// Update the distance depending on the wall collision distance.
	pparams->vieworg += m_offset;
}
