#include <cmath>

#include "light.h"

double DirectionalLight::distanceAttenuation(const vec3f &P) const
{
	// distance to light is infinite, so f(di) goes to 0.  Return 1.
	return 1.0;
}

vec3f DirectionalLight::shadowAttenuation(const vec3f &P) const
{
	// YOUR CODE HERE:
	// You should implement shadow-handling code here.

	vec3f d = getDirection(P);
	vec3f attenuation = {1, 1, 1};

	isect i;
	ray r = ray(P, d);

	vec3f tempP = P;
	ray tempr(r);

	// recursively to find intersection
	while (scene->intersect(tempr, i))
	{
		// if the material is not transparent, return black
		if (i.getMaterial().kt.iszero())
			return vec3f(0, 0, 0);
		// speed up by setting a threshold
		double threshold = 4e-2;
		if (attenuation[0] < threshold && attenuation[1] < threshold && attenuation[2] < threshold)
			return vec3f(0, 0, 0);
		tempP = tempr.at(i.t);
		tempr = ray(tempP, d);
		attenuation = attenuation.elementwiseMultiply(i.getMaterial().kt);
	}
	return attenuation;
}

vec3f DirectionalLight::getColor(const vec3f &P) const
{
	// Color doesn't depend on P
	return color;
}

vec3f DirectionalLight::getDirection(const vec3f &P) const
{
	return -orientation;
}

double PointLight::distanceAttenuation(const vec3f &P) const
{
	// YOUR CODE HERE

	// You'll need to modify this method to attenuate the intensity
	// of the light based on the distance between the source and the
	// point P.  For now, I assume no attenuation and just return 1.0
	double distance = (position - P).length();
	return min(1.0, 1.0 / (constant_attenuation_coeff + linear_attenuation_coeff * distance + quadratic_attenuation_coeff * distance * distance));
}

vec3f PointLight::getColor(const vec3f &P) const
{
	// Color doesn't depend on P
	return color;
}

vec3f PointLight::getDirection(const vec3f &P) const
{
	return (position - P).normalize();
}

vec3f PointLight::shadowAttenuation(const vec3f &P) const
{
	// YOUR CODE HERE:
	// You should implement shadow-handling code here.

	double distance = (position - P).length();
	vec3f d = getDirection(P).normalize();
	ray r = ray(P, d);
	vec3f attenuation = {1, 1, 1};

	isect i;

	vec3f tempP = P;
	ray tempr(r);

	while (scene->intersect(tempr, i))
	{
		// intersection behind the light
		if ((distance -= i.t) < RAY_EPSILON)
			return attenuation;
		// if the material is not transparent, return black
		if (i.getMaterial().kt.iszero())
			return vec3f(0, 0, 0);
		tempP = tempr.at(i.t);
		tempr = ray(tempP, d);
		attenuation = attenuation.elementwiseMultiply(i.getMaterial().kt);
	}
	return attenuation;
}
