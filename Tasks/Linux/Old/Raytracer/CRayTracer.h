
#ifndef __RayTracer__CRayTracer__
#define __RayTracer__CRayTracer__

#include <stdio.h>
#include <assert.h>
#include <map>

#include "CScene.h"

class CRayTracer
{
    public:
    
        CRayTracer();
        ~CRayTracer();
    
        void Init           (const int& iWidth, const int& iHeight);
        bool Render         (const std::string& strFileName);
        CScene *GetScene    ();
        void SaveBMP        (const char *filename, int w, int h, int dpi, CVector3 *data);
        CVector3 RayTrace   (const int& iX, const int& iY);
    
    private:
        void ShootRay       (const CRay& _Ray,int iTraceDepth, CVector3& vAccumulatedPixelColor);
        int m_iWidth;
        int m_iHeight;
        int m_iRayTraceDepth;
        bool m_bSuperSampling;
        CVector3 *m_pDataOut;
        CScene      m_Scene;
};
