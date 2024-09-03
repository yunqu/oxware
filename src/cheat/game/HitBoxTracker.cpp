#include "precompiled.h"

void CHitBoxTracker::update(hl::CStudioModelRenderer* pstudio_model_renderer)
{
	if (!pstudio_model_renderer)
	{
		return;
	}

	m_pstudio_model_renderer = pstudio_model_renderer;

	//get_entity_hitboxes();

	static hl::cvar_t* cl_minmodels;
	if (!cl_minmodels)
	{
		cl_minmodels = CGoldSrcCommandMgr::the().get_cvar("cl_minmodels");

		if (!cl_minmodels)
		{
			return;
		}
	}

	const float minmodels_backup = cl_minmodels->value;

	cl_minmodels->value = 0;
	get_player_hitboxes(false);

	cl_minmodels->value = 1;
	get_player_hitboxes(true);

	cl_minmodels->value = minmodels_backup;
}

std::optional<std::vector<StudioHitBox>*> CHitBoxTracker::get_local_hitboxes()
{
	int local_index = CMemoryHookMgr::the().cl().get()->playernum + 1;

	try
	{
		return std::make_optional(&m_hitboxes[local_index]);
	}
	catch (...)
	{
		CConsole::the().derror("Non-existent local player index: {}", local_index);
		return std::nullopt;
	}
}

std::optional<std::vector<StudioHitBox>*> CHitBoxTracker::get_hitboxes_by_id(int index)
{
	try
	{
		return std::make_optional(&m_hitboxes[index]);
	}
	catch (...)
	{
		CConsole::the().derror("Non-existent player index: {}", index);
		return std::nullopt;
	}
}

std::optional<StudioHitBox(*)[hl::NUM_HITBOXES]> CHitBoxTracker::get_local_min_hitboxes()
{
	int local_index = CMemoryHookMgr::the().cl().get()->playernum + 1;

	try
	{
		return std::make_optional(&m_min_hitboxes[local_index]);
	}
	catch (...)
	{
		CConsole::the().derror("Non-existent local player index: {}", local_index);
		return std::nullopt;
	}
}

std::optional<StudioHitBox(*)[hl::NUM_HITBOXES]> CHitBoxTracker::get_min_hitboxes_by_id(int index)
{
	try
	{
		return std::make_optional(&m_min_hitboxes[index]);
	}
	catch (...)
	{
		CConsole::the().derror("Non-existent player index: {}", index);
		return std::nullopt;
	}
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
	if (ent->index > 0 && ent->index <= cl->maxclients)
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

void CHitBoxTracker::get_player_hitboxes(const bool minmodels)
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

	if (minmodels)
	{
		if (pstudiohdr->numhitboxes > hl::NUM_HITBOXES)
		{
			return;
		}

		auto& hitboxes = m_min_hitboxes[ent->index];

		for (int i = 0; i < pstudiohdr->numhitboxes; i++)
		{
			auto& bbox = pbbox[i];
			auto matrix = (*m_pstudio_model_renderer->m_pbonetransform)[bbox.bone];

			auto& hitbox = hitboxes[i];
			update_hitbox(matrix, bbox, hitbox);
		}

		if (pstudiohdr->numhitboxes < hl::NUM_HITBOXES)
		{
			for (auto i = pstudiohdr->numhitboxes; i < hl::NUM_HITBOXES; i++)
			{
				auto& hitbox = hitboxes[i];

				hitbox.min.Clear();
				hitbox.max.Clear();
				hitbox.origin.Clear();

				for (auto j = 0; j < 8; j++)
				{
					hitbox.points[j].Clear();
				}
			}
		}
	}
	else
	{
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
}