#pragma once
#include "Util.hpp"
#include "Model.hpp"

class Logic
{
public:
	Logic();
	
	void update(float dt);

private:
	Model mImageModel;
	Model mLensModel;

	Buffer mLensVertices;
	Buffer mImageVertices;

	Texture mPlaneTexture;
	Buffer mPlaneTextCoords;
};