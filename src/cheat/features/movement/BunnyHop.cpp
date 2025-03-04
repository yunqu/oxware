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

VarInteger movement_bhop_mode("movement_bhop_mode", "Bunnyhop mode - legit or rage", BHOPMODE_Legit, BHOPMODE_Legit, BHOPMODE_Rage);
VarInteger movement_bhop_repeat_ms("movement_bhop_repeat_ms", "Interval which determines when we can jump again after we jumped", 0, 0, 1000);
VarInteger movement_bhop_standup("movement_bhop_standup", "Enables stand up with selected distance", 0, 0, 20);
VarBoolean movement_bhop_jump_on_ladder("movement_bhop_jump_on_ladder", "Jump if also on a ladder", true);
VarBoolean movement_bhop_jump_in_water("movement_bhop_jump_in_water", "Jump if in water", true);
VarBoolean movement_bhop_break_jump_animation("movement_bhop_break_jump_animation", "Breaks the jump animation with ducking", false);
VarBoolean movement_bhop_notouch_ground_illusion("movement_bhop_notouch_ground_illusion", "", false);
VarBoolean movement_bhop_multijump("movement_bhop_multijump", "Jump with every click of the jump key", false);
VarBoolean movement_bhop_mode_noslowdown("movement_bhop_mode_noslowdown", "Enables bhop noslowdown", false);
VarInteger movement_bhop_mode_noslowdown_method("movement_bhop_mode_noslowdown_method", "Noslowdown method", BHOPNSDN_EngineSpeed, BHOPNSDN_ServerSpeed, BHOPNSDN_EngineSpeed);
VarInteger movement_bhop_mode_noslowdown_factor("movement_bhop_mode_noslowdown_factor", "How much to no-slowdown", 1, 1, 10);
VarInteger movement_bhop_legit_ground_dist_min("movement_bhop_legit_ground_dist_min", "Minimal ground distance where to start jumping", 10, 5, 50);
VarInteger movement_bhop_legit_ground_dist_max("movement_bhop_legit_ground_dist_max", "Maximal ground distance where to start jumping", 25, 5, 50);
VarInteger movement_bhop_legit_efficiency("movement_bhop_legit_efficiency", "Efficiency settings of the bunnyhop", 0, 0, 2);
VarInteger movement_bhop_legit_scroll_density("movement_bhop_legit_scroll_density", "How dense will the resulting scroll pattern be", 2, 1, 5);

void CMovementBunnyHop::update(float frametime)
{
	if (!CMovement::bunnyhop.is_active())
	{
		m_allow_multijump = true;
		return;
	}

	int bhop_mode = movement_bhop_mode.get_value();
	
	bool is_surfing = CLocalState::the().is_surfing();
	if (is_surfing)
	{
		return;
	}
	
	if (movement_bhop_jump_in_water.get_value() && CLocalState::the().get_waterlevel() >= 2)
	{
		if (timer_allows_jump())
		{
			CClientMovementPacket::the().set_button_bit(IN_JUMP, true);
			reset_jump_time();
		}
		return;
	}
	
	if (movement_bhop_jump_on_ladder.get_value() && CLocalState::the().is_on_ladder())
	{
		if (timer_allows_jump())
		{
			CClientMovementPacket::the().set_button_bit(IN_JUMP, true);
			reset_jump_time();
		}
		return;
	}

	if (bhop_mode == BHOPMODE_Legit)
	{
		// unlike rage, legit bhop provides a lot of funcionality and settings that you can tweak.
		legit_bhop(frametime);
	}
	else if (bhop_mode == BHOPMODE_Rage)
	{ 
		// rage bhop is far more obvious but on the other hand, perfect.
		rage_bhop(frametime);
	}

	standup();

	// TODO: animation breakings and illusion from HPP
	break_jump_animation();

	multijump();
}

void CMovementBunnyHop::rage_bhop(float frametime)
{
	if (!timer_allows_jump())
	{
		return;
	}

	// activates only the frame when on ground. This is extremely obvious when someone analyzes
	// your demo, but on the otherhand this creates almost no-slow-down effect.
	bool jump = perfect_condition_for_jump();

	if (jump)
	{
		reset_jump_time();
	}

	CClientMovementPacket::the().set_button_bit(IN_JUMP, jump);
}

