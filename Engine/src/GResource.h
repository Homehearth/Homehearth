#pragma once
namespace resource
{
	class GResource
	{
	public:
		virtual ~GResource() = 0 {};

		//Every resource can be created with a filename
		virtual bool Create(const std::string& filename) = 0;
	};
}