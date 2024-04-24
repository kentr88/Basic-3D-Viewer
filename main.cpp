/*
OneLoneCoder.com - 3D Graphics Part #3 - Cameras & Clipping
"Tredimensjonal Grafikk" - @Javidx9

License
~~~~~~~
One Lone Coder Console Game Engine  Copyright (C) 2018  Javidx9
This program comes with ABSOLUTELY NO WARRANTY.
This is free software, and you are welcome to redistribute it
under certain conditions; See license for details.
Original works located at:
https://www.github.com/onelonecoder
https://www.onelonecoder.com
https://www.youtube.com/javidx9
GNU GPLv3
https://github.com/OneLoneCoder/videos/blob/master/LICENSE

From Javidx9 :)
~~~~~~~~~~~~~~~
Hello! Ultimately I don't care what you use this for. It's intended to be
educational, and perhaps to the oddly minded - a little bit of fun.
Please hack this, change it and use it in any way you see fit. You acknowledge
that I am not responsible for anything bad that happens as a result of
your actions. However this code is protected by GNU GPLv3, see the license in the
github repo. This means you must attribute me if you use it. You can view this
license here: https://github.com/OneLoneCoder/videos/blob/master/LICENSE
Cheers!

Background
~~~~~~~~~~
3D Graphics is an interesting, visually pleasing suite of algorithms. This is the
first video in a series that will demonstrate the fundamentals required to 
build your own software based 3D graphics systems.

Video
~~~~~
https://youtu.be/ih20l3pJoeU
https://youtu.be/XgMWc6LumG4
https://youtu.be/HXSuNxpCzdM

Author
~~~~~~
Twitter: @javidx9
Blog: http://www.onelonecoder.com
Discord: https://discord.gg/WhwHUMV



Last Updated: 14/08/2018
*/


/*
This is a 3D Graphics engine that has been adapted from working in the console to basic openGL using GLFW3. It is based on the work of Javidx9.

Compile
g++ -o run main.cpp -lglfw3 -lkernel32 -lopengl32 -lglu32 -lglew32 -Wall -lwinmm -Werror -pedantic

Last Update: 24/04/2025

*/

#include "header.h"


using namespace std;

void errorCallback(int error, const char* description) {
  	std::cerr << "Error: " << description << std::endl;
}

class GameEngine3D{
private:
	Mesh meshCube;
	Mat4 matProj;	// Matrix that converts from view space to screen space
	Camera camera = Camera(Vec3d(0, 0, -5));
	int windowWidth;
	int windowHeight;
	GLFWwindow* window;
	std::string filename;

	Vec3d Vector_IntersectPlane(Vec3d &plane_p, Vec3d &plane_n, Vec3d &lineStart, Vec3d &lineEnd){
		plane_n = plane_n.normalise();
		float plane_d = -plane_n.dot_product(plane_p);
		float ad = lineStart.dot_product(plane_n);
		float bd = lineEnd.dot_product(plane_n);
		float t = (-plane_d - ad) / (bd - ad);
		Vec3d lineStartToEnd = lineEnd - lineStart;
		Vec3d lineToIntersect = lineStartToEnd * t;
		return lineStart + lineToIntersect;
	}

