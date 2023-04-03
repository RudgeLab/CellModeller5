#pragma once

void initFrameCapture();

bool isFrameCaptureSupported();

void beginFrameCapture();
void endFrameCapture();
void launchFrameReplay();

class FrameCaptureScope
{
private:
	bool capture = false;
public:
	FrameCaptureScope(bool shouldCapture) : capture(shouldCapture) { if (capture) beginFrameCapture(); }
	FrameCaptureScope(const FrameCaptureScope&) = delete;
	FrameCaptureScope(FrameCaptureScope&&) = delete;
	~FrameCaptureScope() { if (capture) endFrameCapture(); }

	FrameCaptureScope& operator=(const FrameCaptureScope&) = delete;
	FrameCaptureScope& operator=(FrameCaptureScope&&) = delete;
};