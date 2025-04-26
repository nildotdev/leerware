#pragma once

#include "config.h"

#pragma region variables_combo_entries
using VisualOverlayBox_t = int;

enum EVisualOverlayBox : VisualOverlayBox_t
{
	VISUAL_OVERLAY_BOX_NONE = 0,
	VISUAL_OVERLAY_BOX_FULL,
	VISUAL_OVERLAY_BOX_CORNERS,
	VISUAL_OVERLAY_BOX_MAX
};

using VisualChamMaterial_t = int;
enum EVisualsChamMaterials : VisualChamMaterial_t
{
	VISUAL_MATERIAL_PRIMARY_WHITE = 0,
	VISUAL_MATERIAL_ILLUMINATE,
	VISUAL_MATERIAL_MAX
};

using MiscDpiScale_t = int;

enum EMiscDpiScale : MiscDpiScale_t
{
	MISC_DPISCALE_DEFAULT = 0,
	MISC_DPISCALE_125,
	MISC_DPISCALE_150,
	MISC_DPISCALE_175,
	MISC_DPISCALE_200,
	MISC_DPISCALE_MAX
};

#pragma endregion

#pragma region variables_multicombo_entries
using MenuAddition_t = unsigned int;
enum EMenuAddition : MenuAddition_t
{
	MENU_ADDITION_NONE = 0U,
	MENU_ADDITION_GLOW = 1 << 0,
	MENU_ADDITION_ALL = MENU_ADDITION_GLOW
};

using RemovalsDropdown_t = unsigned int;

enum ERemovalsDropdown : RemovalsDropdown_t
{
	REMOVALS_NONE = 0U,
	REMOVALS_SMOKE = 1 << 0,
	REMOVALS_FLASH = 1 << 1,
	REMOVALS_LEGS = 1 << 2,
};

#pragma endregion

struct Variables_t
{
#pragma region variables_ragebot
	C_ADD_VARIABLE(bool, bRageEnable, false);
	C_ADD_VARIABLE(bool, bHideshots, false);
	C_ADD_VARIABLE(bool, bDoubletap, false);
	C_ADD_VARIABLE(KeyBind_t, kMinDamageOverride, KeyBind_t(CS_XOR("MD Override"), 0U, EKeyBindMode::TOGGLE));
	C_ADD_VARIABLE(int, nBacktrack, 0);
	C_ADD_VARIABLE(RageBotVars_t, varsGlobal, RageBotVars_t(true));
	C_ADD_VARIABLE(RageBotVars_t, varsScout, RageBotVars_t(false));
	C_ADD_VARIABLE(RageBotVars_t, varsAuto, RageBotVars_t(false));
	C_ADD_VARIABLE(RageBotVars_t, varsPistols, RageBotVars_t(false));
	C_ADD_VARIABLE(RageBotVars_t, varsAWP, RageBotVars_t(false));
	C_ADD_VARIABLE(RageBotVars_t, varsDeagle, RageBotVars_t(false));
	C_ADD_VARIABLE(RageBotVars_t, varsR8, RageBotVars_t(false));
#pragma endregion
#pragma region variables_antiaim
	C_ADD_VARIABLE(bool, bAntiAimEnable, false);
	C_ADD_VARIABLE(unsigned int, nOverrideLeft, 0U);
	C_ADD_VARIABLE(unsigned int, nOverrideRight, 0U);
#pragma endregion

#pragma region variables_visuals
	C_ADD_VARIABLE(bool, bVisualOverlay, false);

	C_ADD_VARIABLE(bool, bDrawSkeleton, false);
	C_ADD_VARIABLE(bool, bDrawBacktrack, false);
	C_ADD_VARIABLE(FrameOverlayVar_t, overlayBox, FrameOverlayVar_t(false));
	C_ADD_VARIABLE(TextOverlayVar_t, overlayName, TextOverlayVar_t(false));
	C_ADD_VARIABLE(BarOverlayVar_t, overlayHealthBar, BarOverlayVar_t(false, false, false, 1.f, Color_t(0, 255, 0), Color_t(255, 0, 0)));
	C_ADD_VARIABLE(BarOverlayVar_t, overlayArmorBar, BarOverlayVar_t(false, false, false, 1.f, Color_t(0, 255, 255), Color_t(255, 0, 0)));

	C_ADD_VARIABLE(bool, bVisualChams, false);
	C_ADD_VARIABLE(int, nVisualChamMaterial, VISUAL_MATERIAL_PRIMARY_WHITE);
	C_ADD_VARIABLE(bool, bVisualChamsIgnoreZ, false); // invisible chams
	C_ADD_VARIABLE(Color_t, colVisualChams, Color_t(0, 255, 0));
	C_ADD_VARIABLE(Color_t, colVisualChamsIgnoreZ, Color_t(255, 0, 0));

