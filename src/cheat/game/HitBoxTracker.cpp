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

void CHitBoxTracker::update(hl::CStudioModelRenderer* pstudio_model_renderer)
{
	if (!pstudio_model_renderer)
	{
		return;
	}

	m_pstudio_model_renderer = pstudio_model_renderer;

	if (!cl_minmodels)
	{
		cl_minmodels = CGoldSrcCommandMgr::the().get_cvar("cl_minmodels");

		if (!cl_minmodels)
		{
			return;
		}
	}
	
	//get_entity_hitboxes();
	get_player_hitboxes();
}

hl::mstudiobbox_t* CHitBoxTracker::get_pbbox()
{
	return (hl::mstudiobbox_t*)((byte*)m_pstudio_model_renderer->m_pStudioHeader + m_pstudio_model_renderer->m_pStudioHeader->hitboxindex);
}

void CHitBoxTracker::update_hitbox(const float(*matrix)[4], const hl::mstudiobbox_t& bbox, StudioHitBox& hitbox)
{
	Vector bbox_transformator;

	CMath::the().vector_transform(bbox.bbmin, matrix, hitbox.min);
	CMath::the().vector_transform(bbox.bbmax, matrix, hitbox.max);

	hitbox.origin = (hitbox.max + hitbox.min) * 0.5f;
	hitbox.group = bbox.group;

	for (int j = 0; j < 8; j++)
	{
		bbox_transformator[0] = (j & 1) ? bbox.bbmin[0] : bbox.bbmax[0];
		bbox_transformator[1] = (j & 2) ? bbox.bbmin[1] : bbox.bbmax[1];
		bbox_transformator[2] = (j & 4) ? bbox.bbmin[2] : bbox.bbmax[2];

		CMath::the().vector_transform(bbox_transformator, matrix, hitbox.points[j]);
	}
}

void CHitBoxTracker::get_entity_hitboxes()
{
	auto pstudiohdr = m_pstudio_model_renderer->m_pStudioHeader;
	if (!pstudiohdr || !pstudiohdr->numbodyparts)
	{
		return;
	}

	auto ent = m_pstudio_model_renderer->m_pCurrentEntity;
	if (!ent)
	{
		return;
	}

	auto cl = CMemoryHookMgr::the().cl().get();
	if (ent->index <= cl->maxclients)
	{
		return;
	}

	auto pbbox = get_pbbox();
	if (!pbbox)
	{
		return;
	}

	auto& hitboxes = m_hitboxes[ent->index];

	hitboxes.clear();
	for (int i = 0; i < pstudiohdr->numhitboxes; i++)
	{
		auto& bbox = pbbox[i];
		auto matrix = (*m_pstudio_model_renderer->m_pbonetransform)[bbox.bone];

		StudioHitBox hitbox;
		update_hitbox(matrix, bbox, hitbox);
		hitboxes.push_back(hitbox);
	}
}

void CHitBoxTracker::get_player_hitboxes()
{
	auto pstudiohdr = m_pstudio_model_renderer->m_pStudioHeader;
	if (!pstudiohdr || !pstudiohdr->numbodyparts)
	{
		return;
	}

	auto ent = m_pstudio_model_renderer->m_pCurrentEntity;
	if (!ent)
	{
		return;
	}

	auto cl = CMemoryHookMgr::the().cl().get();
	if (ent->index < 1 || ent->index > cl->maxclients)
	{
		return;
	}

	if (!m_pstudio_model_renderer->m_pPlayerInfo)
	{
		return; // https://github.com/FWGS/xash3d/blob/ab7c3848f85571a3f86817a2ced3fb87371ea560/engine/client/gl_studio.c#L3480
	}

	auto pbbox = get_pbbox();
	if (!pbbox)
	{
		return;
	}

	auto& hitboxes = m_hitboxes[ent->index];

	hitboxes.clear();
	for (int i = 0; i < pstudiohdr->numhitboxes; i++)
	{
		auto& bbox = pbbox[i];
		auto matrix = (*m_pstudio_model_renderer->m_pbonetransform)[bbox.bone];

		StudioHitBox hitbox;
		update_hitbox(matrix, bbox, hitbox);
		hitboxes.push_back(hitbox);
	}
}