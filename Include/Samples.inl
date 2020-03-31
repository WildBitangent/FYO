﻿#include "Util.hpp"

void initSamples(std::vector<Sample>& samples)
{
	samples.emplace_back(Sample{
		"Empty",
		[](Lens& lens) { lens.clear(); }
	});

	samples.emplace_back(Sample{
		"Default",
		[](Lens& lens) {
			lens.clear();
			lens.create(LensStruct{.radius1 = 10.f, .type = LensType::PLANOCONVEX, .center = { 0, 3, 25 }, .width = 1.5f, .dimensions = { 5, 7 } });
			lens.create(LensStruct{.radius1 = 5.f, .radius2 = 5.f, .type = LensType::BICONCAVE, .center = { 0, 3, 15 }, .width = 1.5f, .dimensions = { 5, 7 } });
			lens.create(LensStruct{.radius1 = 10.f, .radius2 = 10.f, .type = LensType::BICONVEX, .center = { 0, 3, 7 }, .width = 1.5f, .dimensions = { 5, 7 } });
		}
	});
}