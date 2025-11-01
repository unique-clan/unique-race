/* (c) Shereef Marzouk. See "licence DDRace.txt" and the readme.txt in the root of the distribution for more information. */
/* Based on Race mod stuff and tweaked by GreYFoX@GTi and others to fit our DDRace needs. */
#include "Unique.h"

#include <engine/server.h>
#include <engine/shared/config.h>

#include <game/mapitems.h>
#include <game/server/entities/character.h>
#include <game/server/gamecontext.h>
#include <game/server/player.h>
#include <game/server/score.h>
#include <game/version.h>

#define GAME_TYPE_RACE "Race"
#define GAME_TYPE_FASTCAP "FastCap"
#define GAME_TYPE_SHORTS "Shorts"
#define TEST_TYPE_NAME "TestUniqueRace"

CGameControllerUnique::CGameControllerUnique(class CGameContext *pGameServer) :
	IGameController(pGameServer)
{
	if(g_Config.m_SvTestingCommands)
		m_pGameType = TEST_TYPE_NAME;
	else if(g_Config.m_SvShorts)
		m_pGameType = GAME_TYPE_SHORTS;
	else
		m_pGameType = g_Config.m_SvFastcap ? GAME_TYPE_FASTCAP : GAME_TYPE_RACE;
	m_GameFlags = protocol7::GAMEFLAG_RACE;
	m_pRecordFlagChar = nullptr;
}

CGameControllerUnique::~CGameControllerUnique() = default;

CScore *CGameControllerUnique::Score()
{
	return GameServer()->Score();
}

