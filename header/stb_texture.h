//=====================================================================
// Image loader for ray tracing applications
// Author:
// Andrew Davidson, Department of Computer Science and Software Engineering
// University of Canterbury, Christchurch, New Zealand.
//=====================================================================

#ifndef STB_TEXTURE
#define STB_TEXTURE

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <iostream>

using namespace std;

class STBTexture {
	private:
		int width, height, channels;
		unsigned char* imageData;
	public:
		STBTexture(): width(0), height(0), channels(0) {}
		STBTexture(const char* filename) {
			int ok = stbi_info(filename, &width, &height, &channels);
			if (ok == 1) {
				imageData = stbi_load(filename, &width, &height, &channels, 0);
				cout << "Loaded " << filename << endl;
			} else {
				cout << "Failed to load " << filename << endl;
			}
		}

		//Return color at texture coord (s, t) where s and t are in [0,1]
		glm::vec3 getColorAt(float s, float t) {
			if (width == 0 || height == 0) return glm::vec3(0);
			int i = (int)(s * width);  //pixel coordinates
			int j = (int)(t * height);
			if (i < 0 || i > width - 1 || j < 0 || j > height - 1) return glm::vec3(0); //out of bounds
			
			int pixelIndex = ((j * width) + i) * channels;
			return glm::vec3(
				(float)imageData[pixelIndex    ]/255.0, 
				(float)imageData[pixelIndex + 1]/255.0, 
				(float)imageData[pixelIndex + 2]/255.0);
		}
};

#endif

