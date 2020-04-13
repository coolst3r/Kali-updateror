#include "legitbottab.h"
#include "ragebottab.h"

#include "../../SDK/definitions.h"
#include "../../settings.h"
#include "../../Utils/xorstring.h"
#include "../../ImGUI/imgui_internal.h"
#include "../atgui.h"

#pragma GCC diagnostic ignored "-Wformat-security"

static ItemDefinitionIndex currentWeapon = ItemDefinitionIndex::INVALID;
//static bool enabled = false;
static bool silent = false;
static bool friendly = false;
static bool closestBone = false;
static bool desiredBones[] = {true, true, true, true, true, true, true, // center mass
							  true, true, true, true, true, true, true, // left arm
							  true, true, true, true, true, true, true, // right arm
							  true, true, true, true, true, // left leg
							  true, true, true, true, true  // right leg
							 };
static Bone bone = BONE_HEAD;
static float RagebotautoAimValue = 360.0f;
static bool aimStepEnabled = false;
static bool rcsEnabled = false;
static bool rcsAlwaysOn = false;
static float rcsAmountX = 2.0f;
static float rcsAmountY = 2.0f;
static bool autoPistolEnabled = false;
static bool autoShootEnabled = false;
static bool autoScopeEnabled = false;
static bool ignoreJumpEnabled = false;
static bool ignoreEnemyJumpEnabled = false;
static bool HitChanceEnabled = false;
static float HitChange = 20.f;
static bool autoWallEnabled = false;
static float autoWallValue = 10.0f;
static bool autoSlow = false;
static bool predEnabled = false;
static bool scopeControlEnabled = false;

void UI::ReloadRageWeaponSettings()
{
	ItemDefinitionIndex index = ItemDefinitionIndex::INVALID;
	if (Settings::Ragebot::weapons.find(currentWeapon) != Settings::Ragebot::weapons.end())
		index = currentWeapon;

	silent = Settings::Ragebot::weapons.at(index).silent;
	friendly = Settings::Ragebot::weapons.at(index).friendly;
	closestBone = Settings::Ragebot::weapons.at(index).closestBone;
	bone = Settings::Ragebot::weapons.at(index).bone;
	RagebotautoAimValue = Settings::Ragebot::weapons.at(index).RagebotautoAimFov;
	aimStepEnabled = Settings::Ragebot::weapons.at(index).aimStepEnabled;
	rcsEnabled = Settings::Ragebot::weapons.at(index).rcsEnabled;
	rcsAlwaysOn = Settings::Ragebot::weapons.at(index).rcsAlwaysOn;
	rcsAmountX = Settings::Ragebot::weapons.at(index).rcsAmountX;
	rcsAmountY = Settings::Ragebot::weapons.at(index).rcsAmountY;
	autoPistolEnabled = Settings::Ragebot::weapons.at(index).autoPistolEnabled;
	autoShootEnabled = Settings::Ragebot::weapons.at(index).autoShootEnabled;
	autoScopeEnabled = Settings::Ragebot::weapons.at(index).autoScopeEnabled;
	ignoreJumpEnabled = Settings::Ragebot::weapons.at(index).ignoreJumpEnabled;
	ignoreEnemyJumpEnabled = Settings::Ragebot::weapons.at(index).ignoreEnemyJumpEnabled;
	HitChanceEnabled = Settings::Ragebot::weapons.at(index).HitChanceEnabled;
	HitChange = Settings::Ragebot::weapons.at(index).HitChance;
	autoWallEnabled = Settings::Ragebot::weapons.at(index).autoWallEnabled;
	autoWallValue = Settings::Ragebot::weapons.at(index).autoWallValue;
	autoSlow = Settings::Ragebot::weapons.at(index).autoSlow;
	predEnabled = Settings::Ragebot::weapons.at(index).predEnabled;
	scopeControlEnabled = Settings::Ragebot::weapons.at(index).scopeControlEnabled;

	for (int bone = BONE_PELVIS; bone <= BONE_RIGHT_SOLE; bone++)
		desiredBones[bone] = Settings::Ragebot::weapons.at(index).desiredBones[bone];
}

