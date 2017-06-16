//----------------------------------------------------------------------------------------
//
//	siv::PerlinNoise
//	Perlin noise library for modern C++
//
//	Copyright (C) 2013-2016 Ryo Suzuki <reputeless@gmail.com>
//
//	Permission is hereby granted, free of charge, to any person obtaining a copy
//	of this software and associated documentation files(the "Software"), to deal
//	in the Software without restriction, including without limitation the rights
//	to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
//	copies of the Software, and to permit persons to whom the Software is
//	furnished to do so, subject to the following conditions :
//
//	The above copyright notice and this permission notice shall be included in
//	all copies or substantial portions of the Software.
//
//	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
//	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
//	THE SOFTWARE.
//
//----------------------------------------------------------------------------------------

# pragma once
# include <cstdint>
# include <numeric>
# include <algorithm>
# include <random>
namespace aly {
class PerlinNoise {
private:
	std::int32_t p[512];
	double fade(double t) const noexcept;
	double lerp(double t, double a, double b) const noexcept;
	double grad(std::int32_t hash, double x, double y, double z) const noexcept;

public:

	explicit PerlinNoise(std::uint32_t seed =
			std::default_random_engine::default_seed) {
		reseed(seed);
	}

	void reseed(std::uint32_t seed);
	double noise(double x) const;
	double noise(double x, double y) const;
	double noise(double x, double y, double z) const;
	double octaveNoise(double x, std::int32_t octaves) const;
	double octaveNoise(double x, double y, std::int32_t octaves) const;
	double octaveNoise(double x, double y, double z,
			std::int32_t octaves) const;
	double noise0_1(double x) const;
	double noise0_1(double x, double y) const;
	double noise0_1(double x, double y, double z) const;
	double octaveNoise0_1(double x, std::int32_t octaves) const;
	double octaveNoise0_1(double x, double y, std::int32_t octaves) const;
	double octaveNoise0_1(double x, double y, double z,
			std::int32_t octaves) const;
};
}
