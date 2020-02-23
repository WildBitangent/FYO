#pragma once 
#include "Model.hpp"

struct LensTransform
{
	Model& model;
	DirectX::XMMATRIX transform;
};

class LensDatabase
{
public:
	static void loadModel(const std::string& path);
	static Model& get(size_t i);
	
private:
	static std::vector<Model> mLens;
};