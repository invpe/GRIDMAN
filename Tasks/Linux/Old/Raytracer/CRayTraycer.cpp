

#include "CRayTracer.h"

CRayTracer::CRayTracer()
{
    m_iRayTraceDepth = 1;
    m_bSuperSampling = true;
}
void CRayTracer::Init(const int& iWidth, const int& iHeight)
{
    m_iWidth = iWidth;
    m_iHeight = iHeight;
    
    int n = iWidth * iHeight;
    m_pDataOut = new CVector3[n];
    
    m_Scene.Init();
    
}
// Single pixel raytrace
CVector3 CRayTracer::RayTrace(const int& iX, const int& iY)
{
    CScene *pScene                  = &m_Scene;
    CCamera *pCamera                = pScene->GetCamera();
    
    
    // Calculate w,u,v vector for camera (Move to CCamera)
    CVector3 w = (pCamera->GetTarget() - pCamera->GetPosition());
    w.normalize();
    CVector3 u = CVector3::cross(CVector3(0,1,0),w);
    CVector3 v = CVector3::cross(w,u);
    
    
    //
    CVector3 vSSColor(0,0,0);
    CVector3 vColor(0,0,0);
    
    // Perform supersampling with box filter and average down adjacent pixels
    if(m_bSuperSampling == true)
    {
        vSSColor = CVector3(0,0,0);
        for(int iSSX = -2; iSSX < 2; iSSX++)
        {
            for(int iSSY= -2; iSSY < 2; iSSY++)
            {
                
                //http://tiku.io/questions/1662254/camera-in-ray-tracing
                // map to -0.5 -> 0.5 with 0.0 at the center
                float xp = (float)(iSSX+iX)/(float)m_iWidth - 0.5;
                float yp = (float)(iSSY+iY)/(float)m_iHeight - 0.5;
                
                // Calculate Ray Direction
                CVector3 dir = w + xp * u + yp * v;
                dir.normalize();
                
                // Build a ray from Camera to the pixel
                CRay _Ray;
                _Ray.Set(pCamera->GetPosition(), dir);
                
                //
                ShootRay(_Ray, 0, vSSColor);
            }
        }
        
        // Store the pixel color
        vSSColor /= 16;
    }
    
    // Fire the Primary ray and add supersampling color
    // map to -0.5 -> 0.5 with 0.0 at the center
    float xp = (float)iX/(float)m_iWidth - 0.5;
    float yp = (float)iY/(float)m_iHeight - 0.5;
    
    // Calculate Ray Direction
    CVector3 dir = w + xp * u + yp * v;
    dir.normalize();
    
    // Build a ray from Camera to the pixel
    CRay _Ray;
    _Ray.Set(pCamera->GetPosition(), dir);
    
    ShootRay(_Ray, 0, vColor);

    // Return Primary color + eventual SuperSampling color
    return vColor + vSSColor;
}
// Render full image and store to output file
bool CRayTracer::Render(const std::string& strFileName)
{
    for(int x = 0; x < m_iWidth; x++)
    {
        for(int y = 0; y < m_iHeight; y++)
        {
            
            // Store pixel color
            m_pDataOut[y * m_iWidth + x] = RayTrace(x, y);
            
        }
    }
    
    //
    SaveBMP(strFileName.c_str(), m_iWidth, m_iHeight, 72, m_pDataOut);
 
    //
    return true;
}
//
void CRayTracer::ShootRay(const CRay& _Ray, int iTraceDepth, CVector3& vAccumulatedPixelColor)
{
    //
    if(iTraceDepth > m_iRayTraceDepth)
    {
        printf("Reached Max Trace Depth\n");
        return;
    }
    
    // Get all scene objects and find out which has the closest ray intersection
    std::vector<CObject*> vpObjects = GetScene()->GetObjects();
    
    // We will store the intersecting objects ID's in a map value which is sorted by the Key which is a distance of intersection
    std::map<float,int> mIntersections;
    
    // Find closest intersection object
    for(int a = 0; a < vpObjects.size(); a++)
    {
        
        // Store the distance from the origin to the intersection point
        float fDistance = vpObjects[a]->GetCollision()->GetIntersection(_Ray);
      

        // Store only these that are hit
        if(fDistance >= 0)
            mIntersections[fDistance] = a;

    }

    // No objects intersected with the ray
    if(mIntersections.empty())
        return;
    
    //
    CObject *pIntersectedObject = NULL;
    
    
    // Get first intersected object
    pIntersectedObject = vpObjects[mIntersections.begin()->second];
 
    // Vector containing World Space Coordinates of intersection with the object
    CVector3 vWorldSpaceIntersectionPosition        = _Ray.GetOrigin() + _Ray.GetDirection() * mIntersections.begin()->first;
            
    // Compute Light for this object
    if(pIntersectedObject->GetType() == CObject::eType::LIGHT)
    {
        vAccumulatedPixelColor += pIntersectedObject->GetMaterial()->GetColor();
    }
    else
    {
        // Get a list of lights and calculate lighting for each of the lights in the scene for the intersected object
        std::vector<CObject*> vpLights = GetScene()->GetObjects(CObject::eType::LIGHT);
        for(int b = 0; b < vpLights.size(); b++)
        {
            
            // SHADOW TEST
            // Shoot the ray in the direction of this light and find if there was any object on the line
            // If yes - skip this light - we are in the shadow
            CVector3 vDirectionToLight = vpLights[b]->GetPosition() - vWorldSpaceIntersectionPosition;
            vDirectionToLight.normalize();
            
            CRay _ShadowTestRay;
            _ShadowTestRay.Set(vWorldSpaceIntersectionPosition, vDirectionToLight);
            
            //
            float fShadeFactor = 1.0;
            
            // Check against all objects in the scene that are closer to the vWorldSpaceIntersectionPosition than the Current light
            for(int c = 0; c < vpObjects.size(); c++)
            {
                // Do not test against ourselfs
                if(vpObjects[c] == pIntersectedObject)
                    continue;
                
                // Do not test against lights they do not cast shadows
                if(vpObjects[c]->GetType() == CObject::eType::LIGHT)
                    continue;
                
                // Check if ray going from intersection point to the light is obscured by any object
                if(vpObjects[c]->GetCollision()->GetIntersection(_ShadowTestRay) >= 0  )
                {
                    if(vpObjects[c]->GetCastShadows() == false)
                        continue;
                    
                    // Calculate distances
                    float fDistanceToLight  = CVector3::distance(vpLights[b]->GetPosition(),vWorldSpaceIntersectionPosition);
                    float fDistanceToObject = CVector3::distance(vpObjects[c]->GetPosition(),vWorldSpaceIntersectionPosition);
             
                    // Check if the distance to the object is closer to the light
                    if(fDistanceToLight > fDistanceToObject)
                    {
                        fShadeFactor = 0.0;
                    }
                }
            }
            
            // Add ambient color
            vAccumulatedPixelColor += pIntersectedObject->GetMaterial()->GetColor() * m_Scene.GetAmbient();
          
            
            
                // Length between light and intersected position
                CVector3 L          = vpLights[b]->GetPosition() - vWorldSpaceIntersectionPosition;
                L.normalize();
            
                // Normal of the intersected position
                CVector3 N          = pIntersectedObject->GetCollision()->GetNormal( vWorldSpaceIntersectionPosition );
            
                // Does have a diffuse component?
                if (pIntersectedObject->GetMaterial()->GetDiffuse() > 0.0f)
                {
                    //
                    float dot = CVector3::dot( N, L );
                
                    // test for 'dot > 0' prevents faces that are turned away from the light source get lit.
                    if (dot > 0.0f)
                    {
                        //
                        float diff = dot * pIntersectedObject->GetMaterial()->GetDiffuse() * fShadeFactor;
                    
                        // add diffuse component to ray color
                        vAccumulatedPixelColor += diff * pIntersectedObject->GetMaterial()->GetColor() * vpLights[b]->GetMaterial()->GetColor();
                        
                    }
                }
            
                // determine specular component
                if (pIntersectedObject->GetMaterial()->GetSpecular() > 0)
                {
                    // point light source: sample once for specular highlight
                    CVector3 V = _Ray.GetDirection();
                    CVector3 R = L - 2.0f * CVector3::dot( L, N ) * N;
                    float dot = CVector3::dot( V, R );
                    if (dot > 0)
                    {
                        //
                        float spec = powf( dot, 20 ) * pIntersectedObject->GetMaterial()->GetSpecular() * fShadeFactor;
                    
                        // add specular component to ray color
                        vAccumulatedPixelColor += spec * vpLights[b]->GetMaterial()->GetColor() ;
                        
                    }
                }
        }
        
        
        
        // Compute Reflection (goes recursion)
        float fReflection = pIntersectedObject->GetMaterial()->GetReflection();
        if(fReflection > 0.0f)
        {
            CVector3 N = pIntersectedObject->GetCollision()->GetNormal( vWorldSpaceIntersectionPosition );
            CVector3 R = _Ray.GetDirection() - 2.0f * CVector3::dot( _Ray.GetDirection(), N ) * N;
                    
            //
            if (iTraceDepth < m_iRayTraceDepth)
            {
                CRay _NewRay;
                _NewRay.Set(vWorldSpaceIntersectionPosition + R * EPSILON, R);
                        
                CVector3 vReflectionColor(0,0,0);
                ShootRay( _NewRay, iTraceDepth, vReflectionColor );
                        
                vAccumulatedPixelColor += fReflection * vReflectionColor * pIntersectedObject->GetMaterial()->GetColor();
                
                
                iTraceDepth++;
            }
        }
        
    }

}
CScene *CRayTracer::GetScene()
{
    return &m_Scene;
}
void CRayTracer::SaveBMP(const char *filename, int w, int h, int dpi, CVector3 *data)
{
    FILE *f;
    int k = w*h;
    int s = 4*k;
    int filesize = 54 + s;
    
    double factor = 39.375;
    int m = static_cast<int>(factor);
    
    int ppm = dpi*m;
    
    unsigned char bmpfileheader[14] = {'B','M', 0,0,0,0, 0,0,0,0, 54,0,0,0};
    unsigned char bmpinfoheader[40] = {40,0,0,0, 0,0,0,0, 0,0,0,0, 1,0,24,0};
    
    bmpfileheader[ 2] = (unsigned char)(filesize);
    bmpfileheader[ 3] = (unsigned char)(filesize>>8);
    bmpfileheader[ 4] = (unsigned char)(filesize>>16);
    bmpfileheader[ 5] = (unsigned char)(filesize>>24);
    
    bmpinfoheader[ 4] = (unsigned char)(w);
    bmpinfoheader[ 5] = (unsigned char)(w>>8);
    bmpinfoheader[ 6] = (unsigned char)(w>>16);
    bmpinfoheader[ 7] = (unsigned char)(w>>24);
    
    bmpinfoheader[ 8] = (unsigned char)(h);
    bmpinfoheader[ 9] = (unsigned char)(h>>8);
    bmpinfoheader[10] = (unsigned char)(h>>16);
    bmpinfoheader[11] = (unsigned char)(h>>24);
    
    bmpinfoheader[21] = (unsigned char)(s);
    bmpinfoheader[22] = (unsigned char)(s>>8);
    bmpinfoheader[23] = (unsigned char)(s>>16);
    bmpinfoheader[24] = (unsigned char)(s>>24);
    
    bmpinfoheader[25] = (unsigned char)(ppm);
    bmpinfoheader[26] = (unsigned char)(ppm>>8);
    bmpinfoheader[27] = (unsigned char)(ppm>>16);
    bmpinfoheader[28] = (unsigned char)(ppm>>24);
    
    bmpinfoheader[29] = (unsigned char)(ppm);
    bmpinfoheader[30] = (unsigned char)(ppm>>8);
    bmpinfoheader[31] = (unsigned char)(ppm>>16);
    bmpinfoheader[32] = (unsigned char)(ppm>>24);
    
    f = fopen(filename,"wb");
    
    fwrite(bmpfileheader,1,14,f);
    fwrite(bmpinfoheader,1,40,f);
    
    for (int i = 0; i < k; i++)
    {
        CVector3 rgb = data[i];
        int R = rgb.x * 255;
        int G = rgb.y * 255;
        int B = rgb.z * 255;
        
        if(R>255)R=255;
        if(G>255)G=255;
        if(B>255)B=255;
        if(R<0) R = 0;
        if(G<0) G= 0;
        if(B<0) B =0 ;
        
        unsigned char color[3];
        color[0] = static_cast<unsigned char>(R);
        color[1] = static_cast<unsigned char>(G);
        color[2] = static_cast<unsigned char>(B); // on mac x/z is swapped?

        fwrite(color,1,3,f);
    }
    
    fclose(f);
}
CRayTracer::~CRayTracer()
{
    
}
