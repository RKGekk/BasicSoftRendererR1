#pragma once

#include <limits>

class ZBuffer {
public:
	ZBuffer(int width, int height) : width(width), height(height), pBuffer(new float[width * height]) {}
	ZBuffer(const ZBuffer&) = delete;
	~ZBuffer() {
		delete[] pBuffer;
		pBuffer = nullptr;
	}

	ZBuffer& operator=(const ZBuffer&) = delete;

	void Clear() {
		const int nDepths = width * height;
		for (int i = 0; i < nDepths; i++) {
			pBuffer[i] = std::numeric_limits<float>::infinity();
		}
	}

	float& At(int x, int y) {
		return pBuffer[y * width + x];
	}

	const float& At(int x, int y) const {
		return const_cast<ZBuffer*>(this)->At(x, y);
	}

	bool TestAndSet(int x, int y, float depth) {

		float& depthInBuffer = At(x, y);
		if (depth < depthInBuffer) {
			depthInBuffer = depth;
			return true;
		}
		return false;
	}

	int GetWidth() const {
		return width;
	}

	int GetHeight() const {
		return height;
	}

private:
	int		width;
	int		height;
	float*	pBuffer = nullptr;
};