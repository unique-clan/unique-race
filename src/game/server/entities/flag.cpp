/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */

#include <game/server/gamecontext.h>
#include "flag.h"

CFlag::CFlag(CGameWorld *pGameWorld)
: CEntity(pGameWorld, CGameWorld::ENTTYPE_FLAG)
{
	m_ProximityRadius = ms_PhysSize;
	m_pCarryingCharacter = NULL;
}

void CFlag::Snap(int SnappingClient)
{
	m_Pos = m_pCarryingCharacter->m_Pos;

	if(!GameServer()->m_apPlayers[SnappingClient]->m_ShowOthers)
		return;
	if(NetworkClipped(SnappingClient) && !GameServer()->m_apPlayers[SnappingClient]->m_ShowAll)
		return;

	CNetObj_Flag *pFlag = (CNetObj_Flag *)Server()->SnapNewItem(NETOBJTYPE_FLAG, 0, sizeof(CNetObj_Flag));
	if(!pFlag)
		return;
	pFlag->m_X = (int)m_Pos.x;
	pFlag->m_Y = (int)m_Pos.y;
	pFlag->m_Team = 0;

	CNetObj_GameData *pGameDataObj = (CNetObj_GameData *)Server()->SnapNewItem(NETOBJTYPE_GAMEDATA, 0, sizeof(CNetObj_GameData));
	if(!pGameDataObj)
		return;
	pGameDataObj->m_TeamscoreRed = 0;
	pGameDataObj->m_TeamscoreBlue = 0;
	pGameDataObj->m_FlagCarrierRed = m_pCarryingCharacter->GetPlayer()->GetCID();
	pGameDataObj->m_FlagCarrierBlue = FLAG_MISSING;
}