void CGameControllerUnique::HandleCharacterTiles(CCharacter *pChr, int MapIndex, float FractionOfTick)
{
	CPlayer *pPlayer = pChr->GetPlayer();
	const int ClientId = pPlayer->GetCid();

	int TileIndex = GameServer()->Collision()->GetTileIndex(MapIndex);
	int TileFIndex = GameServer()->Collision()->GetFrontTileIndex(MapIndex);

	int Tile1 = 0;
	int Tile2 = 0;
	int Tile3 = 0;
	int Tile4 = 0;
	int FTile1 = 0;
	int FTile2 = 0;
	int FTile3 = 0;
	int FTile4 = 0;

	//Sensitivity
	if(!g_Config.m_SvProximityChecks)
	{
		int S1 = GameServer()->Collision()->GetPureMapIndex(vec2(pChr->GetPos().x + pChr->GetProximityRadius() / 3.f, pChr->GetPos().y - pChr->GetProximityRadius() / 3.f));
		int S2 = GameServer()->Collision()->GetPureMapIndex(vec2(pChr->GetPos().x + pChr->GetProximityRadius() / 3.f, pChr->GetPos().y + pChr->GetProximityRadius() / 3.f));
		int S3 = GameServer()->Collision()->GetPureMapIndex(vec2(pChr->GetPos().x - pChr->GetProximityRadius() / 3.f, pChr->GetPos().y - pChr->GetProximityRadius() / 3.f));
		int S4 = GameServer()->Collision()->GetPureMapIndex(vec2(pChr->GetPos().x - pChr->GetProximityRadius() / 3.f, pChr->GetPos().y + pChr->GetProximityRadius() / 3.f));
		Tile1 = GameServer()->Collision()->GetTileIndex(S1);
		Tile2 = GameServer()->Collision()->GetTileIndex(S2);
		Tile3 = GameServer()->Collision()->GetTileIndex(S3);
		Tile4 = GameServer()->Collision()->GetTileIndex(S4);
		FTile1 = GameServer()->Collision()->GetFrontTileIndex(S1);
		FTile2 = GameServer()->Collision()->GetFrontTileIndex(S2);
		FTile3 = GameServer()->Collision()->GetFrontTileIndex(S3);
		FTile4 = GameServer()->Collision()->GetFrontTileIndex(S4);
	}

	const ERaceState &PlayerDDRaceState = pChr->m_DDRaceState;
	bool IsOnStartTile = (TileIndex == TILE_START) || (TileFIndex == TILE_START) || FTile1 == TILE_START || FTile2 == TILE_START || FTile3 == TILE_START || FTile4 == TILE_START || Tile1 == TILE_START || Tile2 == TILE_START || Tile3 == TILE_START || Tile4 == TILE_START;
	// start
	if(IsOnStartTile && PlayerDDRaceState != ERaceState::CHEATED)
	{
		const int Team = GameServer()->GetDDRaceTeam(ClientId);
		if(Teams().GetSaving(Team))
		{
			GameServer()->SendStartWarning(ClientId, "You can't start while loading/saving of team is in progress");
			pChr->Die(ClientId, WEAPON_WORLD);
			return;
		}
		if(g_Config.m_SvTeam == SV_TEAM_MANDATORY && (Team == TEAM_FLOCK || Teams().Count(Team) <= 1))
		{
			GameServer()->SendStartWarning(ClientId, "You have to be in a team with other tees to start");
			pChr->Die(ClientId, WEAPON_WORLD);
			return;
		}
		if(g_Config.m_SvTeam != SV_TEAM_FORCED_SOLO && Team > TEAM_FLOCK && Team < TEAM_SUPER && Teams().Count(Team) < g_Config.m_SvMinTeamSize && !Teams().TeamFlock(Team))
		{
			char aBuf[128];
			str_format(aBuf, sizeof(aBuf), "Your team has fewer than %d players, so your team rank won't count", g_Config.m_SvMinTeamSize);
			GameServer()->SendStartWarning(ClientId, aBuf);
		}
		if(g_Config.m_SvResetPickups)
		{
			pChr->ResetPickups();
		}
		/*bool started = false;
		if(pChr->m_DDRaceState != ERaceState::STARTED)
		{
			started = true;
		}*/
		if(pChr->m_DDRaceState != ERaceState::STARTED)
			Teams().OnCharacterStart(ClientId, FractionOfTick);
		/*if(pChr->m_DDRaceState == ERaceState::STARTED && started)
		{
			dbg_msg("dbg", "Start Tick %d, StartOffset %f", Server()->Tick(), FractionOfTick);
		}*/

		pChr->m_LastTimeCp = -1;
		pChr->m_LastTimeCpBroadcasted = -1;
		for(float &CurrentTimeCp : pChr->m_aCurrentTimeCp)
		{
			CurrentTimeCp = 0.0f;
		}
	}

	// finish
	if(((TileIndex == TILE_FINISH) || (TileFIndex == TILE_FINISH) || FTile1 == TILE_FINISH || FTile2 == TILE_FINISH || FTile3 == TILE_FINISH || FTile4 == TILE_FINISH || Tile1 == TILE_FINISH || Tile2 == TILE_FINISH || Tile3 == TILE_FINISH || Tile4 == TILE_FINISH) && PlayerDDRaceState == ERaceState::STARTED)
	{
		//dbg_msg("dbg", "hit finish in tick %d", Server()->Tick());
		Teams().OnCharacterFinish(ClientId, FractionOfTick);
	}

	// unlock team
	else if(((TileIndex == TILE_UNLOCK_TEAM) || (TileFIndex == TILE_UNLOCK_TEAM)) && Teams().TeamLocked(GameServer()->GetDDRaceTeam(ClientId)))
	{
		Teams().SetTeamLock(GameServer()->GetDDRaceTeam(ClientId), false);
		GameServer()->SendChatTeam(GameServer()->GetDDRaceTeam(ClientId), "Your team was unlocked by an unlock team tile");
	}

	// solo part
	/*if(((TileIndex == TILE_SOLO_ENABLE) || (TileFIndex == TILE_SOLO_ENABLE)) && !Teams().m_Core.GetSolo(ClientId))
	{
		GameServer()->SendChatTarget(ClientId, "You are now in a solo part");
		pChr->SetSolo(true);
	}
	else if(((TileIndex == TILE_SOLO_DISABLE) || (TileFIndex == TILE_SOLO_DISABLE)) && Teams().m_Core.GetSolo(ClientId))
	{
		GameServer()->SendChatTarget(ClientId, "You are now out of the solo part");
		pChr->SetSolo(false);
	}*/
}

