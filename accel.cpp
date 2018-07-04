#include "interception.h"
#include "utils.h"
#include <windows.h>
#include <math.h>
#include <iostream>

int main()
{
	InterceptionContext context;
	InterceptionDevice device;
	InterceptionStroke stroke;

	raise_process_priority();

	context = interception_create_context();

	interception_set_filter(context, interception_is_mouse, INTERCEPTION_FILTER_MOUSE_MOVE);

	double
		frameTime_ms = 0,
		oldX = 0,
		oldY = 0,
		dx,
		dy;

	LARGE_INTEGER frameTime, oldFrameTime, PCfreq;

	QueryPerformanceCounter(&oldFrameTime);
	QueryPerformanceFrequency(&PCfreq);

	printf("MLT04 stair effect emulation based on interaccel.\nRequires the Interception driver.\nCTRL+C to stop.");

	while (interception_receive(context, device = interception_wait(context), &stroke, 1) > 0)
	{
		if (interception_is_mouse(device))
		{
			InterceptionMouseStroke &mstroke = *(InterceptionMouseStroke *)&stroke;

			if (!(mstroke.flags & INTERCEPTION_MOUSE_MOVE_ABSOLUTE)) {

				// figure out frametime
				QueryPerformanceCounter(&frameTime);
				frameTime_ms = (double)(frameTime.QuadPart - oldFrameTime.QuadPart) * 1000.0 / PCfreq.QuadPart;
				if (frameTime_ms > 200)
					frameTime_ms = 200;

				// retrieve new mouse data
				dx = (double)mstroke.x;
				dy = (double)mstroke.y;

				// output new counts
				if (abs(dx + oldX) >= 1 && abs(dy + oldY) >= 1) {
					mstroke.x = (int)(dx + oldX);
					mstroke.y = (int)(dy + oldY);
					oldX = oldY = 0;
				}
				else if (abs(dx + oldX) >= 2 || abs(dy + oldY) >= 2) {
						mstroke.x = (int)(dx + oldX);
						mstroke.y = (int)(dy + oldY);
						oldX = oldY = 0;
				}
				else {
					oldX += (int)(dx);
					oldY += (int)(dy);
					mstroke.x = 0;
					mstroke.y = 0;
				}

				oldFrameTime = frameTime;
			}

			interception_send(context, device, &stroke, 1);
		} 
	}

	interception_destroy_context(context);

	return 0;
}
