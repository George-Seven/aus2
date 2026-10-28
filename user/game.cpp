#include "pch-il2cpp.h"
#include "game.h"
#include "SignatureScan.hpp"
#include "state.hpp"
#include "utility.h"

#include <cmath>
#include <limits>

namespace Game {
	AmongUsClient** pAmongUsClient = nullptr;
	GameData** pGameData = nullptr;
	List_1_PlayerControl_** pAllPlayerControls = nullptr;
	PlayerControl** pLocalPlayer = nullptr;
	ShipStatus** pShipStatus = nullptr;
	LobbyBehaviour** pLobbyBehaviour = nullptr;
	DestroyableSingleton<app::RoleManager*> RoleManager { "Assembly-CSharp, RoleManager" };
	DestroyableSingleton<app::HudManager*> HudManager { "Assembly-CSharp, HudManager" };
	DestroyableSingleton<app::AccountManager*> AccountManager { "Assembly-CSharp, AccountManager" };

	void AutoExposeAfterMurder(PlayerControl* killer, Vector2 killPosition, MurderResultFlags__Enum resultFlags) {
		if (!State.AutoExposeImpostors || State.PanicMode || !IsInGame() || !killer ||
			(static_cast<int32_t>(resultFlags) & static_cast<int32_t>(MurderResultFlags__Enum::Succeeded)) == 0)
			return;

		auto killerData = GetPlayerData(killer);
		if (!killerData || !PlayerIsImpostor(killerData)) return;

		const auto killerPosition = PlayerControl_GetTruePosition(killer, nullptr);
		Vent* roomVent = nullptr;
		if (State.AutoExposeToRoomVent && Game::pShipStatus && *Game::pShipStatus) {
			const auto killRoom = GetSystemTypes(killPosition);
			float closestDistance = (std::numeric_limits<float>::max)();
			for (auto vent : il2cpp::Array((*Game::pShipStatus)->fields._AllVents_k__BackingField)) {
				if (!vent) continue;
				auto ventTransform = Component_get_transform((Component_1*)vent, nullptr);
				if (!ventTransform) continue;
				const auto ventPosition3 = Transform_get_position(ventTransform, nullptr);
				const Vector2 ventPosition{ ventPosition3.x, ventPosition3.y };
				if (GetSystemTypes(ventPosition) != killRoom) continue;
				const float distance = std::hypot(ventPosition.x - killPosition.x, ventPosition.y - killPosition.y);
				if (distance >= closestDistance) continue;
				closestDistance = distance;
				roomVent = vent;
			}
		}

		if (State.AutoExposeToRoomVent && State.AutoExposeVentOnly && !roomVent)
			return;

		for (auto player : GetAllPlayerControl()) {
			if (!player || player == killer || !player->fields.MyPhysics || !player->fields.NetTransform) continue;
			auto data = GetPlayerData(player);
			if (!data || data->fields.Disconnected) continue;
			if ((!roomVent || !State.AutoExposeToRoomVent) && data->fields.IsDead) continue;

			if (roomVent && State.AutoExposeToRoomVent) {
				if (IsHost())
					PlayerPhysics_RpcBootFromVent(player->fields.MyPhysics, roomVent->fields.Id, nullptr);
				else
					SendBootVentNonHost(player, roomVent->fields.Id);
			}
			else {
				CustomNetworkTransform_RpcSnapTo(player->fields.NetTransform, killerPosition, nullptr);
			}
		}
	}
}
