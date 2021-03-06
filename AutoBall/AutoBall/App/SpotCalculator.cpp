#include "Stdafx.h"
#include "SpotCalculator.h"
#include "Goal.h"
#include "FootBallTeam.h"
#include "FootBallPitch.h"
#include "Entity/BasePlayer.h"
#include "Entity/FootBall.h"
#include "../Public/GameSetup.h"


SpotCalculator::~SpotCalculator()
{
	SAFE_DELETE( m_pPassTimer );
}


SpotCalculator::SpotCalculator(int           numX,
											 int           numY,
											 FootBallTeam*   team):m_pBestSupportingSpot(NULL),
											 m_pTeam(team)
{
	const Region* Playinger = team->Pitch()->PlayingArea();

	double HeightOfSSRegion = Playinger->Height() * 0.8;
	double WidthOfSSRegion  = Playinger->Width() * 0.9;
	double SliceX = WidthOfSSRegion / numX ;
	double SliceY = HeightOfSSRegion / numY;

	double left  = Playinger->Left() + (Playinger->Width()-WidthOfSSRegion)/2.0 + SliceX/2.0;
	double right = Playinger->Right() - (Playinger->Width()-WidthOfSSRegion)/2.0 - SliceX/2.0;
	double top   = Playinger->Top() + (Playinger->Height()-HeightOfSSRegion)/2.0 + SliceY/2.0;

	for (int x=0; x<(numX/2)-1; ++x)
	{
		for (int y=0; y<numY; ++y)
		{      
			if (m_pTeam->Color() == FootBallTeam::blue)
			{
				m_Spots.push_back(BestSpot(Vector2D(left+x*SliceX, top+y*SliceY), 0.0));
			}

			else
			{
				m_Spots.push_back(BestSpot(Vector2D(right-x*SliceX, top+y*SliceY), 0.0));
			}
		}
	}

	m_pPassTimer = new TimeCount(1);
}


Vector2D SpotCalculator::BestSupportingPosition()
{
	if (!m_pPassTimer->isReadyOK() && m_pBestSupportingSpot)
	{
		return m_pBestSupportingSpot->m_vPos;
	}

	/// 重置最佳接应点
	m_pBestSupportingSpot = NULL;

	double BestScoreSoFar = 0.0;

	std::vector<BestSpot>::iterator curSpot;

	for (curSpot = m_Spots.begin(); curSpot != m_Spots.end(); ++curSpot)
	{
		/// 首先删除以前的分数
		curSpot->m_dScore = 1.0;

		/// 首先传到这个位置是否安全
		if(m_pTeam->isPassSafeFromAllOpponents(m_pTeam->ControllingPlayer()->Pos(),
			curSpot->m_vPos,
			NULL,
			3))
		{
			curSpot->m_dScore += 2.0;
		}

		/// 可以在这个位置射门
		if( m_pTeam->CanShootGoal(curSpot->m_vPos,6))
		{
			curSpot->m_dScore += 1.0;
		}   

		/// 这个点离控球队员多远,越远分数越高
		/// 同时有一个最远的OptimalDistance的距离
		if (m_pTeam->SupportingPlayer())
		{
			const double OptimalDistance = 200.0;

			double dist = Vec2DDistance(m_pTeam->ControllingPlayer()->Pos(),
				curSpot->m_vPos);

			double temp = fabs(OptimalDistance - dist);

			if (temp < OptimalDistance)
			{
				/// 标准化距离，把它加到分数中
				curSpot->m_dScore += 2.0 *(OptimalDistance-temp)/OptimalDistance;  
			}
		}

		/// 检查到目前位置这个点是否是最高分
		if (curSpot->m_dScore > BestScoreSoFar)
		{
			BestScoreSoFar = curSpot->m_dScore;

			m_pBestSupportingSpot = &(*curSpot);
		}    
	}

	return m_pBestSupportingSpot->m_vPos;
}

Vector2D SpotCalculator::GetBestSupportingSpot()
{
	if (m_pBestSupportingSpot)
	{
		return m_pBestSupportingSpot->m_vPos;
	}

	else
	{ 
		return BestSupportingPosition();
	}
}

void SpotCalculator::Render()const
{
	GetInstObj(CGDI).HollowBrush();
	GetInstObj(CGDI).GreyPen();

	for (unsigned int spt=0; spt<m_Spots.size(); ++spt)
	{
		GetInstObj(CGDI).Circle(m_Spots[spt].m_vPos, m_Spots[spt].m_dScore);
	}

	if (m_pBestSupportingSpot)
	{
		GetInstObj(CGDI).GreenPen();
		GetInstObj(CGDI).Circle(m_pBestSupportingSpot->m_vPos, m_pBestSupportingSpot->m_dScore);
	}
}