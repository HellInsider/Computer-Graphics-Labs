#include "u_spheresgrid.h"
#include "anim.h"

spheresGrid::spheresGrid(anim* Ani, const vec& pos, INT gridSize) : unit(), gridSize(gridSize)
{
    material* mtl = Ani->Materials.Find("default");

    //spheres = new std::vector<primitives>();
    for (int i = 0; i < gridSize; i++) 
    {
        for (int j = 0; j < gridSize; j++) 
        {
            spheres.Prims.push_back(new prim(topology::sphere::sphere(pos + vec(offset * i, offset * j, 1), radius)));
            spheres.Prims[i * gridSize + j]->Mtl = mtl;
        }
    }
}

spheresGrid::~spheresGrid() 
{
    //spheres.clear();    //IDD: Do I need this? Don't Understand how you clear units.
    for (int i = 0; i < gridSize; i++)
    {
        for (int j = 0; j < gridSize; j++)
        {
            delete spheres.Prims[i * gridSize + j];
        }
    }
    spheres.Prims.clear();
}

VOID spheresGrid::Response(anim* Ani)
{
}

VOID spheresGrid::Render(anim* Ani)
{
    for (int i = 0; i < gridSize; i++)
    {
        for (int j = 0; j < gridSize; j++)
        {
            spheres.Prims[i * gridSize + j]->Draw(matr());
        }
    }
    //sph.Draw(matr::MatrTranslation(Pos[0]));
    //trees.Draw(matr::MatrTranslation(Pos[1]));
}