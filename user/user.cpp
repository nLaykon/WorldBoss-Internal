#include "pch-il2cpp.h"
#include "user.h"


#include "../framework/gamehook.h"

namespace User
{
	static float espL_Red = 1.f;
	static float espL_Green = 0.f;
	static float espL_Blue = 0.f;
	static float espL_Alpha = 1.f;


	static float espB_Red = 1.f;
	static float espB_Green = 0.f;
	static float espB_Blue = 0.f;
	static float espB_Alpha = 1.f;

	// Runs every frame before ImGui Render - Do all your drawing here
	void Tick()
	{
		Store::players = Unity::GameObject::FindWithTag("Player");

		Store::PlayerListUpdate();

		Watermark();
		DrawESP();
		DrawAim();

		GameHook::ToggleHooks();
	}

	void Menu()
	{
		if (Store::m_show)
		{
			ImGui::SetNextWindowSize(ImVec2(300.000f, 400.000f), ImGuiCond_Once);
			ImGui::Begin("NoEyes", NULL, 7);
			static int tabb = 0;
			{ImGui::SameLine();
			if (ImGui::Button("Visuals"))
			{
				tabb = 0;
			}
			ImGui::SameLine();

			if (ImGui::Button("Aimbot"))
			{
				tabb = 1;
			}
			ImGui::SameLine();
			if (ImGui::Button("Misc"))
			{
				tabb = 2;
			}
			ImGui::SameLine();
			if (ImGui::Button("Config"))
			{
				tabb = 3;
			}
			}
			if (tabb == 0)
			{
				ImGui::Checkbox("ESP Lines", &Store::m_espLines);

				ImGui::Checkbox("ESP Names", &Store::m_espNames);

				ImGui::Checkbox("ESP Boxes", &Store::m_espBoxes);
				
				ImGui::Checkbox("ESP Distance", &Store::m_espDistance);
				ImGui::Checkbox("ESP Dead Players", &Store::m_espDeadPlayers);

			}
			else if (tabb == 1)
			{
				ImGui::Checkbox("Enable Aim", &Store::m_aim);
				ImGui::SliderFloat("Aim Smooth", &Store::m_aimSpeed, 0.5, 5);

				ImGui::Checkbox("Display FOV", &Store::m_aimFovDisplay);
				ImGui::SliderInt("Aim FOV", &Store::m_aimFov, 0, 1000);
			}
			else if (tabb == 2)
			{
				ImGui::Checkbox("No Recoil (Not Working!)", &Store::m_noRecoil);
				ImGui::Checkbox("Enable Rapid Fire (Not Working!)", &Store::m_rapid);
			}
			else if (tabb == 3)
			{
				ImGui::Text("ESP Lines Colors");
				ImGui::SliderFloat("Line Red", &espL_Red, 0, 1);
				ImGui::SliderFloat("Line Green", &espL_Green, 0, 1);
				ImGui::SliderFloat("Line Blue", &espL_Blue, 0, 1);
				ImGui::SliderFloat("Line Alpha", &espL_Alpha, 0, 1);

				ImGui::Text("");
				ImGui::Text("ESP Box Colors");
				ImGui::SliderFloat("Box Red", &espB_Red, 0, 1);
				ImGui::SliderFloat("Box Green", &espB_Green, 0, 1);
				ImGui::SliderFloat("Box Blue", &espB_Blue, 0, 1);
				ImGui::SliderFloat("Box Alpha", &espB_Alpha, 0, 1);
			}

			ImGui::End();
		}
	}

	void Watermark()
	{
		uintptr_t playerCount = Store::players->m_uMaxLength;
		std::string debugMessage = "[Made By WeHaveNoEyes] Updated and Upgraded by laykonlee2020";
		std::string debugMessage2 = "INSERT (Ins) To Open";
		ImGui::GetBackgroundDrawList()->AddText({ 0, 0 }, ImColor(1.f, 1.f, 1.f), debugMessage.c_str());
		ImGui::GetBackgroundDrawList()->AddText({ 0, 15 }, ImColor(1.f, 0.f, 1.f), debugMessage2.c_str());
	}

