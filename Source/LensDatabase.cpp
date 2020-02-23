#include "LensDatabase.hpp"

std::vector<Model> LensDatabase::mLens;

void LensDatabase::loadModel(const std::string& path)
{
	mLens.emplace_back(path);
}

Model& LensDatabase::get(size_t i)
{
	return mLens[i];
}
