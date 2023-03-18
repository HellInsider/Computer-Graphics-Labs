#include "u_sky.h"
#include "anim.h"

sky::sky( anim *Ani ) : unit(),
                                    sph(topology::box::box(vec(-100), vec(100)))

{
  material *mtl = Ani->Materials.Find("sky");
  if (mtl == NULL)
  {
    shader *shd = Ani->Shaders.Find("sky");
    if (shd == NULL)
    {
      shd = Ani->AddShader("shaders/sky");
    }
    mtl = Ani->AddMaterial("sky", vec(0.5, 0.5, 0.5), 0.5, 0.5, 0, shd);
    mtl->Tex[0] = Ani->AddTexture("BIN\\Textures\\cube1\\*.jpg", TRUE, TRUE);
  }
  sph.Mtl = mtl;
}

VOID sky::Response( anim *Ani )
{
  sph.Transform = matr::MatrTranslation(Ani->Cam.GetLoc());
}

VOID sky::Render( anim *Ani )
{
  Ani->SetCulling(false);
  sph.Draw(matr());     
  Ani->SetCulling(true);
}

