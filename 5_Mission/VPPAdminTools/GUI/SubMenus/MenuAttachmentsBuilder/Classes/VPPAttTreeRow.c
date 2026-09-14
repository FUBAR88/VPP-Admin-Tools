class VPPAttTreeNode
{
	string m_ClassName;
	string m_Label;
	bool m_IsHeader;
	bool m_Checked;
	bool m_Expanded;
	int m_Depth;
	ref array<ref VPPAttTreeNode> m_Children;

	void VPPAttTreeNode(string className, string label, bool isHeader, int depth)
	{
		m_ClassName = className;
		m_Label = label;
		m_IsHeader = isHeader;
		m_Checked = false;
		m_Expanded = false;
		m_Depth = depth;
		m_Children = new array<ref VPPAttTreeNode>;
	}
};

class VPPAttTreeRow: ScriptedWidgetEventHandler
{
	protected Widget m_Root;
	protected Widget m_SelFill;
	protected CheckBoxWidget m_Chk;
	protected TextWidget m_Txt;
	protected TextWidget m_TxtToggle;
	protected ButtonWidget m_BtnView;
	protected ImageWidget m_ImgView;
	protected Managed m_MenuHost;
	protected VPPAttTreeNode m_Node;
	protected bool m_Selected;

	void VPPAttTreeRow(Widget parent, Managed menu, VPPAttTreeNode node)
	{
		m_MenuHost = menu;
		m_Node = node;
		m_Selected = false;

		if (!g_Game)
			return;

		m_Root = g_Game.GetWorkspace().CreateWidgets(VPPATUIConstants.AttTreeRow, parent);
		if (!m_Root)
			return;

		m_Root.SetHandler(this);
		m_SelFill = m_Root.FindAnyWidget("SelFill");
		m_Chk = CheckBoxWidget.Cast(m_Root.FindAnyWidget("ChkItem"));
		m_Txt = TextWidget.Cast(m_Root.FindAnyWidget("TxtItem"));
		m_TxtToggle = TextWidget.Cast(m_Root.FindAnyWidget("TxtToggle"));
		m_BtnView = ButtonWidget.Cast(m_Root.FindAnyWidget("BtnView"));
		if (m_BtnView)
		{
			m_BtnView.SetHandler(this);
			m_ImgView = ImageWidget.Cast(m_BtnView.FindAnyWidget("ImgView"));
		}

		if (m_Chk)
			m_Chk.SetHandler(this);

		ApplyNode();
	}

	void ~VPPAttTreeRow()
	{
		if (m_Root)
			m_Root.Unlink();
	}

	void ApplyNode()
	{
		if (!m_Node)
			return;

		float indent = 6 + (m_Node.m_Depth * 14);
		if (m_Chk)
		{
			if (m_Node.m_IsHeader)
			{
				m_Chk.Show(false);
			}
			else
			{
				m_Chk.Show(true);
				m_Chk.SetChecked(m_Node.m_Checked);
				m_Chk.SetPos(indent, 2);
			}
		}

		if (m_TxtToggle)
		{
			if (m_Node.m_IsHeader)
			{
				m_TxtToggle.Show(true);
				m_TxtToggle.SetPos(indent, 0);
				if (m_Node.m_Expanded)
					m_TxtToggle.SetText("-");
				else
					m_TxtToggle.SetText("+");
			}
			else
			{
				m_TxtToggle.Show(false);
			}
		}

		if (m_Txt)
		{
			if (m_Node.m_IsHeader)
			{
				m_Txt.SetPos(indent + 16, 0);
				m_Txt.SetColor(ARGB(255, 232, 163, 61));
				m_Txt.SetText(m_Node.m_Label);
			}
			else
			{
				m_Txt.SetPos(indent + 26, 0);
				m_Txt.SetColor(ARGB(255, 225, 228, 231));
				m_Txt.SetText(m_Node.m_ClassName);
			}
		}

		if (m_BtnView)
		{
			if (m_Node.m_IsHeader)
				m_BtnView.Show(false);
			else
				m_BtnView.Show(true);
		}

		SetViewActive(false);
	}

	override bool OnClick(Widget w, int x, int y, int button)
	{
		if (w == m_Chk)
			return false;

		if (w == m_BtnView || IsViewWidget(w))
		{
			NotifyView();
			return true;
		}

		return false;
	}

	override bool OnMouseButtonDown(Widget w, int x, int y, int button)
	{
		if (w == m_Chk)
			return false;

		if (w == m_BtnView || IsViewWidget(w))
			return true;

		NotifySelect();
		return true;
	}

	override bool OnChange(Widget w, int x, int y, bool finished)
	{
		if (w != m_Chk)
			return false;

		if (!m_Node)
			return true;

		if (m_Node.m_IsHeader)
			return true;

		m_Node.m_Checked = m_Chk.IsChecked();
		NotifyTick();
		return true;
	}

	void NotifySelect()
	{
		if (!g_Game)
			return;

		if (!m_MenuHost)
			return;

		g_Game.GameScript.CallFunction(m_MenuHost, "OnAttTreeRowClick", NULL, this);
	}

	void NotifyTick()
	{
		if (!g_Game)
			return;

		if (!m_MenuHost)
			return;

		g_Game.GameScript.CallFunction(m_MenuHost, "OnAttTreeTick", NULL, this);
	}

	void NotifyView()
	{
		if (!g_Game)
			return;

		if (!m_MenuHost)
			return;

		g_Game.GameScript.CallFunction(m_MenuHost, "OnAttTreeRowView", NULL, this);
	}

	bool IsViewWidget(Widget w)
	{
		if (!w)
			return false;

		if (w == m_BtnView || w == m_ImgView)
			return true;

		Widget parent = w.GetParent();
		if (parent == m_BtnView)
			return true;

		return false;
	}

	void SetViewActive(bool state)
	{
		if (!m_ImgView)
			return;

		if (state)
			m_ImgView.SetColor(ARGB(255, 232, 163, 61));
		else
			m_ImgView.SetColor(ARGB(255, 154, 160, 168));
	}

	void SetSelected(bool state)
	{
		m_Selected = state;
		if (m_SelFill)
			m_SelFill.Show(state);

		if (!m_Root)
			return;

		if (state)
			m_Root.SetColor(ARGB(255, 232, 163, 61));
		else
			m_Root.SetColor(ARGB(216, 27, 30, 34));
	}

	void SyncCheck()
	{
		if (!m_Chk || !m_Node)
			return;

		if (m_Node.m_IsHeader)
			return;

		m_Chk.SetChecked(m_Node.m_Checked);
	}

	VPPAttTreeNode GetNode()
	{
		return m_Node;
	}

	Widget GetRoot()
	{
		return m_Root;
	}
};
