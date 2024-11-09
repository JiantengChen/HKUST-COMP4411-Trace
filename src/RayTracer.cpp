// The main ray tracer.

#include <Fl/fl_ask.h>

#include "RayTracer.h"
#include "scene/light.h"
#include "scene/material.h"
#include "scene/ray.h"
#include "fileio/read.h"
#include "fileio/parse.h"
#include "ui/TraceUI.h"

extern TraceUI *traceUI;

// add reflect
vec3f RayTracer::reflect(ray r, isect i, bool flipNormal)
{
	vec3f D = r.getDirection().normalize();

	vec3f normal = i.N.normalize();
	if (!flipNormal)
	{
		normal *= -1;
	}

	vec3f R = D - 2 * D.dot(normal) * normal;

	return R;
}

// judge is total internal reflection
bool RayTracer::isTIR(ray r, isect i, double n_i, double n_t)
{
	return (
		pow(i.N.normalize().dot(r.getDirection().normalize()), 2) <=
		1 - pow(n_t / n_i, 2));
}

// refract
vec3f RayTracer::refract_dir(ray r, isect i, double n_i, double n_t, bool flipNormal)
{
	vec3f ret(0, 0, 0);
	vec3f n = i.N;
	if (flipNormal)
	{
		n *= -1;
	}
	vec3f v = r.getDirection().normalize();

	for (int i = 0; i < 3; i++)
	{
		ret[i] = n_i / n_t * ((sqrt(pow(n.dot(v), 2) + pow(n_t / n_i, 2) - 1) - n.dot(v)) * n[i] + v[i]);
	}
	return ret;
}

// Trace a top-level ray through normalized window coordinates (x,y)
// through the projection plane, and out into the scene.  All we do is
// enter the main ray-tracing method, getting things started by plugging
// in an initial ray weight of (0.0,0.0,0.0) and an initial recursion depth of 0.
vec3f RayTracer::trace(Scene *scene, double x, double y)
{
	ray r(vec3f(0, 0, 0), vec3f(0, 0, 0));
	scene->getCamera()->rayThrough(x, y, r);
	// return traceRay(scene, r, vec3f(1.0, 1.0, 1.0), traceUI->getDepth());
	// add threshold
	return traceRay(scene, r, vec3f(traceUI->getThresh(), traceUI->getThresh(), traceUI->getThresh()), traceUI->getDepth()).clamp();
}

// Do recursive ray tracing!  You'll want to insert a lot of code here
// (or places called from here) to handle reflection, refraction, etc etc.
vec3f RayTracer::traceRay(Scene *scene, const ray &r,
						  const vec3f &thresh, int depth)
{

	if (depth < 0 || thresh[0] > 1 || thresh[1] > 1 || thresh[2] > 1)
	{
		return {0, 0, 0};
	}
	isect i;
	vec3f intensity;

	if (scene->intersect(r, i))
	{
		// YOUR CODE HERE

		// An intersection occured!  We've got work to do.  For now,
		// this code gets the material for the surface that was intersected,
		// and asks that material to provide a color for the ray.

		// This is a great place to insert code for recursive ray tracing.
		// Instead of just returning the result of shade(), add some
		// more steps: add in the contributions from reflected and refracted
		// rays.

		const Material &m = i.getMaterial();

		intensity = m.shade(scene, r, i);

		// refractive index incident and transmitted
		double n_i, n_t;
		bool flipNormal;
		if (r.getDirection().dot(i.N) < 0)
		{
			// ray is entering the object
			n_i = 1.0; // air
			n_t = i.getMaterial().index;
			flipNormal = true;
		}
		else
		{
			// ray is exiting the object
			n_i = i.getMaterial().index;
			n_t = 1.0; // air
			flipNormal = false;
		}

		vec3f reflection_dir = reflect(r, i, flipNormal);
		vec3f kr = i.getMaterial().kr;
		ray reflection_ray(r.at(i.t) + i.N.normalize() * NORMAL_EPSILON, reflection_dir.normalize());
		intensity += kr.elementwiseMultiply(traceRay(scene, reflection_ray, thresh, depth - 1));

		if (!isTIR(r, i, n_i, n_t))
		{
			vec3f refraction_dir = refract_dir(r, i, n_i, n_t, flipNormal);
			vec3f kt = i.getMaterial().kt;
			ray refraction_ray(r.at(i.t), refraction_dir.normalize());
			intensity += kt.elementwiseMultiply(traceRay(scene, refraction_ray, thresh, depth - 1));
		}

		intensity = intensity.clamp();

		return intensity;
	}
	else
	{
		// No intersection.  This ray travels to infinity, so we color
		// it according to the background color, which in this (simple) case
		// is just black.

		return vec3f(0.0, 0.0, 0.0);
	}
}

RayTracer::RayTracer()
{
	buffer = NULL;
	buffer_width = buffer_height = 256;
	scene = NULL;

	m_bSceneLoaded = false;
}

RayTracer::~RayTracer()
{
	delete[] buffer;
	delete scene;
}

void RayTracer::getBuffer(unsigned char *&buf, int &w, int &h)
{
	buf = buffer;
	w = buffer_width;
	h = buffer_height;
}

double RayTracer::aspectRatio()
{
	return scene ? scene->getCamera()->getAspectRatio() : 1;
}

bool RayTracer::sceneLoaded()
{
	return m_bSceneLoaded;
}

bool RayTracer::loadScene(char *fn)
{
	try
	{
		scene = readScene(fn);
	}
	catch (ParseError pe)
	{
		fl_alert("ParseError: %s\n", pe);
		return false;
	}

	if (!scene)
		return false;

	buffer_width = 256;
	buffer_height = (int)(buffer_width / scene->getCamera()->getAspectRatio() + 0.5);

	bufferSize = buffer_width * buffer_height * 3;
	buffer = new unsigned char[bufferSize];

	// separate objects into bounded and unbounded
	scene->initScene();

	// Add any specialized scene loading code here

	m_bSceneLoaded = true;

	return true;
}

void RayTracer::traceSetup(int w, int h)
{
	if (buffer_width != w || buffer_height != h)
	{
		buffer_width = w;
		buffer_height = h;

		bufferSize = buffer_width * buffer_height * 3;
		delete[] buffer;
		buffer = new unsigned char[bufferSize];
	}
	memset(buffer, 0, w * h * 3);
}

void RayTracer::traceLines(int start, int stop)
{
	vec3f col;
	if (!scene)
		return;

	if (stop > buffer_height)
		stop = buffer_height;

	for (int j = start; j < stop; ++j)
		for (int i = 0; i < buffer_width; ++i)
			tracePixel(i, j);
}

void RayTracer::tracePixel(int i, int j)
{
	vec3f col;

	if (!scene)
		return;

	double x = double(i) / double(buffer_width);
	double y = double(j) / double(buffer_height);

	col = trace(scene, x, y);

	unsigned char *pixel = buffer + (i + j * buffer_width) * 3;

	pixel[0] = (int)(255.0 * col[0]);
	pixel[1] = (int)(255.0 * col[1]);
	pixel[2] = (int)(255.0 * col[2]);
}