	void DrawAim()
	{
		if (Store::localPlayer.initialized && Store::m_aim)
		{
			if (Store::m_aimFovDisplay)
			{
				ImGui::GetBackgroundDrawList()->AddCircle(ImVec2(Store::ScreenW / 2, Store::ScreenH / 2), Store::m_aimFov * 1.f, ImColor(1.f, 0.f, 0.f), 74);

				double nearestDistance = 99999.f;
				Unity::Vector3 nearestHeadScreenPos = { -1.f, -1.f, -1.f };

				Unity::Vector3 screenCenter;
				screenCenter.x = Store::ScreenW / 2;
				screenCenter.y = Store::ScreenH / 2;

				Unity::Vector3 localPos = Store::localPlayer.position();

				for (Player player : Store::playerList)
				{
					if (!player.initialized || player.local || player.isDead())
						continue;

					Unity::CComponent* head = player.getBone("head");
					Unity::Vector3 headPos = head->GetTransform()->GetPosition();

					Unity::Vector3 screenPos;
					if (Util::worldToScreen(headPos, &screenPos)) {

						double distance = player.distance(localPos);
						if (distance < nearestDistance)
						{
							float screenDistance = Util::screenDistance(screenPos, screenCenter);

							if (screenDistance <= Store::m_aimFov)
							{
								nearestDistance = distance;
								nearestHeadScreenPos = screenPos;
							}
						}
					}
				}

				if (nearestHeadScreenPos.x > 0)
				{
					// We have a target cuh
					if (GetAsyncKeyState(VK_XBUTTON2))
					{
						Unity::Vector2 moveToPoint = Util::aimAtPoint(nearestHeadScreenPos.x, nearestHeadScreenPos.y);
						mouse_event(MOUSEEVENTF_MOVE, (DWORD)moveToPoint.x, (DWORD)-(moveToPoint.y), NULL, NULL);
					}
				}
			}
		}
	}




	void DrawESP()
	{
		// Do nothing until we have a local player...
		if (Store::localPlayer.initialized)
		{
			Unity::Vector3 localPosition = Store::localPlayer.position();

			for (Player player : Store::playerList)
			{
				// Do nothing for the local player
				if (!player.initialized || player.local)
					continue;

				Unity::Vector3 playerPos = player.position();
		

				Unity::Vector3 screenPos;
				if (Util::worldToScreen(playerPos, &screenPos)) {
					// Get distance from me
					double distance = player.distance(Store::localPlayer.position());

					bool* dead = player.isDead();

					float yModifier = 0.f;

					if (Store::m_espDistance)
					{
						std::string distanceString = "[" + std::to_string((int)distance) + "m]";
						ImVec2 distanceStringSize = ImGui::CalcTextSize(distanceString.c_str());

						yModifier += distanceStringSize.y;

						if (Store::m_espDeadPlayers)
						{
							if (dead)
							{
								ImGui::GetBackgroundDrawList()->AddText({ screenPos.x - (distanceStringSize.x / 2), Store::ScreenH - screenPos.y }, ImColor(1.f, 0.f, 0.f), distanceString.c_str());
							}
							else {
								ImGui::GetBackgroundDrawList()->AddText({ screenPos.x - (distanceStringSize.x / 2), Store::ScreenH - screenPos.y }, ImColor(1.f, 1.f, 1.f), distanceString.c_str());
							}

							if (!dead)
								ImGui::GetBackgroundDrawList()->AddText({ screenPos.x - (distanceStringSize.x / 2), Store::ScreenH - screenPos.y }, ImColor(1.f, 1.f, 1.f), distanceString.c_str());
						}
						else {
							if (!dead)
								ImGui::GetBackgroundDrawList()->AddText({ screenPos.x - (distanceStringSize.x / 2), Store::ScreenH - screenPos.y }, ImColor(1.f, 1.f, 1.f), distanceString.c_str());
						}
					}

					if (Store::m_espNames)
					{
						std::string nameString = player.name();
						ImVec2 nameStringSize = ImGui::CalcTextSize(nameString.c_str());

						ImGui::GetBackgroundDrawList()->AddText({ screenPos.x - (nameStringSize.x / 2), Store::ScreenH - screenPos.y + yModifier }, ImColor(1.f, 1.f, 1.f), nameString.c_str());

						yModifier += nameStringSize.y;
					}

					if (Store::m_espBoxes)
					{
						BoxCoords bounds = player.calcBounds();
						ImGui::GetBackgroundDrawList()->AddRect(ImVec2(bounds.topLeft.x, bounds.topLeft.y + (20 / distance)), ImVec2(bounds.bottomRight.x, bounds.bottomRight.y), ImColor(espB_Red, espB_Green, espB_Blue, espB_Alpha));
					}

					if (!dead)
					{
						if (Store::m_espLines)
						{
							ImGui::GetBackgroundDrawList()->AddLine({ (float)Store::ScreenW / 2, (float)Store::ScreenH }, { screenPos.x, Store::ScreenH - screenPos.y + yModifier }, ImColor(espL_Red, espL_Green, espL_Blue, espL_Alpha));
						}
					}
				}
			}
		}
	}
}
