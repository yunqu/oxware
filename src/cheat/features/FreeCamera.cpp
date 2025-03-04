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
*
*	File created by Dae-Moon
*/

#include "precompiled.h"

VarBoolean freecam_enable("freecam_enable", "Allows you to move freely around the world", false);
VarFloat freecam_speed("freecam_speed", "Adjusting the speed of movement", 2.5f, 0.5f, 5.0f);

void CFreeCamera::update(hl::ref_params_t* pparams)
{
	if (freecam_enable.get_value())
	{
		pparams->vieworg = m_origin;
		pparams->viewangles = CMemoryHookMgr::the().cl().get()->viewangles;
	}
	else
	{
		auto ent = CMemoryHookMgr::the().cl_enginefuncs()->pfnGetLocalPlayer();
		m_origin = ent->curstate.origin;
		m_last_viewangles = ent->curstate.angles;
	}
}

void CFreeCamera::move(hl::usercmd_t* cmd)
{
	if (!freecam_enable.get_value())
	{
		return;
	}

	const float frametime = CLocalState::the().get_engine_frametime();
	float speed = freecam_speed.get_value();

	Vector forward, right, up;
	CMath::the().angle_vectors(cmd->viewangles, &forward, &right, &up);

	forward *= cmd->forwardmove * speed * frametime;
	right *= cmd->sidemove * speed * frametime;
	up *= cmd->upmove * speed * frametime;

	if ((cmd->buttons & IN_FORWARD) || (cmd->buttons & IN_BACK)) // Forward & Back
	{
		m_origin += forward;
	}
	if ((cmd->buttons & IN_MOVERIGHT) || (cmd->buttons & IN_MOVELEFT)) // Right & Left
	{
		m_origin += right;
	}
	if ((cmd->buttons & IN_JUMP) || (cmd->buttons & IN_DUCK)) // Up & Down
	{
		m_origin += up;
	}

	cmd->buttons = 0;
	cmd->forwardmove = 0;
	cmd->sidemove = 0;
	cmd->upmove = 0;
	cmd->viewangles = m_last_viewangles;
}