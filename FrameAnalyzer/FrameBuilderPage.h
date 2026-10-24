#pragma once

#include "resource.h"

class CFrameBuilderPage : public CDialogEx
{
public:
    CFrameBuilderPage(CWnd* pParent = nullptr);

#ifdef AFX_DESIGN_TIME
    enum { IDD = IDD_FRAME_BUILDER_PAGE };
#endif

protected:
    virtual void DoDataExchange(CDataExchange* pDX);
    virtual BOOL OnInitDialog();

    DECLARE_MESSAGE_MAP()
};