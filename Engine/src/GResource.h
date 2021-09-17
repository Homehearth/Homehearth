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

		/*
			Add a reference to this Resource.
		*/
		void AddRef();

		/*
			Decrease the reference counter for this Resource.
		*/
		void Release();

		/*
			Returns the current reference counter for this Resource.
		*/
		const unsigned int GetRef() const;
	};

	/*
		Release the pointer to a Resource.
		This will decrease the reference counter and set
		the pointer to nullptr.
	*/
	void SafeRelease(GResource* pointer);
}