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

VarBoolean bullettrace_enable("bullettrace_enable", "Leaves trace of bullet marks", false);
VarColor bullettrace_color("bullettrace_color", "Color of trace", CColor(255, 255, 255, 255));

VarBoolean bullettrace_enemy("bullettrace_enemy", "Trace of enemy", false);
VarBoolean bullettrace_teammates("bullettrace_teammates", "Trace of teammates", false);
VarFloat bullettrace_thickness("bullettrace_thickness", "Thickness of trace", 0.1f, 0.1f, 3.0f);
VarFloat bullettrace_liveness("bullettrace_liveness", "Time of trace life", 1.0f, 0.5f, 5.0f);

void CBulletTrace::on_connected()
{
	m_laser_index = CMemoryHookMgr::the().cl_enginefuncs()->pEventAPI->EV_FindModelIndex("sprites/laserbeam.spr");
}

void CBulletTrace::render()
{
	if (m_laser_index == 0)
	{
		return;
	}

	for (const auto& bullet : m_bullets_fired)
	{
		CMemoryHookMgr::the().cl_enginefuncs()->pEfxAPI->R_BeamPoints(
			(float*)&bullet.start_org, (float*)&bullet.end_org, m_laser_index,
			bullettrace_liveness.get_value(), bullettrace_thickness.get_value(),
			0, bullet.color.a, 2, 0, 0, bullet.color.r, bullet.color.g, bullet.color.b);
	}

	m_bullets_fired.clear();
}

void CBulletTrace::on_bullet_fired(
	int entid, const Vector& forward, const Vector& right, const Vector& up, const Vector& source,
	const Vector& shooting_direction, const Vector& spread, float penetration_dist)
{
	if (!bullettrace_enable.get_value())
	{
		return;
	}

	auto player = CEntityMgr::the().get_local_player();
	if (!player || !(*player)->is_valid())
	{
		return;
	}

	const bool is_local = entid == (*player)->cl_entity()->index;

	if (!is_local)
	{
		const auto local_team = (*player)->get_team();
		player = CEntityMgr::the().get_player_by_id(entid);
		const auto player_team = (*player)->get_team();
		
		if ((!bullettrace_enemy.get_value() && local_team != player_team) ||
			(!bullettrace_teammates.get_value() && local_team == player_team))
		{
			return;
		}
	}

	Vector dir_with_spread = calc_bullet_shoot_dir_vector(shooting_direction, spread, right, up);

	Vector bullet_impact = calc_bullet_endpos(source, dir_with_spread, penetration_dist);
	Vector traced_bullet_impact = calc_traced_bullet_endpos(source, bullet_impact);

	CConsole::the().info("traced_bullet_impact: {}", traced_bullet_impact);

	m_bullets_fired.push_back({ source, traced_bullet_impact, is_local ? bullettrace_color.get_value() : (*player)->get_color_based_on_team() });
}

Vector CBulletTrace::calc_bullet_shoot_dir_vector(const Vector& original_shootdir, const Vector& spread, const Vector& right, const Vector& up)
{
	// apply spread to the final direction
	return original_shootdir + (spread.x * right + spread.y * up);
}

Vector CBulletTrace::calc_bullet_endpos(const Vector& source, const Vector& shootdir_with_spread, float penetration_dist)
{
	// calculate bullet impact position
	return source + shootdir_with_spread * penetration_dist;
}

Vector CBulletTrace::calc_traced_bullet_endpos(const Vector& source, const Vector& end)
{
	auto enginefuncs = CMemoryHookMgr::the().cl_enginefuncs();
	hl::pmtrace_t trace;

	enginefuncs->pEventAPI->EV_SetTraceHull(2);
	enginefuncs->pEventAPI->EV_PlayerTrace((float*)&source, (float*)&end, PM_GLASS_IGNORE, -1, &trace);

	return trace.endpos;
}
