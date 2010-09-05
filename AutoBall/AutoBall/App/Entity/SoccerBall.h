
#pragma  once 

#include "MoveEntity.h"
#include "../Wall2D.h"
#include "../../Public/GameSetup.h"
#include "../../Public/Singleton.h"
#include "../../Render/VGdi.h"

///	 足球类
class  SoccerBall : public CMoveEntity
{
public:
	SoccerBall(Vector2D      pos,            
			   double        BallSize,
			   double        mass,
		std::vector<Wall2D>& PitchBoundary):m_PitchBoundary(PitchBoundary),
		CMoveEntity(pos,
			BallSize,
			Vector2D(0,0),
			-1.0,                
			Vector2D(0,1),
			mass,
			Vector2D(1.0,1.0),  
			0,                 
			0)		
	{

	}

	~SoccerBall(){}

public:

	/// 碰撞检测,然后向相反方向移动
	void TestCollisionWithWalls(const std::vector<Wall2D>& walls);

	//implement base class Update
	void      Update();

	//implement base class Render
	void      Render();

	/// 足球不需要消息
	bool      HandleMessage(const Telegram& msg){return false;}

	/// 收到一个方向力
	void      Kick(Vector2D direction, double force);

	/// 
	///	 给定一个踢球力和通过起点和终点的定义的移动距离
	///	 改方法计算球经过这段距离要花多长时间
	/// 
	double    TimeToCoverDistance(Vector2D from,
		Vector2D to,
		double     force)const;

	/// 该方法计算在给定时间后球的位置
	Vector2D FuturePosition(double time)const;

	/// 这被用于队员和守门员用于停球，停球后表示控制了球
	void      Trap(){m_vVelocity.Zero();}  

	/// 球的上一次更新中的位置
	Vector2D  OldPos()const{return m_vOldPos;}

	/// 设置球的新位置，重置速度为0
	void      PlaceAtPosition(Vector2D NewPos);

private:

	/// 记录球在上一次更新中的位置
	Vector2D                  m_vOldPos;

	/// 组成足球场边界的本地引用(用于碰撞检测)
	const std::vector<Wall2D>& m_PitchBoundary;   

};

inline Vector2D AddNoiseToKick(Vector2D BallPos, Vector2D BallTarget)
{

	double displacement = (Pi - Pi*GetInstObj(CGameSetup).PlayerKickingAccuracy) * RandomClamped();

	Vector2D toTarget = BallTarget - BallPos;

	Vec2DRotateAroundOrigin(toTarget, displacement);

	return toTarget + BallPos;
}