#include "u_ground.h"
#include "../../Anim/anim.h"

ground::ground( anim *Ani ) : unit(),
                                          sph(topology::plane::plane(vec(-100, 0, -100), vec(200, 0, 0), vec(0, 0, 200), 4, 4).EvalNormals())
{
  material* mtl = Ani->Materials.Find("ground");
  if (mtl == NULL)
  {
    shader* shd = Ani->Shaders.Find("def");

    mtl = Ani->AddMaterial("ground", vec(0.05, 0.3, 0.0), 0.99, 0.0, 0, shd);
  }
  sph.Mtl = mtl;

}

VOID ground::Response( anim *Ani )
{

}

VOID ground::Render( anim *Ani )
{
  ///glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  sph.Draw(matr());
}
