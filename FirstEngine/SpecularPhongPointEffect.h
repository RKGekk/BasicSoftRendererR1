#pragma once

#include <algorithm>
#include <cmath>
#include <DirectXMath.h>

#include "Vertex.h"
#include "Triangle.h"
#include "ColorIntegers.h"
#include "TextureClass.h"

class SpecularPhongPointEffect {
public:

	struct PointDiffuseParams {
		static constexpr float linear_attenuation		= 0.9f;
		static constexpr float quadradic_attenuation	= 0.6f;
		static constexpr float constant_attenuation		= 0.682f;
	};

	struct SpecularParams {
		static constexpr float specular_power		= 30.0f;
		static constexpr float specular_intensity	= 0.6f;
	};
	
	// vertex shader
	// output interpolates position, normal, world position
	class VSOutput {
	public:
		VSOutput() = default;
		VSOutput(const DirectX::XMFLOAT4& pos) : pos(pos) {}
		VSOutput(const DirectX::XMFLOAT4& pos, const VSOutput& src) : n(src.n), worldPos(src.worldPos), pos(pos) {}
		VSOutput(const DirectX::XMFLOAT4& pos, const DirectX::XMFLOAT3& n, const DirectX::XMFLOAT3& worldPos) : n(n), pos(pos), worldPos(worldPos) {}

		VSOutput& operator+=(const VSOutput& rhs) {

			pos.x += rhs.pos.x;
			pos.y += rhs.pos.y;
			pos.z += rhs.pos.z;
			pos.w += rhs.pos.w;

			n.x += rhs.n.x;
			n.y += rhs.n.y;
			n.z += rhs.n.z;

			worldPos.x += rhs.worldPos.x;
			worldPos.y += rhs.worldPos.y;
			worldPos.z += rhs.worldPos.z;

			t.x += rhs.t.x;
			t.y += rhs.t.y;

			return *this;
		}

		VSOutput operator+(const VSOutput& rhs) const {
			return VSOutput(*this) += rhs;
		}

		VSOutput& operator-=(const VSOutput& rhs) {
			
			pos.x -= rhs.pos.x;
			pos.y -= rhs.pos.y;
			pos.z -= rhs.pos.z;
			pos.w -= rhs.pos.w;

			n.x -= rhs.n.x;
			n.y -= rhs.n.y;
			n.z -= rhs.n.z;

			worldPos.x -= rhs.worldPos.x;
			worldPos.y -= rhs.worldPos.y;
			worldPos.z -= rhs.worldPos.z;

			t.x -= rhs.t.x;
			t.y -= rhs.t.y;

			return *this;
		}

		VSOutput operator-(const VSOutput& rhs) const {
			return VSOutput(*this) -= rhs;
		}

		VSOutput& operator*=(float rhs) {
			
			pos.x *= rhs;
			pos.y *= rhs;
			pos.z *= rhs;
			pos.w *= rhs;

			n.x *= rhs;
			n.y *= rhs;
			n.z *= rhs;

			worldPos.x *= rhs;
			worldPos.y *= rhs;
			worldPos.z *= rhs;

			t.x *= rhs;
			t.y *= rhs;

			return *this;
		}

		VSOutput operator*(float rhs) const {
			return VSOutput(*this) *= rhs;
		}

		VSOutput& operator/=(float rhs) {
			
			pos.x /= rhs;
			pos.y /= rhs;
			pos.z /= rhs;
			pos.w /= rhs;

			n.x /= rhs;
			n.y /= rhs;
			n.z /= rhs;

			worldPos.x /= rhs;
			worldPos.y /= rhs;
			worldPos.z /= rhs;

			t.x /= rhs;
			t.y /= rhs;

			return *this;
		}

		VSOutput operator/(float rhs) const {
			return VSOutput(*this) /= rhs;
		}

	public:
		DirectX::XMFLOAT4 pos;
		DirectX::XMFLOAT3 n;
		DirectX::XMFLOAT3 worldPos;
		DirectX::XMFLOAT2 t;
	};