	C_ADD_VARIABLE(bool, bThirdPerson, false);
	C_ADD_VARIABLE(int, nThirdPersonDistance, 50.f);

	C_ADD_VARIABLE(bool, bWorldModulation, false);
	C_ADD_VARIABLE(ColorPickerVar_t, colWorld, ColorPickerVar_t(255, 255, 255));
	C_ADD_VARIABLE(ColorPickerVar_t, colMisc, ColorPickerVar_t(255, 255, 255));
	C_ADD_VARIABLE(ColorPickerVar_t, colProps, ColorPickerVar_t(255, 255, 255));
	C_ADD_VARIABLE(ColorPickerVar_t, colParticles, ColorPickerVar_t(255, 255, 255));

	C_ADD_VARIABLE(bool, bRemoveScope, false);
	C_ADD_VARIABLE(unsigned int, nRemovals, ERemovalsDropdown::REMOVALS_NONE);
	C_ADD_VARIABLE(int, nFOV, 90);
	C_ADD_VARIABLE(float, flAspectRatio, 0.f);
#pragma endregion

#pragma region variables_misc
	C_ADD_VARIABLE(bool, bAntiUntrusted, true);
	C_ADD_VARIABLE(bool, bWatermark, true);

	C_ADD_VARIABLE(bool, bHalfDuck, false);
	C_ADD_VARIABLE(bool, bSlowWalk, false);
	C_ADD_VARIABLE(bool, bAutoBHop, false);
	C_ADD_VARIABLE(bool, bAutoStrafe, false);
	C_ADD_VARIABLE(bool, bJumpBug, false);
	C_ADD_VARIABLE(bool, bEdgeJump, false);
	C_ADD_VARIABLE(bool, bQuickStop, false);
#pragma endregion

#pragma region variables_menu
	C_ADD_VARIABLE(unsigned int, nMenuKey, VK_INSERT);
	C_ADD_VARIABLE(unsigned int, nPanicKey, VK_END);
	C_ADD_VARIABLE(int, nDpiScale, MISC_DPISCALE_150);

	/*
	 * color navigation:
	 * [definition N][purpose]
	 * 1. primitive:
	 * - primtv 0 (text)
	 * - primtv 1 (background)
	 * - primtv 2 (disabled)
	 * - primtv 3 (control bg)
	 * - primtv 4 (border)
	 * - primtv 5 (hover)
	 *
	 * 2. accents:
	 * - accent 0 (main)
	 * - accent 1 (dark)
	 * - accent 2 (darker)
	 */
	C_ADD_VARIABLE(unsigned int, nMenuAdditional, EMenuAddition::MENU_ADDITION_ALL);
	C_ADD_VARIABLE(float, flAnimationSpeed, 1.f);


	C_ADD_VARIABLE(ColorPickerVar_t, colPrimtv0, ColorPickerVar_t(255, 255, 255)); // (text)
	C_ADD_VARIABLE(ColorPickerVar_t, colPrimtv1, ColorPickerVar_t(0, 0, 0, 0)); // (background)
	C_ADD_VARIABLE(ColorPickerVar_t, colPrimtv5, ColorPickerVar_t(0, 0, 0, 130)); // (pop-up background)
	C_ADD_VARIABLE(ColorPickerVar_t, colPrimtv2, ColorPickerVar_t(255, 255, 255)); // (disabled)
	C_ADD_VARIABLE(ColorPickerVar_t, colPrimtv3, ColorPickerVar_t(0, 0, 0, 150)); // (control bg)
	C_ADD_VARIABLE(ColorPickerVar_t, colPrimtv4, ColorPickerVar_t(0, 0, 0, 120)); // (border)

	C_ADD_VARIABLE(ColorPickerVar_t, colAccent0, ColorPickerVar_t(255, 50, 160)); // (main)
	C_ADD_VARIABLE(ColorPickerVar_t, colAccent1, ColorPickerVar_t(255, 105, 175)); // (dark)
	C_ADD_VARIABLE(ColorPickerVar_t, colAccent2, ColorPickerVar_t(255, 156, 190)); // (darker)
#pragma endregion
#pragma region variables_legitbot
	C_ADD_VARIABLE(bool, bLegitbot, false);
	C_ADD_VARIABLE(float, flAimRange, 10.0f);
	C_ADD_VARIABLE(float, flSmoothing, 10.0f);
	C_ADD_VARIABLE(bool, bLegitAutoWall, false);
	C_ADD_VARIABLE(bool, bLegitbotAlwaysOn, false);
	C_ADD_VARIABLE(unsigned int, nLegitbotActivationKey, VK_HOME);
#pragma endregion
};

inline Variables_t Vars = {};