void UI::UpdateRageWeaponSettings()
{
	if (Settings::Ragebot::weapons.find(currentWeapon) == Settings::Ragebot::weapons.end() && Settings::Ragebot::enabled)
		Settings::Ragebot::weapons[currentWeapon] = RagebotWeapon_t();

	RagebotWeapon_t settings = {
			.silent = silent,
			.friendly = friendly,
			.closestBone = closestBone,
			.aimStepEnabled = aimStepEnabled,
			.rcsEnabled = rcsEnabled,
			.rcsAlwaysOn = rcsAlwaysOn,
			.HitChanceEnabled = HitChanceEnabled,
			.autoPistolEnabled = autoPistolEnabled,
			.autoShootEnabled = autoShootEnabled,
			.autoScopeEnabled = autoScopeEnabled,
			.ignoreJumpEnabled = ignoreJumpEnabled,
			.ignoreEnemyJumpEnabled = ignoreEnemyJumpEnabled,
			.autoWallEnabled = autoWallEnabled,
			.autoSlow = autoSlow,
			.predEnabled = predEnabled,
			.scopeControlEnabled = scopeControlEnabled,
			.bone = bone,
			.RagebotautoAimFov = RagebotautoAimValue,
			.rcsAmountX = rcsAmountX,
			.rcsAmountY = rcsAmountY,
			.autoWallValue = autoWallValue,
			.HitChance = HitChange,
	};



	for (int bone = BONE_PELVIS; bone <= BONE_RIGHT_SOLE; bone++)
		settings.desiredBones[bone] = desiredBones[bone];

	Settings::Ragebot::weapons.at(currentWeapon) = settings;

	if (Settings::Ragebot::weapons.at(currentWeapon) == Settings::Ragebot::weapons.at(ItemDefinitionIndex::INVALID) &&
		currentWeapon != ItemDefinitionIndex::INVALID)
	{
		Settings::Ragebot::weapons.erase(currentWeapon);
		UI::ReloadRageWeaponSettings();
	}
}