	int Triangle_ClipAgainstPlane(Vec3d plane_p, Vec3d plane_n, Triangle &in_tri, Triangle &out_tri1, Triangle &out_tri2){
		// Make sure plane normal is indeed normal
		plane_n = plane_n.normalise();

		// Return signed shortest distance from point to plane, plane normal must be normalised
		auto dist = [&](Vec3d &p)
		{
			//Vec3d n = Vector_Normalise(p);
			return (plane_n.x * p.x + plane_n.y * p.y + plane_n.z * p.z - plane_n.dot_product(plane_p));
		};

		// Create two temporary storage arrays to classify points either side of plane
		// If distance sign is positive, point lies on "inside" of plane
		Vec3d* inside_points[3];  int nInsidePointCount = 0;
		Vec3d* outside_points[3]; int nOutsidePointCount = 0;

		// Get signed distance of each point in Triangle to plane
		float d0 = dist(in_tri.p[0]);
		float d1 = dist(in_tri.p[1]);
		float d2 = dist(in_tri.p[2]);

		if (d0 >= 0) { inside_points[nInsidePointCount++] = &in_tri.p[0]; }
		else { outside_points[nOutsidePointCount++] = &in_tri.p[0]; }
		if (d1 >= 0) { inside_points[nInsidePointCount++] = &in_tri.p[1]; }
		else { outside_points[nOutsidePointCount++] = &in_tri.p[1]; }
		if (d2 >= 0) { inside_points[nInsidePointCount++] = &in_tri.p[2]; }
		else { outside_points[nOutsidePointCount++] = &in_tri.p[2]; }

		// Now classify Triangle points, and break the input Triangle into 
		// smaller output triangles if required. There are four possible
		// outcomes...

		if (nInsidePointCount == 0)
		{
			// All points lie on the outside of plane, so clip whole Triangle
			// It ceases to exist

			return 0; // No returned triangles are valid
		}

		if (nInsidePointCount == 3)
		{
			// All points lie on the inside of plane, so do nothing
			// and allow the Triangle to simply pass through
			out_tri1 = in_tri;

			return 1; // Just the one returned original Triangle is valid
		}

		if (nInsidePointCount == 1 && nOutsidePointCount == 2)
		{
			// Triangle should be clipped. As two points lie outside
			// the plane, the Triangle simply becomes a smaller Triangle

			// Copy appearance info to new Triangle
			out_tri1.col =  in_tri.col;

			// The inside point is valid, so keep that...
			out_tri1.p[0] = *inside_points[0];

			// but the two new points are at the locations where the 
			// original sides of the Triangle (lines) intersect with the plane
			out_tri1.p[1] = Vector_IntersectPlane(plane_p, plane_n, *inside_points[0], *outside_points[0]);
			out_tri1.p[2] = Vector_IntersectPlane(plane_p, plane_n, *inside_points[0], *outside_points[1]);

			return 1; // Return the newly formed single Triangle
		}

		if (nInsidePointCount == 2 && nOutsidePointCount == 1)
		{
			// Triangle should be clipped. As two points lie inside the plane,
			// the clipped Triangle becomes a "quad". Fortunately, we can
			// represent a quad with two new triangles

			// Copy appearance info to new triangles
			out_tri1.col =  in_tri.col;

			out_tri2.col =  in_tri.col;

			// The first Triangle consists of the two inside points and a new
			// point determined by the location where one side of the Triangle
			// intersects with the plane
			out_tri1.p[0] = *inside_points[0];
			out_tri1.p[1] = *inside_points[1];
			out_tri1.p[2] = Vector_IntersectPlane(plane_p, plane_n, *inside_points[0], *outside_points[0]);

			// The second Triangle is composed of one of he inside points, a
			// new point determined by the intersection of the other side of the 
			// Triangle and the plane, and the newly created point above
			out_tri2.p[0] = *inside_points[1];
			out_tri2.p[1] = out_tri1.p[2];
			out_tri2.p[2] = Vector_IntersectPlane(plane_p, plane_n, *inside_points[1], *outside_points[0]);

			return 2; // Return two newly formed triangles which form a quad
		}
		return 0;
	}

	void drawTriangle(vector<array<float, 9>> triangles, vector<float> colours){
		for(int i = 0; i < (int) triangles.size(); i++){
			glColor3f(colours[i], colours[i], colours[i]);
			glVertexPointer(3, GL_FLOAT, 0, &triangles[i]);
			glDrawArrays(GL_TRIANGLES, 0, 3);
		}
	}

	bool GraphicsInit(){
		// Load object file
		meshCube.LoadFromObjectFile(filename);

		// Projection Matrix
		matProj = Mat4::makeProjection(90.0f, (float)windowHeight / (float)windowWidth, 0.1f, 1000.0f);
		return true;
	}


public:
	GameEngine3D(int w, int h, string _filename){
		windowWidth = w;
		windowHeight = h;
		filename = _filename;

		// Initialize GLFW
		if (!glfwInit()) {
			std::cerr << "Failed to initialize GLFW" << std::endl;
			exit(-1);
		}

		// Set the error callback
		glfwSetErrorCallback(errorCallback);

		// Create a GLFW window
		window = glfwCreateWindow(windowWidth, windowHeight, "Basic 3D Viewer", NULL, NULL);
		if (!window) {
			std::cerr << "Failed to create GLFW window" << std::endl;
			glfwTerminate();
			exit(-1);
		}

		// Make the window's context current
		glfwMakeContextCurrent(window);

		
		// Create user resources as part of this thread
		if (!GraphicsInit()){
			std::cerr << "Failed on user create" << std::endl;
			exit(-1);
		}
	}

