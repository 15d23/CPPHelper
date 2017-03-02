#pragma once
#include <Windows.h>
#include <windef.h>

#include <atlstr.h>
#include <atldef.h>
#include <atltypes.h>
#include <atlbase.h>
#include <atlctrls.h>
#include <atlapp.h>
#include <atlwin.h>

#include <atldlgs.h>
#include <DoubleBuffer.h>


#define NM_GetTipToolInfo NM_FIRST-50
using namespace WTL;

struct TipTooInfo
{
	NMHDR hr;
	HTREEITEM Item;
	LPCWSTR String;
};

//#include <atlctrls.h>


#define NM_LVC_EditItem NM_FIRST-51

struct LVCEditItemInfo
{
	NMHDR hr;
	int iItem;
	int SubItem;
	BOOL NeedEdit;
};



class CListViewCtrl2 :public CWindowImpl < CListViewCtrl2, CListViewCtrl >
{
public:
	CDoubleBuffer mDoubleBuffer;


	BEGIN_MSG_MAP_EX(CListViewCtrl2)
		CHAIN_MSG_MAP_MEMBER(mDoubleBuffer);
	END_MSG_MAP()
};

static void* pData;

class CListViewCtrlEdit :public CWindowImpl <CListViewCtrlEdit, CListViewCtrl>
{
	class CEditForListViewCtrl :public CWindowImpl <CEditForListViewCtrl, CEdit>
	{
		CString Buffer;

		BOOL m_bExchange;//�Ƿ�������ݽ���
		int nItem, nIndex;
		static LRESULT CALLBACK GetMsgProc(int code,       // hook code
			WPARAM wParam,  // removal option
			LPARAM lParam   // message
			)
		{
			auto pPage = (CEditForListViewCtrl*)pData;

			auto pMsg = (MSG*)lParam;
			if (pPage->PreTranslateMessage(pMsg))
			{
				pMsg->message = WM_NULL;
				return 0;
			}

			return CallNextHookEx(pPage->g_hook, code, wParam, lParam);
		}
	public:
		HHOOK g_hook;

		/*virtual WNDPROC GetWindowProc()
		{
			return MyWindowProc;
		}*/

		BOOL PreTranslateMessage(MSG* pMsg)
		{
			if (m_hWnd == pMsg->hwnd&& pMsg->message == WM_KEYDOWN)
			{
				switch (pMsg->wParam)
				{
				case VK_ESCAPE:
					//�����ȡ������ô����������
					m_bExchange = FALSE;
				case VK_RETURN:
					ShowWindow(0);

					return TRUE;
				default:
					break;
				}
			}
			return FALSE;
		}
		
		BEGIN_MSG_MAP_EX(CEditForListViewCtrl)
			MSG_WM_KILLFOCUS(OnKillFocus)
			MSG_WM_SHOWWINDOW(OnShowWindow)
			//DEFAULT_REFLECTION_HANDLER()
		END_MSG_MAP()

		void OnKillFocus(CWindow wndFocus)
		{
			//

			
			if (IsWindowVisible()==0)
			{
				SetMsgHandled(FALSE);
			}
			else
			{
				m_bExchange = TRUE;
				ShowWindow(0);
			}
			//else
			//{
				//ShowWindow(FALSE);
				//this->
				//SetMsgHandled(TRUE);
				//SetFocus();
			//}
		}

		LRESULT OnNotify(int idCtrl, LPNMHDR pnmh)
		{
			SetMsgHandled(FALSE);

			return 0;
		}
		void OnShowWindow(BOOL bShow, UINT nStatus)
		{
			SetMsgHandled(FALSE);
			CListViewCtrl List(GetParent());
			List.GetHeader().EnableWindow(!bShow);

			if (bShow)
			{
				pData = this;

				g_hook = SetWindowsHookEx(WH_GETMESSAGE, GetMsgProc, GetModuleHandle(NULL), GetCurrentThreadId());

				m_bExchange = TRUE;
			}
			else
			{
				UnhookWindowsHookEx(g_hook);

				if (m_bExchange)
				{
					//�ύ���ݸ���
					
						
					GetWindowText(Buffer);

					List.SetItemText(nItem, nIndex, Buffer);
				}
			}
		}


		void ShowEdit(BOOL bShow, int nItem, int nIndex, CListViewCtrlEdit& List)
		{
			// ����༭�������δ����
			if (m_hWnd == NULL)
			{
				if (bShow == FALSE)
					return;

				//����һ���༭�򣨴�СΪ�㣩
				Create(List.m_hWnd, 0, NULL, ES_AUTOHSCROLL | WS_CHILD | ES_LEFT
					| ES_WANTRETURN | WS_BORDER, 0, 1000);

				//CWindow(m_hWnd).ModifyStyle()
				//m_Edit.ShowWindow(SW_HIDE);// ����

				//ʹ��Ĭ������
				//CFont tpFont(DEFAULT_GUI_FONT);

				//tpFont.CreateFontWCreate//StockObject(DEFAULT_GUI_FONT);
				//m_edit.SetFont(&tpFont);
				//tpFont.DeleteObject();
			}

			if (IsWindowVisible() == bShow)
				return;

			//���bShowΪtrue����ʾ�༭��
			if (bShow == TRUE)
			{
				this->nItem = nItem;
				this->nIndex = nIndex;

				CRect rcCtrl;        //��������������
				

				SetWindowTextW(List.GetItemText(nItem, nIndex));

				//CString strItem = CListCtrl::GetItemText(nItem, nIndex);//��ȡ�б�ؼ��������������
				//m_Edit.SetCtrlData(nItem, nIndex, CListCtrl::GetItemText(nItem, nIndex));
				
				List.GetSubItemRect(nItem, nIndex, LVIR_LABEL, rcCtrl);
				rcCtrl.bottom += 3;
				MoveWindow(rcCtrl);// �ƶ���������������
				ShowWindow(SW_SHOW);//��ʾ�ؼ�
				//m_edit.SetWindowText(strItem);// ��ʾ����
				SetFocus();//���ý���

				
				//::SendMessage(m_Edit, EM_SETSEL, 0, -1);//ʹ���ݴ���ѡ��״̬

				SetSelAll();
			}
			else
			{
				ShowWindow(SW_HIDE);
			}
		}

	} m_Edit;

	
	CToolTipCtrl m_pToolTip;