void CMovementBunnyHop::legit_bhop(float frametime)
{
	float gnd_dist = CLocalState::the().get_ground_dist();
	bool is_onground = perfect_condition_for_jump();
	auto cl_enginefuncs = CMemoryHookMgr::the().cl_enginefuncs();

	determine_random_gnddist_for_jump_simul(is_onground);

	auto bhop_eff = (EBhopEfficiency)movement_bhop_legit_efficiency.get_value();

	if (bhop_eff == BHOPEFF_Helper && hop_helper(frametime))
	{
		return;
	}

	// jump if we satisfy following conditions:
	bool perform_jump = (is_onground || gnd_dist < m_simul_start_gnddist);

	// block the jump randomly unless we're really near the ground. this creates sort of a randomization
	// pattern for jumps.
	bool block_jump = false;
	if (randomize_jump_pattern(is_onground))
	{
		block_jump = true;
	}

	if (perform_jump && !block_jump)
	{
		if (bhop_eff == BHOPEFF_RandomFOGBased)
		{
			// randomize this as well
			static int cnt = 0;
			if (++cnt % cl_enginefuncs->pfnRandomLong(1, 10) == 0)
			{
				return;
			}
		}

		if (timer_allows_jump())
		{
			if (is_onground)
			{
				nsdn_speedhack();
			}

			simulate_jump(frametime);

			reset_jump_time();
		}
	}
	else
	{
		if (m_remained_in_jump)
		{
			if (!CClientMovementPacket::the().set_button_bit_atomic(IN_JUMP))
			{
				m_remained_in_jump = false;
			}
		}
	}
}

void CMovementBunnyHop::simulate_jump(float frametime)
{
	m_remained_in_jump = CClientMovementPacket::the().set_button_bit_atomic(IN_JUMP);
}

void CMovementBunnyHop::determine_random_gnddist_for_jump_simul(bool on_ground)
{
	auto cl_enginefuncs = CMemoryHookMgr::the().cl_enginefuncs();
	static bool was_on_ground = false;
	if (on_ground != was_on_ground)
	{
		int min = movement_bhop_legit_ground_dist_min.get_value();
		int max = movement_bhop_legit_ground_dist_max.get_value();

		// randomize the value each time we hit/leave ground
		if (on_ground)
		{
			m_simul_start_gnddist = (float)cl_enginefuncs->pfnRandomLong(min, max); // ground leaving distance
		}
		else
		{
			m_simul_start_gnddist = (float)cl_enginefuncs->pfnRandomLong(min, max); // ground entering distance
		}

		was_on_ground = on_ground;
	}
}

bool CMovementBunnyHop::hop_helper(float frametime)
{
	// try to predict if we'll be on ground next frame. this is a hard one.
	if (predicted_nextframe_on_ground(frametime))
	{
		// releasae the cmd button just in case it wasn't - therefore we can do the +jump at the exact frame
		// where we land on ground.
		CClientMovementPacket::the().set_button_bit(IN_JUMP, false);
		return true;
	}
	
	return false;
}

bool CMovementBunnyHop::perfect_condition_for_jump()
{
	return CLocalState::the().is_on_ground_safe();
}

bool CMovementBunnyHop::randomize_jump_pattern(bool is_onground)
{
	auto cl_enginefuncs = CMemoryHookMgr::the().cl_enginefuncs();
	static unsigned cnt = 0;
	int dens = movement_bhop_legit_scroll_density.get_value();
	if (dens == 5)
	{
		return false;
	}

	if (++cnt % cl_enginefuncs->pfnRandomLong(1, 5 - dens) != 0 && !is_onground)
	{
		return true;
	}

	return false;
}

bool CMovementBunnyHop::standup_permission(float height)
{
	// The code seems bad...

	static auto calc_fall_vel = [](const float height, const EPlayerHull& usehull)
		{
			const float frametime = CLocalState::the().get_engine_frametime();
			const auto pmove = *CMemoryHookMgr::the().pmove().get();

			float fallvel = pmove->flFallVelocity;

			if (fallvel <= 0.f)
				return fallvel;

			float heightground = usehull == HULL_DUCKING ? height : CLocalState::the().get_ground_dist();

			if (heightground == 0.f)
				return fallvel;

			const float gravity = pmove->movevars->gravity;

			if (gravity == 0.f)
				return fallvel;

			float finalfallvel = 0.f;

			while (heightground > 0.f)
			{
				finalfallvel = fallvel;
				fallvel += gravity * frametime;
				heightground -= fallvel * frametime;
			}

			return finalfallvel;
		};

	const auto origin = CLocalState::the().get_origin();
	const auto velocity = CLocalState::the().get_local_velocity_vec();
	const float height_ground_ducking = CGameUtil::the().compute_distance_to_ground(origin, 4096.0f, HULL_DUCKING);
	const float height_ducking = height_ground_ducking - 18.0f; // PM_VEC_DUCK_HULL_MIN, see pm_shared.h
	auto enginefuncs = CMemoryHookMgr::the().cl_enginefuncs();

	// If at standup you will have damage, skip it
	if (calc_fall_vel(height_ground_ducking, HULL_DUCKING) > PLAYER_MAX_SAFE_FALL_SPEED)
	{
		return false;
	}

	if (CLocalState::the().get_fall_velocity() <= 0.0f || height_ducking > height)
	{
		if (!velocity.IsZero2D())
		{
			// Cumpute predicted origin (without .z position)
			Vector prediction = origin + velocity * CLocalState::the().get_engine_frametime();
			prediction.z = origin.z;

			// Check the barrier
			const auto* trace = enginefuncs->pfnPM_TraceLine((float*)&origin, prediction, PM_TRACELINE_ANYVISIBLE, HULL_STANDING, -1);
			if (trace->endpos != prediction)
			{
				// Check possible to standup jump
				trace = enginefuncs->pfnPM_TraceLine((float*)&origin, prediction, PM_TRACELINE_ANYVISIBLE, HULL_DUCKING, -1);
				if (trace->endpos == prediction)
				{
					return true;
				}
			}
			else
			{
				// Fix fast bunny jumping without standup
				Vector end(prediction);
				end.z = -4096.f;
				trace = enginefuncs->pfnPM_TraceLine((float*)&prediction, end, PM_TRACELINE_ANYVISIBLE, HULL_STANDING, -1);
				const float prediction_height = prediction.z - trace->endpos.z;

				// Check difference of height ground
				if (prediction_height == CLocalState::the().get_ground_dist())
				{
					return false;
				}

				if (prediction_height < height)
				{
					return true;
				}
			}
		}

		return false;
	}

	return true;
}