void CGameControllerUnique::SetArmorProgress(CCharacter *pCharacter, int Progress)
{
	if(!g_Config.m_SvHealthAndAmmo)
		pCharacter->SetArmor(std::clamp(10 - (Progress / 15), 0, 10));
}

void CGameControllerUnique::OnPlayerConnect(CPlayer *pPlayer)
{
	IGameController::OnPlayerConnect(pPlayer);
	int ClientId = pPlayer->GetCid();

	// init the player
	Score()->PlayerData(ClientId)->Reset();

	// Can't set score here as LoadScore() is threaded, run it in
	// LoadScoreThreaded() instead
	Score()->LoadPlayerData(ClientId);

	if(!Server()->ClientPrevIngame(ClientId))
	{
		char aBuf[512];
		str_format(aBuf, sizeof(aBuf), "'%s' entered and joined the %s", Server()->ClientName(ClientId), GetTeamName(pPlayer->GetTeam()));
		GameServer()->SendChat(-1, TEAM_ALL, aBuf, -1, CGameContext::FLAG_SIX);

		GameServer()->SendChatTarget(ClientId, "DDraceNetwork Mod. Version: " GAME_VERSION);
		GameServer()->SendChatTarget(ClientId, "please visit DDNet.org or say /info and make sure to read our /rules");
	}
}

void CGameControllerUnique::OnPlayerDisconnect(CPlayer *pPlayer, const char *pReason)
{
	int ClientId = pPlayer->GetCid();
	bool WasModerator = pPlayer->m_Moderating && Server()->ClientIngame(ClientId);

	IGameController::OnPlayerDisconnect(pPlayer, pReason);

	if(!GameServer()->PlayerModerating() && WasModerator)
		GameServer()->SendChat(-1, TEAM_ALL, "Server kick/spec votes are no longer actively moderated.");

	if(g_Config.m_SvTeam != SV_TEAM_FORCED_SOLO)
		Teams().SetForceCharacterTeam(ClientId, TEAM_FLOCK);

	for(int Team = TEAM_FLOCK + 1; Team < TEAM_SUPER; Team++)
		if(Teams().IsInvited(Team, ClientId))
			Teams().SetClientInvited(Team, ClientId, false);
}

void CGameControllerUnique::OnReset()
{
	IGameController::OnReset();
	Teams().Reset();
}

void CGameControllerUnique::Tick()
{
	IGameController::Tick();
	Teams().ProcessSaveTeam();
	Teams().Tick();
}

void CGameControllerUnique::DoTeamChange(class CPlayer *pPlayer, int Team, bool DoChatMsg)
{
	Team = ClampTeam(Team);
	if(Team == pPlayer->GetTeam())
		return;

	CCharacter *pCharacter = pPlayer->GetCharacter();

	if(Team == TEAM_SPECTATORS)
	{
		if(g_Config.m_SvTeam != SV_TEAM_FORCED_SOLO && pCharacter)
		{
			// Joining spectators should not kill a locked team, but should still
			// check if the team finished by you leaving it.
			int DDRTeam = pCharacter->Team();
			Teams().SetForceCharacterTeam(pPlayer->GetCid(), TEAM_FLOCK);
			Teams().CheckTeamFinished(DDRTeam);
		}
	}

	IGameController::DoTeamChange(pPlayer, Team, DoChatMsg);
}

int CGameControllerUnique::OnCharacterDeath(class CCharacter *pVictim, class CPlayer *pKiller, int Weapon)
{
	if(m_pRecordFlagChar && m_pRecordFlagChar == pVictim)
		m_pRecordFlagChar = NULL;
	return 0;
}

int CGameControllerUnique::TileFlagsToPickupFlags(int TileFlags) const
{
	return 0;
}
