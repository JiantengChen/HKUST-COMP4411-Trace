#include <cmath>
#include <assert.h>

#include "Box.h"

bool Box::intersectLocal(const ray &r, isect &i) const
{
	// YOUR CODE HERE:
	// Add box intersection code here.
	// it currently ignores all boxes and just returns false.

	// Initialize the ray origin and direction
	vec3f ray_d = r.getDirection();
	vec3f ray_o = r.getPosition();

	// set the t_near and t_far to the maximum and minimum value
	double t_far = DBL_MAX;
	double t_near = -DBL_MAX;

	i.obj = this;
	vec3f n_near, n_far;

	for (int i = 0; i < 3; i++)
	{
		if (abs(ray_d[i]) < RAY_EPSILON)
		{
			// If the ray is parallel to the plane, return false
			if (ray_o[i] < -0.5 || ray_o[i] > 0.5)
			{
				return false;
			}
		}
		else
		{
			// Calculate the intersection t value
			double t1 = (-0.5 - ray_o[i]) / ray_d[i];
			double t2 = (0.5 - ray_o[i]) / ray_d[i];
			vec3f n1, n2;
			n1[i] = -1;
			n2[i] = 1;

			if (t1 > t2)
			{
				swap(t1, t2);
				swap(n1, n2);
			}
			if (t1 > t_near)
			{
				t_near = t1;
				n_near = n1;
			}
			if (t2 < t_far)
			{
				t_far = t2;
				n_far = n2;
			}
			// If the intersection is invalid, return false
			if (t_near > t_far || t_far < RAY_EPSILON)
			{
				return false;
			}
		}
	}
	i.setT(t_near);
	i.setN(n_near);
	return true;
}