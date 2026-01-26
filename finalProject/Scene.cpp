#include "Scene.h"
#include <iostream>

bool Scene::LoadAll(GLuint shaderPBR)
{//Load complete scene objects
    bool ok = true;

    ok &= arcoC.Load("assets/arcoCastillo.obj", shaderPBR,
        "textures/arcoCastillo/arcoCastillo_baseColor.jpg",
        "textures/arcoCastillo/arcoCastillo_normal.jpg",
        "textures/arcoCastillo/arcoCastillo_roughness.jpg",
        "textures/arcoCastillo/arcoCastillo_AO.jpg");

    ok &= paredC.Load("assets/paredCastillo.obj", shaderPBR,
        "textures/paredCastillo/paredCastillo_baseColor.jpg",
        "textures/paredCastillo/paredCastillo_normal.jpg",
        "textures/paredCastillo/paredCastillo_roughness.jpg",
        "textures/paredCastillo/paredCastillo_AO.jpg");

    ok &= suelo.Load("assets/suelo.obj", shaderPBR,
        "textures/suelo/suelo_baseColor.jpg",
        "textures/suelo/suelo_normal.jpg",
        "textures/suelo/suelo_roughness.jpg",
        "textures/suelo/suelo_AO.jpg");

    ok &= rocaS.Load("assets/rocaSuelo.obj", shaderPBR,
        "textures/rocaSuelo/rocaSuelo_baseColor.jpg",
        "textures/rocaSuelo/rocaSuelo_normal.jpg",
        "textures/rocaSuelo/rocaSuelo_roughness.jpg",
        "textures/rocaSuelo/rocaSuelo_AO.jpg");

    ok &= paredD.Load("assets/paredDer.obj", shaderPBR,
        "textures/paredes/paredes_baseColor.jpg",
        "textures/paredes/paredes_normal.jpg",
        "textures/paredes/paredes_roughness.jpg",
        "textures/paredes/paredes_AO.jpg");

    ok &= paredI.Load("assets/paredIzq.obj", shaderPBR,
        "textures/paredes/paredes_baseColor.jpg",
        "textures/paredes/paredes_normal.jpg",
        "textures/paredes/paredes_roughness.jpg",
        "textures/paredes/paredes_AO.jpg");

    // Arthur SIN AO
    ok &= arthur.Load("assets/arthur.obj", shaderPBR,
        "textures/arthur/arthur_baseColor.jpg",
        "textures/arthur/arthur_normal.jpg",
        "textures/arthur/arthur_roughness.jpg",
        "");

    ok &= boneLeg.Load("assets/boneLeg.obj", shaderPBR,
        "textures/boneLeg/boneLeg_baseColor.jpg",
        "textures/boneLeg/boneLeg_normal.jpg",
        "textures/boneLeg/boneLeg_roughness.jpg",
        "textures/boneLeg/boneLeg_AO.jpg");

    ok &= skullLamb.Load("assets/skullLamb.obj", shaderPBR,
        "textures/skullLamb/skullLamb_baseColor.jpg",
        "textures/skullLamb/skullLamb_normal.jpg",
        "textures/skullLamb/skullLamb_roughness.jpg",
        "textures/skullLamb/skullLamb_AO.jpg");

    ok &= skullDeer.Load("assets/skullDeer.obj", shaderPBR,
        "textures/skullDeer/skullDeer_baseColor.jpg",
        "textures/skullDeer/skullDeer_normal.jpg",
        "textures/skullDeer/skullDeer_roughness.jpg",
        "textures/skullDeer/skullDeer_AO.jpg");

    ok &= boneArm.Load("assets/boneArm.obj", shaderPBR,
        "textures/boneArm/boneArm_baseColor.jpg",
        "textures/boneArm/boneArm_normal.jpg",
        "textures/boneArm/boneArm_roughness.jpg",
        "textures/boneArm/boneArm_AO.jpg");

    return ok;
}
