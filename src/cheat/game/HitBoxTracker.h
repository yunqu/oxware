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