#pragma once

class CConVar;

namespace CONVAR
{
	// dump convars to file
	bool Dump(const wchar_t* wszFileName);
	// setup convars
	bool Setup();

	inline CConVar* m_pitch = nullptr;
	inline CConVar* m_yaw = nullptr;
	inline CConVar* sensitivity = nullptr;

	inline CConVar* game_type = nullptr;
	inline CConVar* game_mode = nullptr;

	inline CConVar* mp_teammates_are_enemies = nullptr;
	inline CConVar* mp_damage_scale_ct_head = nullptr;
	inline CConVar* mp_damage_scale_t_head = nullptr;
	inline CConVar* mp_damage_scale_ct_body = nullptr;
	inline CConVar* mp_damage_scale_t_body = nullptr;

	inline CConVar* sv_autobunnyhopping = nullptr;
	inline CConVar* sv_airaccelerate = nullptr;
	inline CConVar* sv_quantize_movement_input = nullptr;
	inline CConVar* sv_maxunlag = nullptr;
	inline CConVar* sv_accelerate = nullptr;
	inline CConVar* sv_maxspeed = nullptr;
}
