#pragma once
#include "ObjectPBR.h"

// clase para escena, cargar objetos
class Scene
{
public:
    bool LoadAll(GLuint shaderPBR);

    // Objetos 
    ObjectPBR arcoC;
    ObjectPBR paredC;
    ObjectPBR suelo;
    ObjectPBR rocaS;
    ObjectPBR paredD;
    ObjectPBR paredI;
    ObjectPBR arthur;
    ObjectPBR boneLeg;
    ObjectPBR skullLamb;
    ObjectPBR skullDeer;
    ObjectPBR boneArm;
};
