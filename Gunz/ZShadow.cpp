#include "stdafx.h"

#include ".\zshadow.h"
#include ".\ZCharacter.h"
#include "RBspObject.h"
#include "MDebug.h"

#define VALID_SHADOW_LENGTH 250
#define VALID_SHADOW_BOUNDARY_SQUARE 62500

extern ZApplication g_app;

rvector ZShadow::mNormal;

//////////////////////////////////////////////////////////////////////////
//	Constructor / Desturctor
//////////////////////////////////////////////////////////////////////////
ZShadow::ZShadow(void)
{
	// normal�� +z
	mNormal = rvector( 0.f, 0.f, 1.f);

	GetIdentityMatrix(mWorldMatLF );
	GetIdentityMatrix(mWorldMatRF );

	bLFShadow = false;
	bRFShadow = false;
}

//////////////////////////////////////////////////////////////////////////
//	draw
//////////////////////////////////////////////////////////////////////////
void ZShadow::draw(bool bForced)
{
	if( (bLFShadow==false) && ((bRFShadow==false)))
		return;

	float blend_factor = ( mfDistance )/ VALID_SHADOW_BOUNDARY_SQUARE;

		 if( blend_factor >= 1  )	blend_factor = 0;
	else if( mfDistance <= 0 )		blend_factor = 1;
	else							blend_factor = 1 - blend_factor;

	DWORD _color = ((DWORD)(blend_factor * 255))<<24 | 0xffffff;

	if(bLFShadow)
		ZGetEffectManager()->AddShadowEffect(mWorldMatLF,_color);

	if(bRFShadow)
		ZGetEffectManager()->AddShadowEffect(mWorldMatRF,_color);

}

//////////////////////////////////////////////////////////////////////////
//	setSize
//	(desc) return scale matrix which scales by size_
//	(ref) this shadow object is unit length(1) 2d box
//	+ helper function
//////////////////////////////////////////////////////////////////////////
rmatrix ZShadow::setSize( float size_  )
{
	return ScalingMatrix(size_);
}

//////////////////////////////////////////////////////////////////////////
//	setDirection
//	(desc) return rotation matrix to match shadow object's normal to input direction
//	+ helper function
//////////////////////////////////////////////////////////////////////////
rmatrix ZShadow::setDirection( rvector& dir_ )
{
	rmatrix xRotMat;
	rmatrix yRotMat;

	rvector xVector = dir_;
	xVector.y = 0;
	float xtheta = DotProduct(mNormal, xVector);

	rvector yVector = dir_;
	yVector.x = 0;
	float yTheta = DotProduct(mNormal, yVector);

	xRotMat = RGetRotX(xtheta);
	yRotMat = RGetRotY(yTheta);

	return xRotMat*yRotMat;
}

bool ZShadow::setMatrix(ZCharacterObject& char_, float size_  )
{
	return setMatrix( *char_.m_pVMesh ,size_);
}

bool ZShadow::setMatrix( RVisualMesh& vmesh, float size_, RBspObject* p_map)
{
	rvector footPosition[2];
	footPosition[0] = vmesh.GetLFootPosition();
	footPosition[1] = vmesh.GetRFootPosition();
	if( p_map == 0 )
		p_map = g_app.GetGame()->GetWorld()->GetBsp();

	rvector floorPosition[2];
	rvector dir[2];

	bLFShadow = true;
	bRFShadow = true;

	if( !p_map->GetShadowPosition( footPosition[0], rvector( 0, 0, -1), &dir[0], &floorPosition[0] ))
	{
		if(g_pGame) {
			floorPosition[0] = g_pGame->GetFloor(footPosition[0]);
		} else {
			bLFShadow = false;
		}

	}
	if( !p_map->GetShadowPosition( footPosition[1], rvector( 0, 0, -1), &dir[1], &floorPosition[1] ))
	{
		if(g_pGame) {
			floorPosition[1] = g_pGame->GetFloor(footPosition[1]);
		} else { 
			bRFShadow = false;
		}
	}

	if( (bLFShadow==false) && ((bRFShadow==false)))
		return false;

	float distanceL , distanceR;
	auto vecx = footPosition[0] - floorPosition[0];
	auto vecy = footPosition[1] - floorPosition[1];
	distanceL = MagnitudeSq(vecx) - 200;
	distanceR = MagnitudeSq(vecy) - 200;
	
	if( VALID_SHADOW_BOUNDARY_SQUARE >= distanceL && floorPosition[0].z < footPosition[0].z )	bLFShadow = true;
	else	bLFShadow = false;
	
	if( VALID_SHADOW_BOUNDARY_SQUARE >= distanceR && floorPosition[1].z < footPosition[1].z)	bRFShadow = true;
	else	bRFShadow = false;
		
	mfDistance = ( distanceL + distanceR ) * 0.5 ;
	
	//matrix setup
	float fSize = vmesh.m_vScale.x * size_;
	rmatrix scaleMat = setSize( size_ );
	
	if( bLFShadow )
		mWorldMatLF = scaleMat * TranslationMatrix(floorPosition[0] + v3{0, 0, 1});
	if( bRFShadow )
		mWorldMatRF = scaleMat * TranslationMatrix(floorPosition[0] + v3{ 0, 0, 1 });

	return true;
}
