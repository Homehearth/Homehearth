#include "EnginePCH.h"
#include "D2D1Core.h"

#define INSTANCE D2D1Core::instance
D2D1Core* INSTANCE = nullptr;

D2D1Core::D2D1Core()
{
	m_writeFactory = nullptr;
	m_factory = nullptr;
	m_writeFont = nullptr;
	m_writeFormat = nullptr;
	m_renderTarget = nullptr;
	m_surface = nullptr;
	m_hwndTarget = nullptr;
	m_solidBrush = nullptr;
}

D2D1Core::~D2D1Core()
{
	if (m_writeFactory)
		m_writeFactory->Release();
	if (m_factory)
		m_factory->Release();
	if (m_writeFont)
		m_writeFont->Release();
	if (m_writeFormat)
		m_writeFormat->Release();
	if (m_renderTarget)
		m_renderTarget->Release();
	if (m_surface)
		m_surface->Release();
	if (m_hwndTarget)
		m_hwndTarget->Release();
	if (m_solidBrush)
		m_solidBrush->Release();
}

const bool D2D1Core::Setup(Window* window)
{
	D2D1_FACTORY_OPTIONS options = {};
#ifdef _DEBUG
	options.debugLevel = D2D1_DEBUG_LEVEL_INFORMATION;
#endif
	// Create a factory for D2D1, if it fails we LOG_ERROR and return false.
	HRESULT hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_MULTI_THREADED, options, &m_factory);
	if (FAILED(hr))
		[] {LOG_ERROR("Creating D2D1Factory failed."); return false; };

	hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory),
		reinterpret_cast<IUnknown**>(&m_writeFactory));
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
	hr = m_factory->CreateDxgiSurfaceRenderTarget(m_surface, props, &m_renderTarget);
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

void D2D1Core::DrawT(const std::string text, Window* window, IDWriteTextFormat* format)
{
	if (INSTANCE->m_renderTarget)
	{
		IDWriteTextFormat* current_format = INSTANCE->m_writeFormat;
		if (format)
			current_format = format;

		RECT rc;
		GetClientRect(window->GetHWnd(), &rc);
		D2D1_RECT_F layoutRect = D2D1::RectF(
			static_cast<FLOAT>(rc.left),
			static_cast<FLOAT>(rc.top),
			static_cast<FLOAT>(rc.right - rc.left),
			static_cast<FLOAT>(rc.bottom - rc.top)
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
			INSTANCE->m_solidBrush
		);

		delete[] pwcsName;
	}
}

void D2D1Core::DrawF(const float upper_x, const float upper_y, const float width, const float height, const Shapes shape)
{
	INSTANCE->m_renderTarget->SetTransform(D2D1::Matrix3x2F::Identity());
	D2D1_SIZE_F rtSize = INSTANCE->m_renderTarget->GetSize();
	ID2D1PathGeometry* geometry = nullptr;
	switch (shape)
	{
	case Shapes::RECTANGLE_FILLED:
		D2D1_RECT_F rectangle_filled = D2D1::RectF(upper_x, upper_y, upper_x + width , upper_y + height);
		INSTANCE->m_renderTarget->FillRectangle(&rectangle_filled, INSTANCE->m_solidBrush);
		break;
	case Shapes::RECTANGLE_OUTLINED:
		D2D1_RECT_F rectangle_outlined = D2D1::RectF(upper_x, upper_y, upper_x + width, upper_y + height);
		INSTANCE->m_renderTarget->DrawRectangle(&rectangle_outlined, INSTANCE->m_solidBrush);
		break;
	case Shapes::TRIANGLE_FILLED:
		INSTANCE->m_factory->CreatePathGeometry(&geometry);
		if (!geometry)
			break;
		ID2D1GeometrySink* sink;
		geometry->Open(&sink);
		D2D1_POINT_2F p1 = D2D1::Point2F(upper_x + width * 0.5f, upper_y);
		sink->BeginFigure(p1, D2D1_FIGURE_BEGIN_FILLED);
		D2D1_POINT_2F p2 = D2D1::Point2F(upper_x + width, upper_y + height);
		D2D1_POINT_2F p3 = D2D1::Point2F(upper_x, upper_y + height);
		sink->AddLine(p2);
		sink->AddLine(p3);
		sink->EndFigure(D2D1_FIGURE_END_CLOSED);
		HRESULT hr = sink->Close();

		sink->Release();
		INSTANCE->m_renderTarget->DrawGeometry(geometry, INSTANCE->m_solidBrush);
		INSTANCE->m_renderTarget->FillGeometry(geometry, INSTANCE->m_solidBrush);
		geometry->Release();
		break;
	}
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