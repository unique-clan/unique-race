#ifndef GAME_SERVER_ENTITIES_SNAPPING_FLAG_H
#define GAME_SERVER_ENTITIES_SNAPPING_FLAG_H

#include <base/vmath.h>

#include <engine/server.h>

class CSnappingFlag
{
public:
	CSnappingFlag(IServer *pServer);
	~CSnappingFlag();
	void Snap();

	vec2 m_Pos;
	std::optional<int> m_SnapId;

private:
	IServer *m_pServer;
};

#endif
