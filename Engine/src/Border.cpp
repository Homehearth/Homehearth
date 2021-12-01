#include "EnginePCH.h"
#include "Border.h"

rtd::Border::Border()
{
}

rtd::Border::Border(const draw_t& opts)
{
	m_opts = opts;
	m_borderOpts.color = D2D1::ColorF(1.0f, 1.0f, 1.0f);
	m_borderOpts.shape = Shapes::RECTANGLE_ROUNDED_OUTLINED;
}

rtd::Border::Border(const draw_shape_t& shape, const draw_t& opts)
{
	m_borderOpts = shape;
	m_opts = opts;
}

void rtd::Border::SetColor(const D2D1_COLOR_F& new_color)
{
	m_borderOpts.color = new_color;
}

D2D1_COLOR_F& rtd::Border::GetColor()
{
	return m_borderOpts.color;
}

bool rtd::Border::SetRound(const float& radius)
{
	HRESULT hr = D2D1Core::GetFactory()->CreatePathGeometry(m_geometry.GetAddressOf());
	if (SUCCEEDED(hr))
	{
		ID2D1GeometrySink* pSink = nullptr;
		hr = m_geometry->Open(&pSink);
		if (SUCCEEDED(hr))
		{
			pSink->SetFillMode(D2D1_FILL_MODE_WINDING);

			pSink->BeginFigure(D2D1::Point2(m_opts.x_pos, m_opts.y_pos + m_opts.height / 2.0f),
				D2D1_FIGURE_BEGIN_HOLLOW);

			pSink->AddArc(
				D2D1::ArcSegment(
					D2D1::Point2F(m_opts.x_pos + m_opts.width, m_opts.y_pos + m_opts.height / 2.0f),
					D2D1::SizeF(radius, radius),
					0.0f,
					D2D1_SWEEP_DIRECTION_CLOCKWISE,
					D2D1_ARC_SIZE_SMALL));

			pSink->AddArc(
				D2D1::ArcSegment(
					D2D1::Point2F(m_opts.x_pos, m_opts.y_pos + m_opts.height / 2.0f),
					D2D1::SizeF(radius, radius),
					0.0f,
					D2D1_SWEEP_DIRECTION_CLOCKWISE,
					D2D1_ARC_SIZE_SMALL));

			pSink->EndFigure(D2D1_FIGURE_END_CLOSED);
		}

		hr = pSink->Close();
		pSink->Release();
		pSink = nullptr;
	}

	return SUCCEEDED(hr);
}

void rtd::Border::SetLineWidth(const LineWidth& width)
{
	m_lineWidth = width;
}

void rtd::Border::UpdatePos(const draw_t& new_opts)
{
	m_opts = new_opts;
}

void rtd::Border::SetShape(const draw_shape_t& new_shape)
{
	m_borderOpts = new_shape;
}

void rtd::Border::SetShape(const Shapes& new_shape)
{
	m_borderOpts.shape = new_shape;
}

void rtd::Border::Draw()
{
	if (!m_geometry)
		D2D1Core::DrawF(m_opts, m_borderOpts, m_lineWidth);
	else
		D2D1Core::DrawF(m_geometry.Get());
}

void rtd::Border::OnClick()
{
}

void rtd::Border::OnHover()
{
}

ElementState rtd::Border::CheckClick()
{
	//// CheckCollisions if mouse key is pressed.
	//if (InputSystem::Get().CheckMouseKey(MouseKey::LEFT, KeyState::PRESSED))
	//{
	//	// Is within bounds?
	//	if (InputSystem::Get().GetMousePos().x > m_opts.x_pos &&
	//		InputSystem::Get().GetMousePos().x < m_opts.x_pos + m_opts.width &&
	//		InputSystem::Get().GetMousePos().y > m_opts.y_pos &&
	//		InputSystem::Get().GetMousePos().y < m_opts.y_pos + m_opts.height)
	//	{
	//		return ElementState::INSIDE;
	//	}
	//	else
	//		return ElementState::OUTSIDE;
	//}

	return ElementState::NONE;
}

bool rtd::Border::CheckHover()
{
	return false;
}
