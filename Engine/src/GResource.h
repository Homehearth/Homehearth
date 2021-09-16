#pragma once
namespace resource
{
	class GResource
	{
	private:
		unsigned int m_references = 0;
	public:
		virtual ~GResource() = 0 {};

		//Every resource can be created with a filename
		virtual bool Create(const std::string& filename) = 0;

		void AddRef();
		void DecreaseRef();
	};
}