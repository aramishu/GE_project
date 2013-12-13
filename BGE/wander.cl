__kernel void wander (__global float* in_pos_x, __global float* in_pos_y, __global float* in_pos_z, 
						__global float* in_vel_x, __global float* in_vel_y, __global float* in_vel_z)
{
	const int i = get_global_id (0);

	/*just increment the x position for testing*/
	in_pos_x[i] + 1.0f;
}