	bool Render(float fElapsedTime){
		Mat4 matWorld = Mat4::makeIdentity();	// Form World Matrix

		// Get view matrix from camera class
		Mat4 matView = camera.matView();

		// Store triagles for rastering later
		vector<Triangle> vecTrianglesToClip;
		vector<array<float, 9>> trianglesToDraw;
		vector<float> coloursToDraw;

		// Draw Triangles
		for (auto tri : meshCube.tris){
			Triangle triProjected, triTransformed, triViewed;

			// World Matrix Transform
			for(int i = 0; i < 3; i++){
				triTransformed.p[i] = matWorld * tri.p[i];
			}

			// Calculate Triangle Normal
			Vec3d normal, line1, line2;

			// Get lines either side of Triangle
			line1 = triTransformed.p[1] - triTransformed.p[0];
			line2 = triTransformed.p[2] - triTransformed.p[0];

			// Take cross product of lines to get normal to Triangle surface
			normal = line1.cross_product(line2);

			// You normally need to normalise a normal!
			normal = normal.normalise();
			
			// Get Ray from Triangle to camera
			Vec3d vCameraRay = triTransformed.p[0] - camera.pos;


			// If ray is aligned with normal, then Triangle is visible
			if (normal.dot_product(vCameraRay) < 0.0f){
				// Illumination
				Vec3d light_direction = { 0.0f, 1.0f, -0.5f };
				light_direction = light_direction.normalise();

				// How "aligned" are light direction and Triangle surface normal?
				float dp = max(0.2f, (float)(light_direction.dot_product(normal) * 1));
				dp = min(dp, 0.85f);

				triTransformed.col = dp;

				// Convert World Space --> View Space
				for(int i = 0; i < 3; i++){
					triViewed.p[i] = matView * triTransformed.p[i];
				}
				triViewed.col = triTransformed.col;

				// Clip Viewed Triangle against near plane, this could form two additional
				// additional triangles. 
				int nClippedTriangles = 0;
				Triangle clipped[2];
				nClippedTriangles = Triangle_ClipAgainstPlane({ 0.0f, 0.0f, 0.1f }, { 0.0f, 0.0f, 1.0f }, triViewed, clipped[0], clipped[1]);

				// We may end up with multiple triangles form the clip, so project as
				// required
				for (int n = 0; n < nClippedTriangles; n++){
					Vec3d vOffsetView = { 1,1,0 };

					//for each point in the triangle
					for (int i = 0; i < 3; i++){
						// Project triangles from 3D --> 2D
						triProjected.p[i] = matProj * clipped[n].p[i];

						// Scale into view, we moved the normalising into cartesian space
						// out of the matrix.vector function from the previous videos, so
						// do this manually
						triProjected.p[i] = triProjected.p[i] / triProjected.p[i].w;

						// X/Y are inverted so put them back
						triProjected.p[i].x *= -1.0f;
						triProjected.p[i].y *= 1.0f;

						// Offset verts into visible normalised space
						triProjected.p[i] = triProjected.p[i] + vOffsetView;

						// Scale into view
						triProjected.p[i].x *= 0.5f * (float)windowWidth;
						triProjected.p[i].y *= 0.5f * (float)windowHeight;
					}
					triProjected.col = clipped[n].col;

					// Store Triangle for sorting
					vecTrianglesToClip.push_back(triProjected);
				}			
			}
		}

		// Sort triangles from back to front
		sort(vecTrianglesToClip.begin(), vecTrianglesToClip.end(), [](Triangle &t1, Triangle &t2){
			float z1 = (t1.p[0].z + t1.p[1].z + t1.p[2].z) / 3.0f;
			float z2 = (t2.p[0].z + t2.p[1].z + t2.p[2].z) / 3.0f;
			return z1 > z2;
		});


		// Loop through all transformed, viewed, projected, and sorted triangles
		// Clip triangles against all four screen edges and normalise to OpenGL screen coordinates
		for (auto &triToRaster : vecTrianglesToClip){
			// Clip triangles against all four screen edges, this could yield
			// a bunch of triangles, so create a queue that we traverse to 
			//  ensure we only test new triangles generated against planes
			Triangle clipped[2];
			list<Triangle> listTriangles;

			// Add initial Triangle
			listTriangles.push_back(triToRaster);
			int nNewTriangles = 1;

			for (int p = 0; p < 4; p++){
				int nTrisToAdd = 0;
				while (nNewTriangles > 0){
					// Take Triangle from front of queue
					Triangle test = listTriangles.front();
					listTriangles.pop_front();
					nNewTriangles--;

					// Clip it against a plane. We only need to test each 
					// subsequent plane, against subsequent new triangles
					// as all triangles after a plane clip are guaranteed
					// to lie on the inside of the plane. I like how this
					// comment is almost completely and utterly justified
					switch (p)
					{
					case 0:	nTrisToAdd = Triangle_ClipAgainstPlane({ 0.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f }, test, clipped[0], clipped[1]); break;
					case 1:	nTrisToAdd = Triangle_ClipAgainstPlane({ 0.0f, (float)windowHeight - 1, 0.0f }, { 0.0f, -1.0f, 0.0f }, test, clipped[0], clipped[1]); break;
					case 2:	nTrisToAdd = Triangle_ClipAgainstPlane({ 0.0f, 0.0f, 0.0f }, { 1.0f, 0.0f, 0.0f }, test, clipped[0], clipped[1]); break;
					case 3:	nTrisToAdd = Triangle_ClipAgainstPlane({ (float)windowWidth - 1, 0.0f, 0.0f }, { -1.0f, 0.0f, 0.0f }, test, clipped[0], clipped[1]); break;
					}

					// Clipping may yield a variable number of triangles, so
					// add these new ones to the back of the queue for subsequent
					// clipping against next planes
					for (int w = 0; w < nTrisToAdd; w++)
						listTriangles.push_back(clipped[w]);
				}
				nNewTriangles = listTriangles.size();
			}


			// Draw the transformed, viewed, clipped, projected, sorted, clipped triangles
			for (auto &t : listTriangles){
				// normalise to screen and push to draw list
				for(int i = 0; i < 3; i++){
					t.p[i].x = (t.p[i].x / (windowWidth / 2)) - 1;
					t.p[i].y = (t.p[i].y / (windowHeight / 2)) - 1;					
				}

				std::array<float, 9> point{t.p[0].x, t.p[0].y, 0.0f, t.p[1].x, t.p[1].y, 0.0f, t.p[2].x, t.p[2].y, 0.0f};
				trianglesToDraw.push_back(point);
				coloursToDraw.push_back(t.col);				
			}
		}


		drawTriangle(trianglesToDraw, coloursToDraw);

		return true;
	}

