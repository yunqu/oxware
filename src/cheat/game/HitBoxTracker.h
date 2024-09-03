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

#ifndef HITBOXTRACKER_H
#define HITBOXTRACKER_H

struct StudioHitBox
{
	int group;

	Vector min;
	Vector max;
	Vector origin;
	Vector points[8];
};

class CHitBoxTracker
{
public:
	DECL_BASIC_CLASS(CHitBoxTracker);

public:
	void update(hl::CStudioModelRenderer* pstudio_model_renderer);

	std::optional<std::vector<StudioHitBox>*> get_local_hitboxes();
	std::optional<std::vector<StudioHitBox>*> get_hitboxes_by_id(int index);
	
	std::optional<StudioHitBox(*)[hl::NUM_HITBOXES]> get_local_min_hitboxes();
	std::optional<StudioHitBox(*)[hl::NUM_HITBOXES]> get_min_hitboxes_by_id(int index);

private:
	hl::CStudioModelRenderer* m_pstudio_model_renderer = nullptr;

	std::vector<StudioHitBox> m_hitboxes[MAX_EDICTS];
	StudioHitBox m_min_hitboxes[MAX_CLIENTS][hl::NUM_HITBOXES];

	hl::mstudiobbox_t* get_pbbox();

	void update_hitbox(const float(*matrix)[4], const hl::mstudiobbox_t& bbox, StudioHitBox& hitbox);

	void get_entity_hitboxes();
	void get_player_hitboxes(const bool minmodels);
};

#endif // !HITBOXTRACKER_H