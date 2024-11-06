#include "ray.h"
#include "material.h"
#include "light.h"

typedef list<Light *>::iterator liter;
typedef list<Light *>::const_iterator const_liter;

// Apply the phong model to this point on the surface of the object, returning
// the color of that point.
vec3f Material::shade(Scene *scene, const ray &r, const isect &i) const
{
	// YOUR CODE HERE

	// For now, this method just returns the diffuse color of the object.
	// This gives a single matte color for every distinct surface in the
	// scene, and that's it.  Simple, but enough to get you started.
	// (It's also inconsistent with the phong model...)

	// Your mission is to fill in this method with the rest of the phong
	// shading model, including the contributions of all the light sources.
	// You will need to call both distanceAttenuation() and shadowAttenuation()
	// somewhere in your code in order to compute shadows and bum?light falloff.

	vec3f I = ke + ka.elementwiseMultiply(scene->ambient_light);
	vec3f P = r.at(i.t);
	vec3f N = i.N;
	vec3f V = -r.getDirection();

	for (const_liter it = scene->beginLights(); it != scene->endLights(); ++it)
	{
		vec3f shadow_attenuation = (*it)->shadowAttenuation(P);
		double distance_attenuation = (*it)->distanceAttenuation(P);
		vec3f all_attentuation = distance_attenuation * shadow_attenuation;

		// TODO if having a texture, use it here

		// TODO if having a bump map, use it here?

		vec3f L = (*it)->getDirection(P);
		vec3f R = (2 * (N.dot(L)) * N - L).normalize();

		vec3f diffuse = kd * max(0.0, N.dot(L));
		// refer to https://course.cse.ust.hk/comp4411/Password_Only/projects/trace02/morehelp.html, multiply the shininess by 128
		vec3f specular = ks * pow(max(0.0, R.dot(V)), shininess * 128);

		I += all_attentuation.elementwiseMultiply((*it)->getColor(P)).elementwiseMultiply(diffuse + specular);
	}

	return I;
}
