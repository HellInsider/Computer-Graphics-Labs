#include "u_spheresgrid.h"
#include "anim.h"

spheres_grid::spheres_grid(anim* Ani, const vec& pos, INT gridSize) : unit(), gridSize(gridSize)
{
    for (int i = 0; i < gridSize; i++) 
    {
        for (int j = 0; j < gridSize; j++) 
        {
          material* mtl = Ani->AddMaterial(std::to_string(i) + "_" + std::to_string(j),
            color3(0.5), 1.0 / (gridSize + 1) * (i + 0.5), 1.0 / (gridSize + 1) * (j + 0.5));
            spheres.Prims.push_back(new prim(topology::sphere::sphere(pos + vec(offset * i, offset * j, 1), radius)));
            spheres.Prims[i * gridSize + j]->Mtl = mtl;
        }
    }
}

spheres_grid::~spheres_grid() 
{
}

VOID spheres_grid::Response(anim* Ani)
{
}

VOID spheres_grid::Render(anim* Ani)
{
    spheres.Draw(matr());
}