#pragma once

#include <fstream>
#include <algorithm>
#include <sstream>
#include <iostream>
#include <cmath>
#include <string>
#include <chrono>
#include <vector>
#include <array>
#include <list>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// include imgui
#include "lib/imgui/imgui.h"
#include "lib/imgui/backends/imgui_impl_glfw.h"
#include "lib/imgui/backends/imgui_impl_opengl3.h"



class Mat4;

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
	Vec3d operator+(Vec3d v){
		return Vec3d(x + v.x, y + v.y, z + v.z);
	}
	Vec3d operator-(Vec3d v) {
		return Vec3d(x - v.x, y - v.y, z - v.z);
	}

	//multiplication of vector by scalar
	Vec3d operator*(float k) {
		return Vec3d(x * k, y * k, z * k);
	}

	//division of vector by scalar
	Vec3d operator/(float k) {
		return Vec3d(x / k, y / k, z / k);
	}

	//dot product
	float dot_product(Vec3d v) {
		return x * v.x + y * v.y + z * v.z;
	}

	//length
	float vec_length(Vec3d v) {
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

class Mat4 {
public:
	float m[4][4] = { 0.0f };
	Mat4() = default;

	static Mat4 makeIdentity() {
		Mat4 m;
		for (int i = 0; i < 4; i++) {
			m.m[i][i] = 1.0f;
		}
		return m;
	}

	static Mat4 makeRotationX(float fAngleRad){
		Mat4 matrix;
		matrix.m[0][0] = 1.0f;
		matrix.m[1][1] = cosf(fAngleRad);
		matrix.m[1][2] = sinf(fAngleRad);
		matrix.m[2][1] = -sinf(fAngleRad);
		matrix.m[2][2] = cosf(fAngleRad);
		matrix.m[3][3] = 1.0f;
		return matrix;
	}

	static Mat4 makeRotationY(float fAngleRad){
		Mat4 matrix;
		matrix.m[0][0] = cosf(fAngleRad);
		matrix.m[0][2] = sinf(fAngleRad);
		matrix.m[2][0] = -sinf(fAngleRad);
		matrix.m[1][1] = 1.0f;
		matrix.m[2][2] = cosf(fAngleRad);
		matrix.m[3][3] = 1.0f;
		return matrix;
	}

	static Mat4 makeRotationZ(float fAngleRad){
		Mat4 matrix;
		matrix.m[0][0] = cosf(fAngleRad);
		matrix.m[0][1] = sinf(fAngleRad);
		matrix.m[1][0] = -sinf(fAngleRad);
		matrix.m[1][1] = cosf(fAngleRad);
		matrix.m[2][2] = 1.0f;
		matrix.m[3][3] = 1.0f;
		return matrix;
	}

	static Mat4 makeTranslation(float x, float y, float z){
		Mat4 matrix;
		matrix.m[0][0] = 1.0f;
		matrix.m[1][1] = 1.0f;
		matrix.m[2][2] = 1.0f;
		matrix.m[3][3] = 1.0f;
		matrix.m[3][0] = x;
		matrix.m[3][1] = y;
		matrix.m[3][2] = z;
		return matrix;
	}

	static Mat4 makeProjection(float fFovDegrees, float fAspectRatio, float fNear, float fFar){
		float fFovRad = 1.0f / tanf(fFovDegrees * 0.5f / 180.0f * 3.14159f);
		Mat4 matrix;
		matrix.m[0][0] = fAspectRatio * fFovRad;
		matrix.m[1][1] = fFovRad;
		matrix.m[2][2] = fFar / (fFar - fNear);
		matrix.m[3][2] = (-fFar * fNear) / (fFar - fNear);
		matrix.m[2][3] = 1.0f;
		matrix.m[3][3] = 0.0f;
		return matrix;
	}



	Vec3d operator*(Vec3d i)
	{
		Vec3d v;
		v.x = i.x * m[0][0] + i.y * m[1][0] + i.z * m[2][0] + i.w * m[3][0];
		v.y = i.x * m[0][1] + i.y * m[1][1] + i.z * m[2][1] + i.w * m[3][1];
		v.z = i.x * m[0][2] + i.y * m[1][2] + i.z * m[2][2] + i.w * m[3][2];
		v.w = i.x * m[0][3] + i.y * m[1][3] + i.z * m[2][3] + i.w * m[3][3];
		return v;
	}
	

	Mat4 operator*(Mat4 m2){
		Mat4 matrix;
		for (int c = 0; c < 4; c++)
			for (int r = 0; r < 4; r++)
				matrix.m[r][c] = m[r][0] * m2.m[0][c] + m[r][1] * m2.m[1][c] + m[r][2] * m2.m[2][c] + m[r][3] * m2.m[3][c];
		return matrix;
	}


	static Mat4 pointAt(Vec3d pos, Vec3d target, Vec3d up){
		// Calculate new forward direction
		Vec3d newForward = target - pos;
		newForward = newForward.normalise();

		// Calculate new Up direction
		Vec3d a = newForward * up.dot_product(newForward);
		Vec3d newUp = up - a;
		newUp = newUp.normalise();

		// New Right direction is easy, its just cross product
		Vec3d newRight = newUp.cross_product(newForward);

		// Construct Dimensioning and Translation Matrix	
		Mat4 matrix;
		matrix.m[0][0] = newRight.x;	matrix.m[0][1] = newRight.y;	matrix.m[0][2] = newRight.z;	matrix.m[0][3] = 0.0f;
		matrix.m[1][0] = newUp.x;		matrix.m[1][1] = newUp.y;		matrix.m[1][2] = newUp.z;		matrix.m[1][3] = 0.0f;
		matrix.m[2][0] = newForward.x;	matrix.m[2][1] = newForward.y;	matrix.m[2][2] = newForward.z;	matrix.m[2][3] = 0.0f;
		matrix.m[3][0] = pos.x;			matrix.m[3][1] = pos.y;			matrix.m[3][2] = pos.z;			matrix.m[3][3] = 1.0f;
		return matrix;

	}


	Mat4 quickInverse(){
		Mat4 matrix;
		matrix.m[0][0] = m[0][0]; matrix.m[0][1] = m[1][0]; matrix.m[0][2] = m[2][0]; matrix.m[0][3] = 0.0f;
		matrix.m[1][0] = m[0][1]; matrix.m[1][1] = m[1][1]; matrix.m[1][2] = m[2][1]; matrix.m[1][3] = 0.0f;
		matrix.m[2][0] = m[0][2]; matrix.m[2][1] = m[1][2]; matrix.m[2][2] = m[2][2]; matrix.m[2][3] = 0.0f;
		matrix.m[3][0] = -(m[3][0] * matrix.m[0][0] + m[3][1] * matrix.m[1][0] + m[3][2] * matrix.m[2][0]);
		matrix.m[3][1] = -(m[3][0] * matrix.m[0][1] + m[3][1] * matrix.m[1][1] + m[3][2] * matrix.m[2][1]);
		matrix.m[3][2] = -(m[3][0] * matrix.m[0][2] + m[3][1] * matrix.m[1][2] + m[3][2] * matrix.m[2][2]);
		matrix.m[3][3] = 1.0f;
		return matrix;
	}

};


class Camera {
public:
	Vec3d pos;	// Location of camera in world space
	Vec3d lookDir;	// Direction vector along the direction camera points
	float fYaw;		// FPS Camera rotation in XZ plane
	float fPitch;	// FPS Camera rotation in YZ plane

	Camera() = default;
	Camera(Vec3d pos) : pos(pos) {
		//look at z by default
		lookDir = Vec3d(0, 0, 1);
		fYaw = 0;
		fPitch = 0;
	}


	Mat4 matView() {
		// Create "Point At" Matrix for camera
		Vec3d vTarget = { 0,0,1 };
		Mat4 matRotX = Mat4::makeRotationX(fPitch);
		Mat4 matRotY = Mat4::makeRotationY(fYaw);
		Mat4 matCameraRot = matRotX * matRotY;
		lookDir = matCameraRot * vTarget;
		vTarget = pos + lookDir;
		Mat4 matCamera = Mat4::pointAt(pos, vTarget, { 0,1,0 });
		return matCamera.quickInverse();
	}
};