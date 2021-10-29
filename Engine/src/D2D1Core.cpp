#include "EnginePCH.h"
#include "D2D1Core.h"

#define INSTANCE D2D1Core::instance
D2D1Core* INSTANCE = nullptr;

D2D1Core::D2D1Core()
{
	m_writeFactory = nullptr;
	m_factory = nullptr;
	m_writeFormat = nullptr;
	m_renderTarget = nullptr;
	m_surface = nullptr;
	m_hwndTarget = nullptr;
	m_solidBrush = nullptr;
	m_windowPointer = nullptr;
	m_imageFactory = nullptr;
}

D2D1Core::~D2D1Core()
{
	CoUninitialize();
}

const bool D2D1Core::Setup(Window* window)
{
	D2D1_FACTORY_OPTIONS options = {};
	m_windowPointer = window;
#ifdef _DEBUG
	options.debugLevel = D2D1_DEBUG_LEVEL_INFORMATION;
#endif
	// Create a factory for D2D1, if it fails we LOG_ERROR and return false.
	HRESULT hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, options, m_factory.GetAddressOf());
	if (FAILED(hr))
		[] {LOG_ERROR("Creating D2D1Factory failed."); return false; };

	hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory),
		reinterpret_cast<IUnknown**>(m_writeFactory.GetAddressOf()));
	if (FAILED(hr))
		[] {LOG_ERROR("Creating Write Factory failed."); return false; };

	// Get the surface from the backbuffer from D3D11.
	D3D11Core::Get().SwapChain()->GetBuffer(0, IID_PPV_ARGS(&m_surface));

	FLOAT dpi = static_cast<FLOAT>(GetDpiForWindow(window->GetHWnd()));
	D2D1_RENDER_TARGET_PROPERTIES props =
		D2D1::RenderTargetProperties(
			D2D1_RENDER_TARGET_TYPE_DEFAULT,
			D2D1::PixelFormat(DXGI_FORMAT_UNKNOWN, D2D1_ALPHA_MODE_PREMULTIPLIED),
			dpi,
			dpi
		);

	// Create a render target for the surface we created with D3D11 swapchain.
	hr = m_factory->CreateDxgiSurfaceRenderTarget(m_surface.Get(), props, m_renderTarget.GetAddressOf());
	if (FAILED(hr))
		[] {LOG_ERROR("Surface render target failed."); return false; };

	// Setup a HwndRender target with default properties.
	RECT rc;
	GetClientRect(window->GetHWnd(), &rc);
	D2D1_SIZE_U size = D2D1::SizeU(rc.right - rc.left, rc.bottom - rc.top);
	hr = m_factory->CreateHwndRenderTarget(
		D2D1::RenderTargetProperties(),
		D2D1::HwndRenderTargetProperties(window->GetHWnd(),
			D2D1::SizeU(rc.right - rc.left, rc.bottom - rc.top)),
		&m_hwndTarget);
	if (FAILED(hr))
		[] { LOG_ERROR("Creating HWND render target failed."); return false; };

	// Setup a default solid color brush to be used when rendering.
	hr = m_renderTarget->CreateSolidColorBrush(D2D1::ColorF(1.0f, 1.0f, 1.0f), &m_solidBrush);
	if (FAILED(hr))
		[] {LOG_WARNING("Creating default solid color brush failed."); };

	hr = m_writeFactory->CreateTextFormat(
		L"Times New Roman",
		NULL,
		DWRITE_FONT_WEIGHT_REGULAR,
		DWRITE_FONT_STYLE_NORMAL,
		DWRITE_FONT_STRETCH_NORMAL,
		24.0f,
		L"en-us",
		&m_writeFormat
	);
	if (FAILED(hr))
		[] {LOG_WARNING("Creating default Text Format failed."); };
	else
	{
		m_writeFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
		m_writeFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
	}

	hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);

	hr = CoCreateInstance(CLSID_WICImagingFactory,
		NULL,
		CLSCTX_INPROC_SERVER,
		IID_PPV_ARGS(&m_imageFactory));

	return true;
}

void D2D1Core::Initialize(Window* window)
{
	if (!INSTANCE)
	{
		INSTANCE = new D2D1Core();
		INSTANCE->Setup(window);
	}
}