	class VertexShader {
	public:
		VertexShader() {
			DirectX::XMStoreFloat4x4(&proj, DirectX::XMMatrixIdentity());
			DirectX::XMStoreFloat4x4(&worldView, DirectX::XMMatrixIdentity());
			DirectX::XMStoreFloat4x4(&worldViewProj, DirectX::XMMatrixIdentity());
		}

		void BindWorldView(DirectX::XMFLOAT4X4& transformation_in) {
			worldView = transformation_in;

			DirectX::XMMATRIX worldViewXM = DirectX::XMLoadFloat4x4(&worldView);
			DirectX::XMMATRIX projXM = DirectX::XMLoadFloat4x4(&proj);

			DirectX::XMStoreFloat4x4(&worldViewProj, DirectX::XMMatrixMultiply(worldViewXM, projXM));
		}

		void BindProjection(DirectX::XMFLOAT4X4& transformation_in) {
			proj = transformation_in;

			DirectX::XMMATRIX worldViewXM = DirectX::XMLoadFloat4x4(&worldView);
			DirectX::XMMATRIX projXM = DirectX::XMLoadFloat4x4(&proj);

			DirectX::XMStoreFloat4x4(&worldViewProj, DirectX::XMMatrixMultiply(worldViewXM, projXM));
		}

		DirectX::XMFLOAT4X4& GetProj() {
			return proj;
		}

		VSOutput operator()(Vertex& v) {
			VSOutput out;

			DirectX::XMMATRIX worldViewXM		= DirectX::XMLoadFloat4x4(&worldView);
			DirectX::XMMATRIX worldViewProjXM	= DirectX::XMLoadFloat4x4(&worldViewProj);

			DirectX::XMVECTOR posXM				= DirectX::XMLoadFloat4(&v.pos);
			DirectX::XMVECTOR nXM				= DirectX::XMLoadFloat3(&v.n);

			DirectX::XMStoreFloat4(&out.pos,		DirectX::XMVector4Transform(posXM,	worldViewProjXM));
			DirectX::XMStoreFloat3(&out.n,			DirectX::XMVector4Transform(nXM,	worldViewXM));
			DirectX::XMStoreFloat3(&out.worldPos,	DirectX::XMVector4Transform(posXM,	worldViewXM));
			out.t = v.t;

			return out;
		}

	protected:
		DirectX::XMFLOAT4X4 proj;
		DirectX::XMFLOAT4X4 worldView;
		DirectX::XMFLOAT4X4 worldViewProj;
	};

	// invoked for each pixel of a triangle
	// takes an input of attributes that are the
	// result of interpolating vertex attributes
	// and outputs a color
	class GeometryShader {
	public:

		Triangle<VSOutput> operator()(VSOutput& in0, VSOutput& in1, VSOutput& in2, size_t triangle_index) {
			return { in0, in1, in2 };
		}
	};

	class PixelShader {
	public:

