#pragma once

#include <fstream>
#include <algorithm>
#include <sstream>
#include <iostream>
#include <cmath>
#include <string>
#include <chrono>
#include <vector>
#include <list>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>


class Vec3d{
	public:
	float x = 0;
	float y = 0;
	float z = 0;
	float w = 1; // Need a 4th term to perform sensible matrix vector multiplication

	Vec3d() = default;
	Vec3d(float x, float y, float z) : x(x), y(y), z(z) {}
	Vec3d(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {}


	//addition and subtraction of vectors
	Vec3d operator+(const Vec3d& v) const {
		return Vec3d(x + v.x, y + v.y, z + v.z);
	}
	Vec3d operator-(const Vec3d& v) const {
		return Vec3d(x - v.x, y - v.y, z - v.z);
	}

	//multiplication of vector by scalar
	Vec3d operator*(const float& k) const {
		return Vec3d(x * k, y * k, z * k);
	}

	//division of vector by scalar
	Vec3d operator/(const float& k) const {
		return Vec3d(x / k, y / k, z / k);
	}

	//dot product
	float dot_product(const Vec3d& v) const {
		return x * v.x + y * v.y + z * v.z;
	}

	//length
	float vec_length(Vec3d& v) {
		return sqrt(dot_product(v));
	}

	//normalise
	Vec3d normalise() {
		float l = vec_length(*this);
		return *this / l;
	}

	//cross product
	Vec3d cross_product(const Vec3d& v) const {
		return Vec3d(y * v.z - z * v.y, z * v.x - x * v.z, x * v.y - y * v.x);
	}

};

class Triangle {
public:
	Vec3d p[3];
	float col;
};

class Mesh {
public:
	std::vector<Triangle> tris;

	bool LoadFromObjectFile(std::string sFilename)
	{
		std::ifstream f(sFilename);
		if (!f.is_open())
			return false;

		// Local cache of verts
		std::vector<Vec3d> verts;

		while (!f.eof())
		{
			char line[128];
			f.getline(line, 128);

			std::stringstream s;
			s << line;

			char junk;

			if (line[0] == 'v')
			{
				Vec3d v;
				s >> junk >> v.x >> v.y >> v.z;
				verts.push_back(v);
			}

			if (line[0] == 'f')
			{
				int f[3];
				s >> junk >> f[0] >> f[1] >> f[2];
				tris.push_back({ verts[f[0] - 1], verts[f[1] - 1], verts[f[2] - 1] });
			}
		}
		return true;
	}
};

class mat4x4 {
public:
	float m[4][4] = { 0.0f };
};