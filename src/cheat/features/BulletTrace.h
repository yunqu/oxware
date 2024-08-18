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

#ifndef BULLETTRACE_H
#define BULLETTRACE_H
#pragma once

extern VarBoolean bullettrace_enable;
extern VarColor bullettrace_color;

extern VarBoolean bullettrace_enemy;
extern VarBoolean bullettrace_teammates;
extern VarFloat bullettrace_thickness;
extern VarFloat bullettrace_liveness;

struct bullet_trace_t
{
	Vector start_org;
	Vector end_org;
	CColor color;
};

class CBulletTrace
{
public:
	DECL_BASIC_CLASS(CBulletTrace);

public:
	void on_connected();
	void render();
	void on_bullet_fired(int entid, const Vector& forward, const Vector& right, const Vector& up, const Vector& source, 
						 const Vector& shooting_direction, const Vector& spread, float penetration_dist);

private:
	Vector calc_bullet_shoot_dir_vector(const Vector& original_shootdir, const Vector& spread, const Vector& right, const Vector& up);
	Vector calc_bullet_endpos(const Vector& source, const Vector& shootdir_with_spread, float penetration_dist);
	Vector calc_traced_bullet_endpos(const Vector& source, const Vector& end);

private:
	std::vector<bullet_trace_t> m_bullets_fired;
	int m_laser_index;
};

#endif // BULLETTRACE_H