bool CMovementBunnyHop::predicted_nextframe_on_ground(float frametime)
{
	float gnd_dist = CLocalState::the().get_ground_dist();
	float fall_vel = CLocalState::the().get_fall_velocity();
	bool is_onground = perfect_condition_for_jump();

	if (is_onground)
	{
		// already on ground
		return false;
	}

	if (gnd_dist == 0.0f || fall_vel == 0.0f)
	{
		return false;
	}

	// try to predict if we'll be on ground next frame. this is far from perfect, but should work most of the time.
	return fall_vel > 0.0f && ((fall_vel * frametime) * 2.0f > gnd_dist);
}

void CMovementBunnyHop::render_debug()
{
	CEngineFontRendering::the().render_debug("--- Bhop ---");

	CEngineFontRendering::the().render_debug("Perfect condition for jump: {}", perfect_condition_for_jump());
	CEngineFontRendering::the().render_debug("m_simul_start_gnddist: {}", m_simul_start_gnddist);
}

void CMovementBunnyHop::nsdn_speedhack()
{
	if (!movement_bhop_mode_noslowdown.get_value())
	{
		return;
	}

	auto method = (EBhopNoSlowDownMethod)movement_bhop_mode_noslowdown_method.get_value();
	int factor = movement_bhop_mode_noslowdown_factor.get_value();

	switch (method)
	{
		case BHOPNSDN_ServerSpeed:
		{
			int speed = std::max((10 - (factor - 1)), 0);
			CEngineSpeedControl::the().server_speedhack(speed);
			break;
		}
		case BHOPNSDN_EngineSpeed:
		{
			CEngineSpeedControl::the().classic_cs16_cheating_scene_speedhack(((double)(10 - (factor - 1)) / 4.0) / 10.0);
			break;
		}
	}
}

bool CMovementBunnyHop::timer_allows_jump()
{
	int repeat_ms = movement_bhop_repeat_ms.get_value();
	if (repeat_ms == 0)
	{
		return true;
	}

	if (GetTickCount() - m_jump_timer_ms <= repeat_ms)
	{
		return false;
	}

	return true;
}

void CMovementBunnyHop::reset_jump_time()
{
	int repeat_ms = movement_bhop_repeat_ms.get_value();
	if (repeat_ms == 0)
	{
		return;
	}

	m_jump_timer_ms = GetTickCount();
}

void CMovementBunnyHop::standup()
{
	const float units = (float)movement_bhop_standup.get_value();
	if (units == 0.0f)
	{
		return;
	}

	if (!standup_permission(units))
	{
		return;
	}

	CClientMovementPacket::the().set_button_bit(IN_DUCK, true);
}

void CMovementBunnyHop::break_jump_animation()
{
	if (!movement_bhop_break_jump_animation.get_value())
	{
		return;
	}

	const bool is_ducked = CClientMovementPacket::the().is_in_modified(IN_DUCK);
	const bool is_jumped = CClientMovementPacket::the().is_in_modified(IN_JUMP);

	if (CLocalState::the().get_fog_counter() > 0 && !is_ducked && is_jumped)
	{
		CClientMovementPacket::the().set_button_bit(IN_DUCK, true);
	}

	notouch_ground_illusion();
}

void CMovementBunnyHop::notouch_ground_illusion()
{
	static int nFramesInDuck;

	if (!movement_bhop_notouch_ground_illusion.get_value())
	{
		return;
	}

	if (CLocalState::the().get_fog_counter() == 0)
	{
		if (nFramesInDuck)
		{
			nFramesInDuck--;
			CClientMovementPacket::the().set_button_bit(IN_DUCK, true);
		}
	}
	else
	{
		// Minimum frames for change to duck state
		nFramesInDuck = 2;
	}
}

void CMovementBunnyHop::multijump()
{
	if (!m_allow_multijump)
	{
		return;
	}

	if (!movement_bhop_multijump.get_value())
	{
		return;
	}

	if (perfect_condition_for_jump())
	{
		return;
	}

	CClientMovementPacket::the().set_button_bit(IN_JUMP, true);
	m_allow_multijump = false;
}