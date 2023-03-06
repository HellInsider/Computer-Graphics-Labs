#include "u_sky.h"
#include "anim.h"

sky::sky( anim *Ani ) : unit(),
                                    sph(topology::sphere::sphere(vec(0), 100, 50, 23))

{
  /*std::vector<BYTE> img((std::istreambuf_iterator<char>(std::ifstream("bin/textures/Skyoba.g32"))),
                        std::istreambuf_iterator<char>());
  INT
    w = img[0] + (img[1] << 8),
    h = img[2] + (img[3] << 8);*/
  material *mtl = Ani->Materials.Find("sky");
  if (mtl == NULL)
  {
    shader *shd = Ani->Shaders.Find("sky");
    if (shd == NULL)
    {
      shd = Ani->AddShader("shaders/sky");
    }
    //mtl = Ani->AddMaterial("sky", vec(0.1, 0.6, 0.8), vec(0.1), vec(0), 0, 0, shd);
    //mtl->Tex[0] = Ani->AddTexture("bin/textures/Skyoba.g32", w, h, 4, &img[4]);
  }
  sph.Mtl = mtl;
}

VOID sky::Response( anim *Ani )
{

}

VOID sky::Render( anim *Ani )
{
  ///glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  sph.Draw(matr());     
}

