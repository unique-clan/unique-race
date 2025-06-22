#include "snapping_flag.h"

CSnappingFlag::CSnappingFlag(IServer *pServer)
{
	m_pServer = pServer;
}

CSnappingFlag::~CSnappingFlag()
{
	if(m_SnapId.has_value())
		m_pServer->SnapFreeId(m_SnapId.value());
}

void CSnappingFlag::Snap()
{
	if(!m_SnapId.has_value())
		m_SnapId = m_pServer->SnapNewId();
	CNetObj_Flag *pFlag = (CNetObj_Flag *)m_pServer->SnapNewItem(NETOBJTYPE_FLAG, m_SnapId.value(), sizeof(CNetObj_Flag));
	if(!pFlag)
		return;
	pFlag->m_X = (int)m_Pos.x;
	pFlag->m_Y = (int)m_Pos.y;
	pFlag->m_Team = TEAM_RED;
}