	TipTooInfo TipInfo;

	
public:
	

	BOOL IsEditShow()
	{
		if (m_Edit.m_hWnd)
		{
			return m_Edit.IsWindowVisible();
		}
		else
			return FALSE;
	}


	/*BOOL PreTranslateMessage(MSG* pMsg)
	{
		return m_Edit.PreTranslateMessage(pMsg);
	}*/
	//CDoubleBuffer mDoubleBuffer;


	BEGIN_MSG_MAP_EX(CListViewCtrlEdit)
		MSG_WM_LBUTTONDBLCLK(OnLButtonDblClk)
		MSG_WM_LBUTTONDOWN(OnLButtonDown)
		MSG_WM_MOUSEMOVE(OnMouseMove)
		//CHAIN_MSG_MAP_MEMBER(mDoubleBuffer);
	END_MSG_MAP()

	void OnLButtonDblClk(UINT nFlags, CPoint point)
	{
		SetMsgHandled(FALSE);

		LVHITTESTINFO lvhti; //�����б�ؼ�������������Ե����ݽṹ
		lvhti.pt = point;  //�������λ��
		int nItem = SubItemHitTest(&lvhti);//���û����������Ժ����������к�
		if (nItem == -1)   //�������ڿؼ���˫���������κδ���
			return;


		LVCEditItemInfo ItemInfo{ NMHDR{ m_hWnd, GetDlgCtrlID(), NM_LVC_EditItem }, nItem, lvhti.iSubItem, TRUE };


		::SendMessage(GetParent(), WM_NOTIFY, 0, (LPARAM)&ItemInfo);

		if (ItemInfo.NeedEdit == FALSE)
			return;
	
		m_Edit.ShowEdit(TRUE, nItem, lvhti.iSubItem, *this); //�����Զ��庯������ʾ�༭��
		
	}

	void OnLButtonDown(UINT nFlags, CPoint point)
	{

		
		SetMsgHandled(FALSE);

		if (m_Edit.m_hWnd)
		{
			m_Edit.ShowWindow(0);
		}
	}

	BOOL SubclassWindow(HWND hWnd)
	{
		if (hWnd&&CWindowImpl<CListViewCtrlEdit, CListViewCtrl>::SubclassWindow(hWnd))
		{
			

			TipInfo.hr.code = NM_GetTipToolInfo;
			TipInfo.hr.hwndFrom = m_hWnd;
			TipInfo.hr.idFrom = CWindow(m_hWnd).GetDlgCtrlID();
			TipInfo.Item = (HTREEITEM)-1;

			m_pToolTip.Create(m_hWnd, 0, NULL, TTS_ALWAYSTIP);

			m_pToolTip.AddTool(m_hWnd, LPSTR_TEXTCALLBACK);
			this->SetToolTips(m_pToolTip);

			m_pToolTip.Activate(TRUE);

			//m_pToolTip.SetTipBkColor(GetSysColor(COLOR_WINDOW));
			//m_pToolTip.SetTipTextColor(GetSysColor(COLOR_HIGHLIGHTTEXT));
			m_pToolTip.SetDelayTime(TTDT_AUTOPOP, 10000);
			m_pToolTip.SetMaxTipWidth(300);

			return TRUE;
		}
		else
		{
			return FALSE;
		}
	}

	void OnMouseMove(UINT nFlags, CPoint point)
	{
		SetMsgHandled(FALSE);

		UINT temp;
		HTREEITEM hItem = (HTREEITEM)HitTest(point, &temp);
		
		if (temp & LVHT_ONITEM)
		{
			if (TipInfo.Item != hItem)
			{
				TipInfo.Item = hItem;
				//m_pToolTip.SetWindowTextW(L"����");
				//m_pToolTip.Activate(1);
				//
				//m_pToolTip.ShowWindow(1);
				TipInfo.String = NULL;


				::SendMessage(GetParent(), WM_NOTIFY, 0, (LPARAM)&TipInfo);

				if (TipInfo.String)
				{

					//m_pToolTip.SetToolInfo()
					m_pToolTip.Activate(1);
					m_pToolTip.UpdateTipText(TipInfo.String, m_hWnd);

					//m_pToolTip.ShowWindow(1);

				}
				else
				{
					//m_pToolTip.ShowWindow(0);
					//m_pToolTip.Pop();
					m_pToolTip.Activate(0);
				}
			}

			return;

		}
		else
		{
			TipInfo.Item = (HTREEITEM)-1;
		}

		m_pToolTip.Activate(0);
		//m_pToolTip.Pop();
		//m_pToolTip.ShowWindow(0);
		//m_pToolTip.Activate(0);

		//SetMsgHandled(FALSE);
	}
};