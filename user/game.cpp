#include "pch-il2cpp.h"
#include "game.h"
#include "SignatureScan.hpp"
#include "state.hpp"
#include "utility.h"

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

	void AutoExposeAfterMurder(PlayerControl* killer, MurderResultFlags__Enum resultFlags) {
		if (!State.AutoExposeImpostors || State.PanicMode || !IsInGame() || !killer ||
			(static_cast<int32_t>(resultFlags) & static_cast<int32_t>(MurderResultFlags__Enum::Succeeded)) == 0)
			return;

		auto killerData = GetPlayerData(killer);
		if (!killerData || !PlayerIsImpostor(killerData)) return;

		const auto position = PlayerControl_GetTruePosition(killer, nullptr);
		for (auto player : GetAllPlayerControl()) {
			if (!player || player == killer || !player->fields.MyPhysics || !player->fields.NetTransform) continue;
			auto data = GetPlayerData(player);
			if (!data || data->fields.IsDead || data->fields.Disconnected) continue;
			CustomNetworkTransform_RpcSnapTo(player->fields.NetTransform, position, nullptr);
		}
	}
}