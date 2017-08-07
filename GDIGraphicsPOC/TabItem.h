#include "stdafx.h"
#include "windows.h"
#include <string>

#define WIN32_LEAN_AND_MEAN

#include <Shellapi.h>

#include <atlapp.h>
#include <atlctrls.h>
#include <atlctrlx.h>

#include <atlbase.h> // Base ATL classes
#include <atlwin.h> // ATL windowing classes
#include <atlsplit.h>
#include <atlframe.h>
#include "CobaltFusion/dbgstream.h"
#include "CobaltFusion/Str.h"
#include "CobaltFusion/stringbuilder.h"
#include "Logview.h"
#include <iomanip>

#pragma warning(push, 3)
#pragma warning(disable : 4838)


#include <atlgdix.h> // CustomTabCtrl.h prerequisites
#include "atlcoll.h"
#include "CustomTabCtrl.h"
#include "DotNetTabCtrl.h"
#include "TabbedFrame.h"

#include "AtlWinExt.h"
#pragma warning(pop)

#include "DebugView++Lib/TimelineDC.h"
#include "DebugView++Lib/CTimelineView.h"

namespace fusion {



std::wstring FormatUnits(int n, const std::wstring& unit)
{
    if (n == 0)
        return L"";
    if (n == 1)
        return wstringbuilder() << n << " " << unit;
    return wstringbuilder() << n << " " << unit << "s";
}

std::wstring FormatDuration(double seconds)
{
    int minutes = FloorTo<int>(seconds / 60);
    seconds -= 60 * minutes;

    int hours = minutes / 60;
    minutes -= 60 * hours;

    int days = hours / 24;
    hours -= 24 * days;

    if (days > 0)
        return wstringbuilder() << FormatUnits(days, L"day") << L" " << FormatUnits(hours, L"hour");

    if (hours > 0)
        return wstringbuilder() << FormatUnits(hours, L"hour") << L" " << FormatUnits(minutes, L"minute");

    if (minutes > 0)
        return wstringbuilder() << FormatUnits(minutes, L"minute") << L" " << FormatUnits(FloorTo<int>(seconds), L"second");

    static const wchar_t* units[] = { L"s", L"ms", L"�s", L"ns", nullptr };
    const wchar_t** unit = units;
    while (*unit != nullptr && seconds > 0 && seconds < 1)
    {
        seconds *= 1e3;
        ++unit;
    }

    return wstringbuilder() << std::fixed << std::setprecision(3) << seconds << L" " << *unit;
}

class CLogViewTabItem2 : public CTabViewTabItem
{
public:
	CLogView& GetView();
	gdi::CTimelineView& GetTimeLineView() { return m_timelineView; }
	void Create(HWND parent);
    void InitTimeLine();
private:
	
	CLogView m_logview;
	CHorSplitterWindow m_split;
	CPaneContainer m_top;
	CPaneContainer m_bottom;
	gdi::CTimelineView m_timelineView;
};

void CLogViewTabItem2::InitTimeLine()
{
	m_timelineView.SetFormatter([](gdi::Location l) {
		return Str(FormatDuration(l));
	});

	m_timelineView.SetDataProvider([](gdi::Location, gdi::Location) {
		auto info = std::make_shared<gdi::Line>(L"Some info");
		info->Add(gdi::Artifact(650, gdi::Artifact::Type::Flag, RGB(255, 0, 0)));
		info->Add(gdi::Artifact(700, gdi::Artifact::Type::Flag, RGB(255, 0, 0), RGB(0, 255, 0)));
		info->Add(gdi::Artifact(750, gdi::Artifact::Type::Flag));
		info->Add(gdi::Artifact(800, gdi::Artifact::Type::Flag));
		info->Add(gdi::Artifact(850, gdi::Artifact::Type::Flag));
		info->Add(gdi::Artifact(992, gdi::Artifact::Type::Flag));

		auto sequence = std::make_shared<gdi::Line>(L"Move Sequence");
		sequence->Add(gdi::Artifact(615, gdi::Artifact::Type::Flag, RGB(160, 160, 170)));
		sequence->Add(gdi::Artifact(632, gdi::Artifact::Type::Flag, RGB(160, 160, 170)));
		sequence->Add(gdi::Artifact(636, gdi::Artifact::Type::Flag, RGB(255, 0, 0), RGB(0, 255, 0)));
		sequence->Add(gdi::Artifact(640, gdi::Artifact::Type::Flag, RGB(255, 0, 0)));

		auto data = std::make_shared<gdi::Line>(L"Arbitrary data");
		data->Add(gdi::Artifact(710, gdi::Artifact::Type::Flag, RGB(0, 0, 255)));
		info->Add(gdi::Artifact(701, gdi::Artifact::Type::Flag));

		gdi::TimeLines lines;
		lines.emplace_back(info);
		lines.emplace_back(sequence);
		lines.emplace_back(data);
		return lines;
	});
}

void CLogViewTabItem2::Create(HWND parent)
{
	auto client = RECT();
	GetClientRect(parent, &client);

	m_split.Create(parent, client);
	SetTabView(m_split);

	m_top.Create(m_split, L"top panel");
	GetClientRect(m_top, &client);

	m_logview.Create(m_top, client);
	m_top.SetClient(m_logview);
	m_top.SetTitle(L"top label");

	m_bottom.Create(m_split, L"bottom panel");
	m_split.SetSplitterPanes(m_top, m_bottom, true);
	m_split.SetSplitterPos(500);

	InitTimeLine();

	m_timelineView.Create(m_bottom, CWindow::rcDefault, gdi::CTimelineView::GetWndClassName(),
		WS_CHILD | WS_VISIBLE | WS_HSCROLL | WS_VSCROLL | SS_OWNERDRAW);

	m_timelineView.SetView(0.0, 1000.0);
	m_bottom.SetClient(m_timelineView); //uncomment this line to start rendering m_timelineView (breaks because it needs to be configured)
}

CLogView& CLogViewTabItem2::GetView()
{
	return m_logview;
}

} // namespace fusion