	void Run(){

		auto tp1 = std::chrono::system_clock::now();
		auto tp2 = std::chrono::system_clock::now();

		//mouse
		double lastX = 0.0;
		double lastY = 0.0;

		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

		int fps_update = 0;
		double fps_elapsed_time = 0.0;

		while (!glfwWindowShouldClose(window)){
			// Run as fast as possible
			
				// Handle Timing
				tp2 = std::chrono::system_clock::now();
				std::chrono::duration<float> elapsedTime = tp2 - tp1;
				tp1 = tp2;
				float fElapsedTime = elapsedTime.count();

				//handle mouse - use change in mouse position to rotate camera
				double mouseX, mouseY;
				glfwGetCursorPos(window, &mouseX, &mouseY);
				double xoffset = mouseX - lastX;
				double yoffset = lastY - mouseY; // reversed since y-coordinates go from bottom to top
				lastX = mouseX;
				lastY = mouseY;

				float sensitivity = 5.0f;
				xoffset *= sensitivity;
				yoffset *= sensitivity;

				camera.fYaw += xoffset * fElapsedTime;
				camera.fPitch += yoffset * fElapsedTime;


				//stop pitch going too high or low
				if(camera.fPitch > 1.5f){
					camera.fPitch = 1.5f;
				}

				if(camera.fPitch < -1.5f){
					camera.fPitch = -1.5f;
				}

				Vec3d vForward = camera.lookDir * (8.0f * fElapsedTime);
				Vec3d vRight = { camera.lookDir.z, 0, -camera.lookDir.x };
				vRight = vRight * (8.0f * fElapsedTime);

				Vec3d vUp = { 0,1,0 };
				vUp = vUp * (8.0f * fElapsedTime);

				// Standard FPS Control scheme, but turn instead of strafe
				if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS){
					camera.pos = camera.pos + vForward;
				}

				if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS){
					camera.pos = camera.pos - vForward;
				}

				if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS){
					//pan camera left
					camera.pos = camera.pos + vRight;
				}

				if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS){
					//pan camera right
					camera.pos = camera.pos - vRight;
				}

				if(glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS){
					//move camera up
					camera.pos.y += 8.0f * fElapsedTime;
				}

				if(glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS){
					//move camera down
					camera.pos.y -= 8.0f * fElapsedTime;
				}

				//escape
				if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS){
					glfwSetWindowShouldClose(window, true);
				}
					

				// Handle Frame Update

				//update screen
				// Enable the vertex array functionality
				glEnableClientState(GL_VERTEX_ARRAY);

				// Set the background color to white
				glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
				glClear(GL_COLOR_BUFFER_BIT);

				Render(fElapsedTime);


				// Disable the vertex array functionality
				glDisableClientState(GL_VERTEX_ARRAY);
				// Swap buffers
				glfwSwapBuffers(window);
				// Poll for and process events
				glfwPollEvents();

				// Update Title Bar
				fps_update++;
				if (fps_update > 50){
					double average_fps = 50.0f / fps_elapsed_time;
					// Update window title with FPS
					std::string windowTitle = "GLFW game engine - FPS: " + std::to_string(average_fps);
					glfwSetWindowTitle(window, windowTitle.c_str());
					fps_update = 0;
					fps_elapsed_time = 0.0;
				} else {
					fps_elapsed_time += fElapsedTime;
				}

				
		}

		// Clean up and exit
    	glfwTerminate();
    	return;
	}

};




int main(){
	GameEngine3D game(1200, 800, "teapot.obj");

	game.Run();

    return 0;
}