void Ragebot::RenderTab()
{
	static char filterWeapons[32];

	

	ImGui::Columns(3, nullptr, false);
	{
		ImGui::SetColumnOffset(1, 200);
		ImGui::PushItemWidth(-10);

		if (ImGui::Checkbox(XORSTR("Enabled"), &Settings::Ragebot::enabled))
		{	
			Settings::Legitbot::enabled = false;
			UI::UpdateRageWeaponSettings();
		}

		ImGui::InputText(XORSTR("##FILTERWEAPONS"), filterWeapons, IM_ARRAYSIZE(filterWeapons));
		ImGui::PopItemWidth();
		ImGui::ListBoxHeader(XORSTR("##GUNS"), ImVec2(-1, -1));
		for (auto it : ItemDefinitionIndexMap)
		{
			bool isDefault = (int) it.first < 0;
			if (!isDefault && !Util::Contains(Util::ToLower(std::string(filterWeapons)), Util::ToLower(Util::Items::GetItemDisplayName(it.first).c_str())))
				continue;

			if (Util::Items::IsKnife(it.first) || Util::Items::IsGlove(it.first) || Util::Items::IsUtility(it.first))
				continue;

			const bool item_selected = ((int) it.first == (int) currentWeapon);
			ImGui::PushID((int) it.first);
			std::string formattedName;
			char changeIndicator = ' ';
			bool isChanged = Settings::Ragebot::weapons.find(it.first) != Settings::Ragebot::weapons.end();
			if (!isDefault && isChanged)
				changeIndicator = '*';
			formattedName = changeIndicator + (isDefault ? Util::Items::GetItemDisplayName(it.first).c_str() : Util::Items::GetItemDisplayName(it.first));
			if (ImGui::Selectable(formattedName.c_str(), item_selected))
			{
				currentWeapon = it.first;
				UI::ReloadRageWeaponSettings();
			}
			ImGui::PopID();
		}
		ImGui::ListBoxFooter();
	}
	ImGui::NextColumn();
	{
		ImGui::SetColumnOffset(2, ImGui::GetWindowWidth() / 2 + 75);
		ImGui::BeginChild(XORSTR("COL1"), ImVec2(0, 0), true);
		{
			ImGui::Text(XORSTR("Ragebot Target"));
			ImGui::Separator();
			ImGui::Columns(1, nullptr, false);
			{
				ImGui::PushItemWidth(-1);

					if(ImGui::Button(XORSTR("Bones"), ImVec2(-1, 0)))
						ImGui::OpenPopup(XORSTR("optionBones"));
					ImGui::SetNextWindowSize(ImVec2((ImGui::GetWindowWidth()/1.25f),ImGui::GetWindowHeight()), ImGuiSetCond_Always);
					if( ImGui::BeginPopup(XORSTR("optionBones")) )
					{
						ImGui::PushItemWidth(-1);
						ImGui::Text(XORSTR("Center Mass"));
						if( ImGui::Checkbox(XORSTR("Head"), &desiredBones[BONE_HEAD]) )
							UI::UpdateRageWeaponSettings();
						if( ImGui::Checkbox(XORSTR("Neck"), &desiredBones[BONE_NECK]) )
							UI::UpdateRageWeaponSettings();
						if( ImGui::Checkbox(XORSTR("Upper Spine"), &desiredBones[BONE_UPPER_SPINAL_COLUMN]) )
							UI::UpdateRageWeaponSettings();
						if( ImGui::Checkbox(XORSTR("Middle Spine"), &desiredBones[BONE_MIDDLE_SPINAL_COLUMN]) )
							UI::UpdateRageWeaponSettings();
						if( ImGui::Checkbox(XORSTR("Lower Spine"), &desiredBones[BONE_LOWER_SPINAL_COLUMN]) )
							UI::UpdateRageWeaponSettings();
						if( ImGui::Checkbox(XORSTR("Pelvis"), &desiredBones[BONE_PELVIS]) )
							UI::UpdateRageWeaponSettings();
						if( ImGui::Checkbox(XORSTR("Hip"), &desiredBones[BONE_HIP]) )
							UI::UpdateRageWeaponSettings();
						ImGui::Separator();

						ImGui::Columns(2, nullptr, false);
						{
							ImGui::Text(XORSTR("Player's Right Arm"));
							if( ImGui::Checkbox(XORSTR("Collarbone"), &desiredBones[BONE_RIGHT_COLLARBONE]) )
								UI::UpdateRageWeaponSettings();
							if( ImGui::Checkbox(XORSTR("Shoulder"), &desiredBones[BONE_RIGHT_SHOULDER]) )
								UI::UpdateRageWeaponSettings();
							if( ImGui::Checkbox(XORSTR("Armpit"), &desiredBones[BONE_RIGHT_ARMPIT]) )
								UI::UpdateRageWeaponSettings();
							if( ImGui::Checkbox(XORSTR("Bicep"), &desiredBones[BONE_RIGHT_BICEP]) )
								UI::UpdateRageWeaponSettings();
							if( ImGui::Checkbox(XORSTR("Elbow"), &desiredBones[BONE_RIGHT_ELBOW]) )
								UI::UpdateRageWeaponSettings();
							if( ImGui::Checkbox(XORSTR("Forearm"), &desiredBones[BONE_RIGHT_FOREARM]) )
								UI::UpdateRageWeaponSettings();
							if( ImGui::Checkbox(XORSTR("Wrist"), &desiredBones[BONE_RIGHT_WRIST]) )
								UI::UpdateRageWeaponSettings();
							ImGui::Text(XORSTR("Player's Right Leg"));
							if( ImGui::Checkbox(XORSTR("Buttcheek"), &desiredBones[BONE_RIGHT_BUTTCHEEK]) )
								UI::UpdateRageWeaponSettings();
							if( ImGui::Checkbox(XORSTR("Thigh"), &desiredBones[BONE_RIGHT_THIGH]) )
								UI::UpdateRageWeaponSettings();
							if( ImGui::Checkbox(XORSTR("Knee"), &desiredBones[BONE_RIGHT_KNEE]) )
								UI::UpdateRageWeaponSettings();
							if( ImGui::Checkbox(XORSTR("Ankle"), &desiredBones[BONE_RIGHT_ANKLE]) )
								UI::UpdateRageWeaponSettings();
							if( ImGui::Checkbox(XORSTR("Sole"), &desiredBones[BONE_RIGHT_SOLE]) )
								UI::UpdateRageWeaponSettings();
						}
						ImGui::NextColumn();
						{   // these spaces are here in the strings because checkboxes can't have duplicate titles.
							ImGui::Text(XORSTR("Player's Left Arm"));
							if( ImGui::Checkbox(XORSTR("Collarbone "), &desiredBones[BONE_LEFT_COLLARBONE]) )
								UI::UpdateRageWeaponSettings();
							if( ImGui::Checkbox(XORSTR("Shoulder "), &desiredBones[BONE_LEFT_SHOULDER]) )
								UI::UpdateRageWeaponSettings();
							if( ImGui::Checkbox(XORSTR("Armpit "), &desiredBones[BONE_LEFT_ARMPIT]) )
								UI::UpdateRageWeaponSettings();
							if( ImGui::Checkbox(XORSTR("Bicep "), &desiredBones[BONE_LEFT_BICEP]) )
								UI::UpdateRageWeaponSettings();
							if( ImGui::Checkbox(XORSTR("Elbow "), &desiredBones[BONE_LEFT_ELBOW]) )
								UI::UpdateRageWeaponSettings();
							if( ImGui::Checkbox(XORSTR("Forearm "), &desiredBones[BONE_LEFT_FOREARM]) )
								UI::UpdateRageWeaponSettings();
							if( ImGui::Checkbox(XORSTR("Wrist "), &desiredBones[BONE_LEFT_WRIST]) )
								UI::UpdateRageWeaponSettings();

							ImGui::Text(XORSTR("Player's Left Leg"));
							if( ImGui::Checkbox(XORSTR("Buttcheek "), &desiredBones[BONE_LEFT_BUTTCHEEK]) )
								UI::UpdateRageWeaponSettings();
							if( ImGui::Checkbox(XORSTR("Thigh "), &desiredBones[BONE_LEFT_THIGH]) )
								UI::UpdateRageWeaponSettings();
							if( ImGui::Checkbox(XORSTR("Knee "), &desiredBones[BONE_LEFT_KNEE]) )
								UI::UpdateRageWeaponSettings();
							if( ImGui::Checkbox(XORSTR("Ankle "), &desiredBones[BONE_LEFT_ANKLE]) )
								UI::UpdateRageWeaponSettings();
							if( ImGui::Checkbox(XORSTR("Sole "), &desiredBones[BONE_LEFT_SOLE]) )
								UI::UpdateRageWeaponSettings();
						}
						ImGui::PopItemWidth();
						ImGui::EndPopup();
					}
				


				ImGui::PopItemWidth();
			}
			ImGui::Columns(1);
			ImGui::Separator();
			//Fov Settings Text
			ImGui::Text("FOV Settings");
			ImGui::Separator();

			//FOV settings values
			ImGui::Columns(1);
			{
				ImGui::PushItemWidth(-1);
					if (ImGui::SliderFloat(XORSTR("##FOV"), &RagebotautoAimValue, 0, 260))
					{
						UI::UpdateRageWeaponSettings();
					}		
				ImGui::PopItemWidth();		
			}
			ImGui::Columns(1);
			ImGui::Separator();
			ImGui::Text(XORSTR("Autoshoot"));
			ImGui::Separator();
			if (ImGui::Checkbox(XORSTR("Auto Shoot"), &autoShootEnabled))
				UI::UpdateRageWeaponSettings();
			ImGui::Checkbox(XORSTR("Velocity Check"), &Settings::Ragebot::AutoShoot::velocityCheck);
			if( ImGui::Checkbox(XORSTR("Hit Chance"), &HitChanceEnabled) )
				UI::UpdateRageWeaponSettings();
			if( ImGui::SliderFloat(XORSTR("##HITCHANCE"), &HitChange, 0.f, 100.f) )
				UI::UpdateRageWeaponSettings();
			ImGui::EndChild();
		}
	}
	ImGui::NextColumn();
	{
		ImGui::BeginChild(XORSTR("COL2"), ImVec2(0, 0), true);
		{
			ImGui::Columns(1);
			ImGui::Separator();
			ImGui::Text(XORSTR("Other"));
			ImGui::Separator();
			ImGui::Columns(2, nullptr, true);
			{

				switch (currentWeapon)
				{
					case ItemDefinitionIndex::INVALID:
					case ItemDefinitionIndex::WEAPON_DEAGLE:
					case ItemDefinitionIndex::WEAPON_ELITE:
					case ItemDefinitionIndex::WEAPON_FIVESEVEN:
					case ItemDefinitionIndex::WEAPON_GLOCK:
					case ItemDefinitionIndex::WEAPON_TEC9:
					case ItemDefinitionIndex::WEAPON_HKP2000:
					case ItemDefinitionIndex::WEAPON_USP_SILENCER:
					case ItemDefinitionIndex::WEAPON_P250:
					case ItemDefinitionIndex::WEAPON_CZ75A:
					case ItemDefinitionIndex::WEAPON_REVOLVER:
						if (ImGui::Checkbox(XORSTR("Auto Pistol"), &autoPistolEnabled))
							UI::UpdateRageWeaponSettings();
						break;
					default:
						break;
				}

				if (ImGui::Checkbox(XORSTR("Silent Aim"), &silent))
					UI::UpdateRageWeaponSettings();
				if (ImGui::Checkbox(XORSTR("Prediction"), &predEnabled))
					UI::UpdateRageWeaponSettings();
			}
			ImGui::NextColumn();
			{
				switch (currentWeapon)
				{
					case ItemDefinitionIndex::WEAPON_DEAGLE:
					case ItemDefinitionIndex::WEAPON_ELITE:
					case ItemDefinitionIndex::WEAPON_FIVESEVEN:
					case ItemDefinitionIndex::WEAPON_GLOCK:
					case ItemDefinitionIndex::WEAPON_TEC9:
					case ItemDefinitionIndex::WEAPON_HKP2000:
					case ItemDefinitionIndex::WEAPON_USP_SILENCER:
					case ItemDefinitionIndex::WEAPON_P250:
					case ItemDefinitionIndex::WEAPON_CZ75A:
					case ItemDefinitionIndex::WEAPON_REVOLVER:
						break;
					default:
						if (ImGui::Checkbox(XORSTR("Auto Scope"), &autoScopeEnabled))
							UI::UpdateRageWeaponSettings();
						if (ImGui::Checkbox(XORSTR("Scope Control"), &scopeControlEnabled))
							UI::UpdateRageWeaponSettings();
				}
			}


			ImGui::Columns(1);
			ImGui::Separator();
			ImGui::Text(XORSTR("AutoSlow"));
			ImGui::Separator();
			if (ImGui::Checkbox(XORSTR("Enabled##AUTOSLOW"), &autoSlow))
				UI::UpdateRageWeaponSettings();

			ImGui::Columns(1);
			ImGui::Separator();
			ImGui::Text(XORSTR("Damage Ammount"));
			ImGui::Separator();
			
			ImGui::Columns(1);
			{
				ImGui::PushItemWidth(-1);
				if (ImGui::SliderFloat(XORSTR("##AUTOWALLDMG"), &autoWallValue, 0, 100, XORSTR("Min Damage: %f")))
					UI::UpdateRageWeaponSettings();
				ImGui::PopItemWidth();
			}


			ImGui::Columns(1);
			ImGui::Separator();
			if (currentWeapon > ItemDefinitionIndex::INVALID && Settings::Ragebot::weapons.find(currentWeapon) != Settings::Ragebot::weapons.end())
			{
				if (ImGui::Button(XORSTR("Clear Weapon Settings"), ImVec2(-1, 0)))
				{
					Settings::Ragebot::weapons.erase(currentWeapon);
					UI::ReloadRageWeaponSettings();
				}
			}
			ImGui::EndChild();
		}
	}
}