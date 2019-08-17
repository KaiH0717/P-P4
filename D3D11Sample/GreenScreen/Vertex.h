#pragma once
#include <functional>

struct PER_VERTEX_DATA
{
	float position[4];
	float normal[3];
	float texture[2];
};

struct VERTEXHASH
{
	std::size_t operator()(const PER_VERTEX_DATA& v) const
	{
		return std::size_t(std::hash<float>()(v.position[0]) +
			std::hash<float>()(v.position[1]) +
			std::hash<float>()(v.position[2]) +
			std::hash<float>()(v.position[3]) +
			std::hash<float>()(v.normal[0]) +
			std::hash<float>()(v.normal[1]) +
			std::hash<float>()(v.normal[2]) +
			std::hash<float>()(v.texture[0]) +
			std::hash<float>()(v.texture[1])
			);
	}
};

struct VERTEXEQUAL
{
	bool operator()(const PER_VERTEX_DATA& a, const PER_VERTEX_DATA& b) const
	{
		return (a.position[0] == b.position[0] &&
			a.position[1] == b.position[1] &&
			a.position[2] == b.position[2] &&
			a.position[3] == b.position[3] &&
			a.normal[0] == b.normal[0] &&
			a.normal[1] == b.normal[1] &&
			a.normal[2] == b.normal[2] &&
			a.texture[0] == b.texture[0] &&
			a.texture[1] == b.texture[1]
			);
	}
};