void D2D1Core::Destroy()
{
	if (INSTANCE)
		delete INSTANCE;
}

void D2D1Core::DrawT(const std::string& text, const draw_text_t& opt)
{
	if (INSTANCE->m_renderTarget)
	{
		IDWriteTextFormat* current_format = INSTANCE->m_writeFormat.Get();
		if (opt.textFormat)
			current_format = opt.textFormat;

		RECT rc;
		GetClientRect(INSTANCE->m_windowPointer->GetHWnd(), &rc);
		D2D1_RECT_F layoutRect = D2D1::RectF(
			opt.x_pos,
			opt.y_pos,
			opt.x_pos + opt.x_stretch,
			opt.y_pos + opt.y_stretch
		);

		/*
			Convert the text to WCHAR.
		*/
		const char* t = text.c_str();
		const WCHAR* pwcsName;
		int nChars = MultiByteToWideChar(CP_ACP, 0, t, -1, NULL, 0);
		pwcsName = new WCHAR[nChars];
		MultiByteToWideChar(CP_ACP, 0, t, -1, (LPWSTR)pwcsName, nChars);
		
		INSTANCE->m_renderTarget->SetTransform(D2D1::IdentityMatrix());
		INSTANCE->m_renderTarget->DrawTextW(pwcsName,
			(UINT32)text.length(),
			current_format,
			layoutRect,
			INSTANCE->m_solidBrush.Get()
		);

		delete[] pwcsName;
	}
}

void D2D1Core::DrawF(const draw_t& fig, const draw_shape_t& shape)
{
	D2D1_COLOR_F oldColor = INSTANCE->m_solidBrush->GetColor();
	INSTANCE->m_renderTarget->SetTransform(D2D1::Matrix3x2F::Identity());
	D2D1_SIZE_F rtSize = INSTANCE->m_renderTarget->GetSize();

	// Set the color
	INSTANCE->m_solidBrush->SetColor(shape.color);

	ID2D1PathGeometry* geometry = nullptr;

	// Determine which shape to render.
	switch (shape.shape)
	{
	case Shapes::RECTANGLE_FILLED:
		D2D1_RECT_F rectangle_filled = D2D1::RectF(fig.x_pos, fig.y_pos, fig.x_pos + fig.width , fig.y_pos + fig.height);
		INSTANCE->m_renderTarget->FillRectangle(&rectangle_filled, INSTANCE->m_solidBrush.Get());
		break;
	case Shapes::RECTANGLE_OUTLINED:
		D2D1_RECT_F rectangle_outlined = D2D1::RectF(fig.x_pos, fig.y_pos, fig.x_pos + fig.width, fig.y_pos + fig.height);
		INSTANCE->m_renderTarget->DrawRectangle(&rectangle_outlined, INSTANCE->m_solidBrush.Get(), 5.0f);
		break;
	case Shapes::TRIANGLE_FILLED:
		INSTANCE->m_factory->CreatePathGeometry(&geometry);
		if (!geometry)
			break;
		ID2D1GeometrySink* sink;
		geometry->Open(&sink);
		D2D1_POINT_2F p1 = D2D1::Point2F(fig.x_pos + fig.width * 0.5f, fig.y_pos);
		sink->BeginFigure(p1, D2D1_FIGURE_BEGIN_FILLED);
		D2D1_POINT_2F p2 = D2D1::Point2F(fig.x_pos + fig.width, fig.y_pos + fig.height);
		D2D1_POINT_2F p3 = D2D1::Point2F(fig.x_pos, fig.y_pos + fig.height);
		sink->AddLine(p2);
		sink->AddLine(p3);
		sink->EndFigure(D2D1_FIGURE_END_CLOSED);
		HRESULT hr = sink->Close();

		sink->Release();
		INSTANCE->m_renderTarget->DrawGeometry(geometry, INSTANCE->m_solidBrush.Get());
		INSTANCE->m_renderTarget->FillGeometry(geometry, INSTANCE->m_solidBrush.Get());
		geometry->Release();
		break;
	}

	// Reset it to its old color.
	INSTANCE->m_solidBrush->SetColor(oldColor);
}

