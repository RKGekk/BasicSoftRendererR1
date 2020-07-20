#pragma once

#include <DirectXMath.h>

class Vertex {
public:
	Vertex() = default;
	Vertex(const DirectX::XMFLOAT4& pos) : pos(pos), n(), t(), color() {}
	Vertex(const DirectX::XMFLOAT4& pos, const Vertex& src) : pos(pos), n(src.n), t(), color() {}
	Vertex(const DirectX::XMFLOAT4& pos, const DirectX::XMFLOAT3& n) : pos(pos), n(n), t(), color() {}

public:
	DirectX::XMFLOAT4 pos;
	DirectX::XMFLOAT3 n;
	DirectX::XMFLOAT2 t;
	DirectX::XMFLOAT3 color;
};