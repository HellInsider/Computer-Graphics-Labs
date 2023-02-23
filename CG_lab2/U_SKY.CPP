/* FILE NAME   : U_SKY.CPP
 * PURPOSE     : sky class implementation module
 * PROGRAMMER  : AK5a.
 * LAST UPDATE : 25.07.2017
 */

#include "u_sky.h"
#include "anim.h"

/* Class constructor.
 * ARGUMENT:
 *   - animation interface pointer:
 *       ragl::anim *Ani;
 */
ragl::sky::sky( ragl::anim *Ani ) : unit(),
                                    sph(topology::sphere::sphere(vec(0), 100, 50, 23))

{
  /*std::vector<BYTE> img((std::istreambuf_iterator<char>(std::ifstream("bin/textures/Skyoba.g32"))),
                        std::istreambuf_iterator<char>());
  INT
    w = img[0] + (img[1] << 8),
    h = img[2] + (img[3] << 8);*/
  ragl::material *mtl = Ani->Materials.Find("sky");
  if (mtl == NULL)
  {
    ragl::shader *shd = Ani->Shaders.Find("sky");
    if (shd == NULL)
    {
      shd = Ani->AddShader("shaders/sky");
    }
    mtl = Ani->AddMaterial("sky", vec(0.1, 0.6, 0.8), vec(0.1), vec(0), 0, 0, shd);
    //mtl->Tex[0] = Ani->AddTexture("bin/textures/Skyoba.g32", w, h, 4, &img[4]);
  }
  sph.Mtl = mtl;
}

/* Interframe animation response function.
 * ARGUMENT:
 *   - animation interface pointer:
 *       anim *Ani;
 * RETURNS: None.
 */
VOID ragl::sky::Response( ragl::anim *Ani )
{

}

/* Render unit function.
 * ARGUMENT:
 *   - animation interface pointer:
 *       anim *Ani;
 * RETURNS: None.
 */
VOID ragl::sky::Render( ragl::anim *Ani )
{
  ///glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  sph.Draw(matr());     
}

/* END OF 'U_SKY.CPP' FILE */