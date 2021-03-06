#include "stdafx.h"
#include "RealSpace2.h"

_NAMESPACE_REALSPACE2_BEGIN

rvector RCameraPosition, RCameraDirection, RCameraUp{ 0, 0, 1 };
rmatrix RView, RProjection, RViewProjection, RViewport, RViewProjectionViewport;
rplane RViewFrustum[6];
static float RFov_horiz, RFov_vert, RNearZ, RFarZ;
static D3DVIEWPORT9 g_d3dViewport;

void ComputeViewFrustum(rplane *plane, float x, float y, float z)
{
	plane->a = RView._11*x + RView._12*y + RView._13*z;
	plane->b = RView._21*x + RView._22*y + RView._23*z;
	plane->c = RView._31*x + RView._32*y + RView._33*z;
	plane->d = -plane->a*RCameraPosition.x
		- plane->b*RCameraPosition.y
		- plane->c*RCameraPosition.z;
}

void ComputeZPlane(rplane *plane, float z, int sign)
{
	rvector normal, t;
	t = RCameraPosition + z * RCameraDirection;
	normal = Normalized(float(sign) * RCameraDirection);
	plane->a = normal.x; plane->b = normal.y; plane->c = normal.z;
	plane->d = -plane->a*t.x - plane->b*t.y - plane->c*t.z;
}

void UpdateViewFrustrum()
{
	float fovh2 = RFov_horiz / 2.0f, fovv2 = RFov_vert / 2.0f;
	float ch = cosf(fovh2), sh = sinf(fovh2);
	float cv = cosf(fovv2), sv = sinf(fovv2);

	ComputeViewFrustum(RViewFrustum + 0, -ch, 0, sh);
	ComputeViewFrustum(RViewFrustum + 1, ch, 0, sh);
	ComputeViewFrustum(RViewFrustum + 2, 0, cv, sv);
	ComputeViewFrustum(RViewFrustum + 3, 0, -cv, sv);
	ComputeZPlane(RViewFrustum + 4, RNearZ, 1);
	ComputeZPlane(RViewFrustum + 5, RFarZ, -1);

	RViewProjection = RView * RProjection;
	RViewProjectionViewport = RViewProjection * RViewport;

}

void RSetCamera(const rvector &from, const rvector &at, const rvector &up)
{
	RCameraPosition = from;
	RCameraDirection = at - from;
	RCameraUp = up;

	RUpdateCamera();
}

void RUpdateCamera()
{
	RView = ViewMatrix(RCameraPosition, RCameraDirection, RCameraUp);
	RSetTransform(D3DTS_VIEW, RView);

	auto CheckNaN = [](auto& vec, const v3& def = { 1, 0, 0 })
	{
		if (isnan(vec))
			vec = def;
	};

	CheckNaN(RCameraPosition);
	CheckNaN(RCameraDirection);
	CheckNaN(RCameraUp, { 0, 0, 1 });

	UpdateViewFrustrum();
}

void RSetProjection(float fFov, float fAspect, float fNearZ, float fFarZ)
{
	RFov_horiz = fFov;
	RFov_vert = atanf(tanf(RFov_horiz / 2.0f) / fAspect)*2.0f;
	RNearZ = fNearZ; RFarZ = fFarZ;

	RProjection = PerspectiveProjectionMatrix(fAspect, RFov_vert, fNearZ, fFarZ);
	RSetTransform(D3DTS_PROJECTION, RProjection);

	UpdateViewFrustrum();
}

void RSetProjection(float fFov, float fNearZ, float fFarZ)
{
	FLOAT fAspect = (FLOAT)RGetScreenWidth() / (FLOAT)RGetScreenHeight();

	RFov_horiz = fFov;
	RFov_vert = atanf(tanf(RFov_horiz / 2.0f) / fAspect)*2.0f;
	RNearZ = fNearZ; RFarZ = fFarZ;

	RProjection = PerspectiveProjectionMatrix(fAspect, RFov_vert, fNearZ, fFarZ);
	RSetTransform(D3DTS_PROJECTION, RProjection);

	UpdateViewFrustrum();
}

D3DVIEWPORT9* RGetViewport() { return &g_d3dViewport; }

void RSetViewport(int x1, int y1, int x2, int y2)
{
	D3DVIEWPORT9 *pViewport = RGetViewport();

	pViewport->X = x1; pViewport->Y = y1;
	pViewport->Width = x2 - x1; pViewport->Height = y2 - y1;
	pViewport->MinZ = 0;
	pViewport->MaxZ = 1;
	HRESULT hr = RGetDevice()->SetViewport(pViewport);
	_ASSERT(hr == D3D_OK);

	float RSwx = (float)(x2 - x1) / 2;
	float RSwy = (float)(y2 - y1) / 2;
	float RScx = (float)RSwx + x1;
	float RScy = (float)RSwy + y1;

	RViewport = IdentityMatrix();
	RViewport._11 = RSwx;
	RViewport._22 = -RSwy;
	RViewport._41 = RScx;
	RViewport._42 = RScy;
}

rvector RGetCameraPosition()
{
	return RCameraPosition;
}

void RResetTransform()
{
	RSetTransform(D3DTS_VIEW, RView);
	RSetTransform(D3DTS_PROJECTION, RProjection);
}
_NAMESPACE_REALSPACE2_END