void D2D1Core::DrawP(const draw_t& fig, ID2D1Bitmap* texture)
{
	if (texture == nullptr)
		return;
	
	D2D1_SIZE_F size = texture->GetSize();
	D2D1_POINT_2F upperLeftCorner = D2D1::Point2F(fig.x_pos, fig.y_pos);

	D2D1_RECT_F dest = D2D1::RectF(upperLeftCorner.x, upperLeftCorner.y,
		(upperLeftCorner.x + fig.width), (upperLeftCorner.y + fig.height));

	INSTANCE->m_renderTarget->DrawBitmap(texture, dest);
}

void D2D1Core::Begin()
{
	if(INSTANCE)
		INSTANCE->m_renderTarget->BeginDraw();
}

void D2D1Core::Present()
{
	if(INSTANCE)
		INSTANCE->m_renderTarget->EndDraw();
}

const bool D2D1Core::CreateImage(const std::string& filename, ID2D1Bitmap** p_pointer)
{
	/*
		Setup searchpath, convert char* to WCHAR*
		Load Bitmap from file.
	*/
	std::string searchPath = TEXTUREPATH;
	searchPath.append(filename);
	const char* t = searchPath.c_str();
	const WCHAR* pwcsName;

	// conversion
	int nChars = MultiByteToWideChar(CP_ACP, 0, t, -1, NULL, 0);
	pwcsName = new WCHAR[nChars];
	MultiByteToWideChar(CP_ACP, 0, t, -1, (LPWSTR)pwcsName, nChars);

	HRESULT hr = INSTANCE->LoadBitMap(pwcsName, p_pointer);

	delete[] pwcsName;
	if (SUCCEEDED(hr))
		return true;
	else
		return false;
}

const bool D2D1Core::CreateTextFormat(const WCHAR* fontName, 
	IDWriteFontCollection* fontCollection, 
	const DWRITE_FONT_WEIGHT& weight, const DWRITE_FONT_STYLE& style, 
	const DWRITE_FONT_STRETCH& stretch, const FLOAT& fontSize, const WCHAR* localeName, 
	IDWriteTextFormat** pointer)
{
	HRESULT hr;

	hr = INSTANCE->m_writeFactory->CreateTextFormat(fontName, fontCollection, weight, style, stretch, fontSize, localeName, pointer);

	return SUCCEEDED(hr);
}

HRESULT D2D1Core::LoadBitMap(const LPCWSTR& filePath, ID2D1Bitmap** bitMap)
{
	HRESULT hr;
	IWICBitmapDecoder* decoder = nullptr;
	IWICBitmapFrameDecode* fDecoder = nullptr;
	IWICFormatConverter* convert = nullptr;

	/*
		Creating Bitmap pipeline.
	*/
	hr = m_imageFactory->CreateDecoderFromFilename(
		filePath,
		NULL,
		GENERIC_READ,
		WICDecodeMetadataCacheOnLoad,
		&decoder
	);
	if (SUCCEEDED(hr))
	{
		hr = decoder->GetFrame(0, &fDecoder);
	}
	if (SUCCEEDED(hr))
	{
		hr = m_imageFactory->CreateFormatConverter(&convert);
	}
	if (SUCCEEDED(hr))
	{
		hr = convert->Initialize(
			fDecoder,
			GUID_WICPixelFormat32bppPBGRA,
			WICBitmapDitherTypeNone,
			NULL,
			0.f,
			WICBitmapPaletteTypeMedianCut
		);
	}
	if (SUCCEEDED(hr))
	{
		hr = m_renderTarget->CreateBitmapFromWicBitmap(
			convert,
			NULL,
			bitMap
		);

		LOG_INFO("Creating bitmap success!");

		// Release
		if (convert)
			convert->Release();
		if (fDecoder)
			fDecoder->Release();
		if (decoder)
			decoder->Release();

		if (SUCCEEDED(hr))
			return S_OK;
		else
			return E_FAIL;
	}

	LOG_WARNING("Creating: bitmap texture failed.");

	// Release if anything failed.
	if(convert)
		convert->Release();
	if(fDecoder)
		fDecoder->Release();
	if(decoder)
		decoder->Release();

	return E_FAIL;
}
