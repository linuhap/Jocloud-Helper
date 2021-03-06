// MainDlg.cpp : implementation of the UIMainDlg class
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "uiMainDlg.h"

UIMainDlg::UIMainDlg() : SHostWnd(UIRES.LAYOUT.dlg_main) {
	_oLogicLanguageTranslate.setSWindow(this);

	registerEvent();

	registerScene();
	_pThePageUsing = NULL;
}

UIMainDlg::~UIMainDlg() {
}


BOOL UIMainDlg::onInitDialog(HWND hWnd, LPARAM lParam) {
	loadConfig();

	int number = getBuildNumber();
	if (number != -1) {
		SStringW strTitle = SStringW().Format(_T("%s"), GETSTRING(R.string.title));
		FindChildByID(R.id.txt_title)->SetWindowText(S_CW2T(strTitle));

		wchar_t RaysVersion[128] = { 0 };
		SApplication::getSingleton().GetTranslator()->tr(L"left_version", L"dlg_main", RaysVersion, 128);

		wchar_t SDKVersion[128] = { 0 };
		SApplication::getSingleton().GetTranslator()->tr(L"left_thunder_rts_version", L"dlg_main", SDKVersion, 128);

		SStringW strVersionTitle = SStringW().Format(_T("%s #%d %s"), RaysVersion, number, SDKVersion);
		FindChildByID(R.id.build_number)->SetWindowText(S_CW2T(strVersionTitle));
	}

	_oLogicPageManager.init(std::bind(&UIMainDlg::onPageEventHandler, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3), this);
	return 0;
}

void UIMainDlg::onClose() {
	SNativeWnd::DestroyWindow();
}

void UIMainDlg::onMaximize() {
	SendMessage(WM_SYSCOMMAND, SC_MAXIMIZE);
}

void UIMainDlg::onRestore() {
	SendMessage(WM_SYSCOMMAND, SC_RESTORE);
}

void UIMainDlg::onMinimize() {
	SendMessage(WM_SYSCOMMAND, SC_MINIMIZE);
}

void UIMainDlg::onSize(UINT nType, CSize size) {
	SetMsgHandled(FALSE);	
	//SWindow *pBtnMax = FindChildByName(L"btn_max");
	SWindow *pBtnRestore = FindChildByName(L"btn_restore");
	//if(!pBtnMax || !pBtnRestore) return;
	if (!pBtnRestore)return;
	if (nType == SIZE_MAXIMIZED)
	{
		pBtnRestore->SetVisible(TRUE);
		//pBtnMax->SetVisible(FALSE);
	}
	else if (nType == SIZE_RESTORED)
	{
		pBtnRestore->SetVisible(FALSE);
		//pBtnMax->SetVisible(TRUE);
	}
}

/// ============================== private ==============================
void UIMainDlg::switchLanguage(UIPageTranslateLanguage language) {
	_oLogicLanguageTranslate.switchLanguate(language);
	_oLogicPageManager.switchLanguage();
	this->switchLanguage();
}

void UIMainDlg::switchLanguage() {
	// use this to switch language
	//SStringW str1 = _T("setting_quit");
	//SStringW str2 = _T("dlg_main");
	//FindChildByID(R.id.txt_title)->SetWindowText(S_CW2T(TR(str1, str2)));
}

void UIMainDlg::switchPage(IUIBasePage* from, IUIBasePage* to) {
	STabCtrl *pTabOp = FindChildByID2<STabCtrl>(R.id.tab_ctrls);
	SWindow *dow = FindChildByName(L"theAppMainWindow");
	SWindow *dowRight = FindChildByID(R.id.main_window_right_dlg);
	
	if (to->getScenePageType() == UIScenePageType::SCENE_PAGE_NULL)
	{
		from->beforeDisappear();
		pTabOp->SetCurSel(0);
		dowRight->SetVisible(FALSE, TRUE);
		return;
	}
	else
	{
		dowRight->SetVisible(TRUE, TRUE);
	}
	
	if (to->getScenePageType() != UIScenePageType::SCENE_PAGE_NULL && from->getScenePageType() != UIScenePageType::SCENE_PAGE_NULL)return;
	from->beforeDisappear();
	int index = int(to->getScenePageType()) - int(UIScenePageType::SCENE_PAGE_NULL);
	pTabOp->SetCurSel(index);
	to->beforeAppear();
	_pThePageUsing = to;

	
}

void UIMainDlg::onPageEventHandler(UIPageEvent event, void* eventParam, IUIBasePage* source) {
	int e = int(int(event) - int(UIPageEvent::PAGE_EVENT_DISAPPEAR_INITIATIVE));
	_oEventHandler[e](event, eventParam, source);
}

void UIMainDlg::onPosChanged(CPoint ptPos)
{
	SHostWnd::UpdateAutoSizeCount(true);
	BOOL bHandle;
	onThunderManagerCB(WM_WND_POSCHANGE, NULL, NULL, bHandle);
	//SetMsgHandled(FALSE);
	SHostWnd::UpdateAutoSizeCount(false);
}

