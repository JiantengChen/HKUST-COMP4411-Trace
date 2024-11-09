#include <cmath>

#include "light.h"

#define PI 3.1415926
#define max(a, b) ((a) > (b) ? (a) : (b))
#define min(a, b) ((a) < (b) ? (a) : (b))

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
		// If the shadow ray intersects an object in the scene

		// If the material is opaque, return zero attenuation (full shadow)
		if (i.getMaterial().kt.iszero())
			return vec3f(0, 0, 0);

		// Speed up the process by setting a threshold
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

// add spot light here
vec3f SpotLight::getColor(const vec3f &P) const
{
	return color;
}

vec3f SpotLight::getDirection(const vec3f &P) const
{
	return (position - P).normalize();
}

vec3f SpotLight::shadowAttenuation(const vec3f &P) const
{
	vec3f L = (P - position).normalize();
	double coslambda = max(0, L.dot(orientation));
	double boundary = cos(coneangle * PI / 180.0);
	// outside the focus of the spotlight, no shadow cast at all
	if (coslambda < boundary)
		return vec3f(1, 1, 1);
	double distance = (position - P).length();
	vec3f d = (position - P).normalize();
	ray r(P, d);
	vec3f atten = {1, 1, 1};
	vec3f tempP = P;
	isect i;
	ray tempr(r);
	// recursively find intersection
	while (scene->intersect(tempr, i))
	{
		//  intersection is not before light
		if ((distance -= i.t) < RAY_EPSILON)
		{
			return atten;
		}
		// a totally un-transparent object
		if (i.getMaterial().kt.iszero())
			return {0, 0, 0};
		tempP = tempr.at(i.t);
		tempr = ray(tempP, d);
		atten = atten.elementwiseMultiply(i.getMaterial().kt);
	}
	return atten;
}

double SpotLight::distanceAttenuation(const vec3f &P) const
{
	// distance attenuation here contains the Warn Model as well
	double d = (P - position).length();
	double distance_atten = min(1, 1.0 / (constant_attenuation_coeff + linear_attenuation_coeff * d + quadratic_attenuation_coeff * d * d));
	vec3f L = (P - position).normalize();
	double warn_atten = 1.0;
	double coslambda = max(0, L.dot(orientation));
	double boundary = cos(coneangle * PI / 180.0);
	if (coslambda < boundary)
	{
		// the particle is outside boundary
		warn_atten = 0.0;
	}
	else
	{
		warn_atten = pow(coslambda, focus_constant);
	}
	return distance_atten * warn_atten;
}