		ColorIntegers Shade(VSOutput& in, DirectX::XMFLOAT3& material_color) {

			DirectX::XMVECTOR posXM				= DirectX::XMLoadFloat4(&in.pos);
			DirectX::XMVECTOR nXM				= DirectX::XMLoadFloat3(&in.n);
			DirectX::XMVECTOR worldPosXM		= DirectX::XMLoadFloat3(&in.worldPos);

			DirectX::XMVECTOR light_posXM		= DirectX::XMLoadFloat3(&light_pos);
			DirectX::XMVECTOR light_diffuseXM	= DirectX::XMLoadFloat3(&light_diffuse);
			DirectX::XMVECTOR light_ambientXM	= DirectX::XMLoadFloat3(&light_ambient);
			DirectX::XMVECTOR material_colorXM	= DirectX::XMLoadFloat3(&material_color);
			
			// re-normalize interpolated surface normal
			DirectX::XMVECTOR surf_normXM = DirectX::XMVector3Normalize(nXM);

			// vertex to light data
			DirectX::XMVECTOR v_to_l	= DirectX::XMVectorSubtract(light_posXM, worldPosXM);
			DirectX::XMVECTOR dir		= DirectX::XMVector3Normalize(v_to_l);

			float dist = DirectX::XMVectorGetX(DirectX::XMVector3Length(v_to_l));

			// calculate attenuation
			float attenuation = 1.0f / (PointDiffuseParams::constant_attenuation + PointDiffuseParams::linear_attenuation * dist + PointDiffuseParams::quadradic_attenuation * dist * dist);

			// calculate intensity based on angle of incidence and attenuation
			DirectX::XMVECTOR d = DirectX::XMVectorScale(light_diffuseXM, attenuation * std::max<float>(0.0f, DirectX::XMVectorGetX(DirectX::XMVector3Dot(surf_normXM,  dir))));

			// reflected light vector
			//DirectX::XMVECTOR w = DirectX::XMVectorScale(surf_normXM, DirectX::XMVectorGetX(DirectX::XMVector3Dot(v_to_l, surf_normXM)));
			//DirectX::XMVECTOR r = DirectX::XMVectorSubtract(DirectX::XMVectorScale(w, 2.0f), v_to_l);
			DirectX::XMVECTOR r = DirectX::XMVector3Reflect(v_to_l, surf_normXM);
			DirectX::XMVECTOR rn = DirectX::XMVector3Normalize(r);

			// calculate specular intensity based on angle between viewing vector and reflection vector, narrow with power function
			DirectX::XMVECTOR s = DirectX::XMVectorScale(light_diffuseXM, SpecularParams::specular_intensity * std::pow(std::max<float>(0.0f, DirectX::XMVectorGetX(DirectX::XMVector3Dot(DirectX::XMVectorScale(rn, -1.0f), DirectX::XMVector3Normalize(worldPosXM)))), SpecularParams::specular_power));

			// add diffuse+ambient, filter by material color, saturate and scale
			DirectX::XMVECTOR vMax = DirectX::XMVectorSet(1.0f, 1.0f, 1.0f, 0.0f);
			DirectX::XMVECTOR vMin = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
			DirectX::XMVECTOR result = DirectX::XMVectorClamp(DirectX::XMVectorMultiply(material_colorXM, DirectX::operator+(DirectX::operator+(d, light_ambientXM), s)), vMin, vMax);
			DirectX::XMFLOAT3 out;
			DirectX::XMStoreFloat3(&out, result);

			return ColorIntegers(out.x * 255.0f, out.y * 255.0f, out.z * 255.0f);
		}

		void SetDiffuseLight(DirectX::XMFLOAT3& c) {
			light_diffuse = c;
		}

		void SetAmbientLight(DirectX::XMFLOAT3& c) {
			light_ambient = c;
		}

		void SetLightPosition(DirectX::XMFLOAT3& pos_in) {
			light_pos = pos_in;
		}

		ColorIntegers operator()(VSOutput& in) {

			ColorIntegers color = pTex->GetPixel(
				unsigned int(in.t.x * tex_width + 0.5f) % tex_width,
				unsigned int(in.t.y * tex_height + 0.5f) % tex_width
			);

			auto material_color_t = DirectX::XMFLOAT3(
				color.GetR() / 255.0f,
				color.GetG() / 255.0f,
				color.GetB() / 255.0f
			);

			return Shade(in, material_color_t);
		}

		void BindTexture(TextureClass& tex) {
			pTex = &tex;
			tex_width = pTex->GetWidth();
			tex_height = pTex->GetHeight();
		}

	private:
		DirectX::XMFLOAT3 light_pos			= { 0.0f,0.0f,0.5f };
		DirectX::XMFLOAT3 light_diffuse		= { 1.0f,1.0f,1.0f };
		DirectX::XMFLOAT3 light_ambient		= { 0.1f,0.1f,0.1f };

		DirectX::XMFLOAT3 material_color	= { 0.8f,0.85f,1.0f };
		TextureClass* pTex = nullptr;
		unsigned int tex_width;
		unsigned int tex_height;
	};

public:
	VertexShader	vs;
	GeometryShader	gs;
	PixelShader		ps;
};