#pragma once
namespace resource
{
	class GResource
	{
	public:
		virtual ~GResource() = 0 {};

		//Every resource can be created with a filepath
		virtual bool Create(const std::string& filepath) = 0;
	};
}