#include "U_RT_SCENE.H"
#include "ANIM.H"

ragl::rt_scene::rt_scene(ragl::anim* Ani) : unit()
{
  spheres[0] = new prim(topology::sphere(vec(-5, 0.5, 2), 0.7));
  spheres[1] = new prim(topology::sphere(vec(-5, 1.3, 2), 0.5));
  spheres[2] = new prim(topology::sphere(vec(-5, 1.9, 2), 0.3));

  spheres[3] = new prim(topology::sphere(vec(3, 0.5, 3), 0.3));
  spheres[4] = new prim(topology::sphere(vec(0, 3, 0), 0.2));
  spheres[5] = new prim(topology::sphere(vec(2, 2, 0), 0.7));
  spheres[6] = new prim(topology::sphere(vec(0, 0.4, 1.4), 1.3));

  cube = new prim(topology::box::box(vec(-1), vec(2)));
  
  planes[0] = new prim(topology::plane(vec(-100, -0.5, -100), vec(200, 0, 0), vec(0, 0, 200), 4, 4).EvalNormals());
  planes[1] = new prim(topology::plane(vec(85.15, -0.5, -100), vec(-200, 0, 200), vec(0, 100, 0), 4, 4).EvalNormals());
  
  ragl::shader* shd = Ani->Shaders.Find("def");
  
  ragl::material* mtl = Ani->AddMaterial("snow", vec(0.05), vec(1), vec(0.4), -10, 0.0, shd);
  spheres[0]->Mtl = spheres[1]->Mtl = spheres[2]->Mtl = mtl;
  mtl = Ani->AddMaterial("sphere_def", vec(0.05), vec(0.7, 0.0, 0.8), vec(0.6, 1.0, 1.0), 75, 0.0, shd);
  spheres[5]->Mtl = spheres[6]->Mtl = mtl;
  mtl = Ani->AddMaterial("sphere_blue", vec(0.05), vec(0.0, 0.1, 1.0), vec(0.6, 1.0, 1.0), 75, 0.0, shd);
  spheres[4]->Mtl = mtl;
  mtl = Ani->AddMaterial("sphere_yellow", vec(0.05), vec(1.0, 1.0, 0.0), vec(0.6, 1.0, 1.0), 75, 0.0, shd);
  spheres[3]->Mtl = mtl;
  
  mtl = Ani->AddMaterial("cube", vec(0.05), vec(0.0, 0.8, 0.7), vec(0.2, 0.2, 1.0), -10, 0.0, shd);
  cube->Mtl = mtl;
  
  mtl = Ani->AddMaterial("plane", vec(0.05), vec(0.01), vec(0.5, 0.5, 1.0), -20, 0.0, shd);
  planes[0]->Mtl = planes[1]->Mtl = mtl;
  
}

VOID ragl::rt_scene::Response(ragl::anim* Ani)
{

}

VOID ragl::rt_scene::Render(ragl::anim* Ani)
{
  planes[0]->Draw(matr());
  planes[1]->Draw(matr());
  cube->Draw(matr());

  for (int i = 0; i < 7; i++)
    spheres[i]->Draw(matr());
}