void UIMainDlg::loadConfig() {
	new ConfigFile();
	bool ret = ConfigFile::getSingletonPtr()->parse(L"./config.ini");
	if (!ret) {
		SMessageBox(NULL, SStringT().Format(_T("%s"), _T("config.ini open failed")), TR(_T("messgebox_title_warning"), _T("dlg_main")), MB_OK);
		return;
	}
}

void UIMainDlg::registerEvent() {
	_oEventHandler[int(int(UIPageEvent::PAGE_EVENT_DISAPPEAR_INITIATIVE) - int(UIPageEvent::PAGE_EVENT_DISAPPEAR_INITIATIVE))] =
		std::bind(&UIMainDlg::onEventDisappearInitiative, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
	_oEventHandler[int(int(UIPageEvent::PAGE_EVENT_SWITCH_LANGUAGE) - int(UIPageEvent::PAGE_EVENT_DISAPPEAR_INITIATIVE))] =
		std::bind(&UIMainDlg::onEventSwitchLanguage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
	_oEventHandler[int(int(UIPageEvent::PAGE_EVENT_SWITCH_SCENE) - int(UIPageEvent::PAGE_EVENT_DISAPPEAR_INITIATIVE))] =
		std::bind(&UIMainDlg::onEventSwitchScene, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
}

void UIMainDlg::registerScene() {
	_oLogicPageManager.registerPage(UIScenePageType::SCENE_PAGE_LEFT);
	_oLogicPageManager.registerPage(UIScenePageType::SCENE_PAGE_NULL);
	_oLogicPageManager.registerPage(UIScenePageType::SCENE_PAGE_VIDEO_SAME_ROOM);
	_oLogicPageManager.registerPage(UIScenePageType::SCENE_PAGE_VIDEO_DIFFERENT_ROOM);
}

/// ============ event handler ============
void UIMainDlg::onEventDisappearInitiative(UIPageEvent event, void* eventParam, IUIBasePage* source) {
	// 1. switch scene to null page
	switchPage(source, _oLogicPageManager.getPageFromType(UIScenePageType::SCENE_PAGE_NULL));

	// 2. tell left page switch to null page
	_oLogicPageManager.getPageFromType(UIScenePageType::SCENE_PAGE_LEFT)->switchScene(source->getScenePageType(), UIScenePageType::SCENE_PAGE_NULL);
}

void UIMainDlg::onEventSwitchLanguage(UIPageEvent event, void* eventParam, IUIBasePage* source) {
	switchLanguage(*(UIPageTranslateLanguage*)(eventParam));
}

void UIMainDlg::onEventSwitchScene(UIPageEvent event, void* eventParam, IUIBasePage* source) {
	auto e = (UISceneSwitchEvent*)eventParam;
	switchPage(_oLogicPageManager.getPageFromType(e->_eFrom), _oLogicPageManager.getPageFromType(e->_eTo));
}

///  ============================= register event ==============================
void UIMainDlg::onInitGroup(EventArgs *e) {
	//EventGroupListInitGroup *e2 = sobj_cast<EventGroupListInitGroup>(e);
	//SToggle *pTgl = e2->pItem->FindChildByID2<SToggle>(R.id.tgl_switch);
	//pTgl->SetToggle(!e2->pGroupInfo->bCollapsed);
	//e2->pItem->FindChildByID(R.id.txt_label)->SetWindowText(e2->pGroupInfo->strText);
}

void UIMainDlg::onInitItem(EventArgs *e) {
	//EventGroupListInitItem *e2 = sobj_cast<EventGroupListInitItem>(e);
	//e2->pItem->FindChildByID(R.id.txt_label)->SetWindowText(e2->pItemInfo->strText);
	//e2->pItem->FindChildByID2<SImageWnd>(R.id.img_indicator)->SetIcon(e2->pItemInfo->iIcon);
}

void UIMainDlg::onGroupStateChanged(EventArgs *e) {
	//EventGroupStateChanged *e2 = sobj_cast<EventGroupStateChanged>(e);
	//SToggle *pTgl = e2->pItem->FindChildByID2<SToggle>(R.id.tgl_switch);
	//pTgl->SetToggle(!e2->pGroupInfo->bCollapsed);
}

void UIMainDlg::onCtrlPageClick(EventArgs *e) {
	//EventGroupListItemCheck *e2 = sobj_cast<EventGroupListItemCheck>(e);
	//STabCtrl *pTabOp = FindChildByID2<STabCtrl>(R.id.tab_ctrls);
	//int nIndex = e2->pItemInfo->id - 100;// R.id.page_quick_same_room;
	//pTabOp->SetCurSel(nIndex);
}


LRESULT UIMainDlg::onThunderManagerCB(UINT uMsg, WPARAM wp, LPARAM lp, BOOL &bHandle)
{
	if (WM_THUNDER_MESSAGE == uMsg && _pThePageUsing != NULL)
	{
		ThunderMessage* msg = (ThunderMessage*)lp;
		if (msg)
		{
			_pThePageUsing->onThunderManagerCB(msg->dwEventID, 0, (LPARAM)msg->lpBuffer, bHandle);
		}
	}
	else if(WM_WND_POSCHANGE == uMsg)
	{
		if(_pThePageUsing)_pThePageUsing->onThunderManagerCB(WM_WND_POSCHANGE, wp, lp, bHandle);
	}
	return 0;
}

