class VPPInvItemRow: ScriptedWidgetEventHandler
{
	protected Widget m_Root;
	protected ItemPreviewWidget m_Preview;
	protected TextWidget m_TxtClass;
	protected TextWidget m_TxtQtyLabel;
	protected EditBoxWidget m_EditQty;
	protected ButtonWidget m_BtnSetQty;
	protected ButtonWidget m_BtnMaxQty;
	protected ButtonWidget m_BtnMinQty;
	protected TextWidget m_TxtHealth;
	protected ButtonWidget m_BtnLowerHealth;
	protected ButtonWidget m_BtnRaiseHealth;
	protected ButtonWidget m_BtnDelete;
	protected ButtonWidget m_BtnTake;
	protected Managed m_MenuHost;
	protected ref VPPInvItemInfo m_Info;
	protected EntityAI m_PreviewObject;

	void VPPInvItemRow(Widget parent, Managed menu, VPPInvItemInfo info)
	{
		m_MenuHost = menu;
		m_Info = info;

		if (!g_Game)
			return;

		m_Root = g_Game.GetWorkspace().CreateWidgets(VPPATUIConstants.InvItemRow, parent);
		if (!m_Root)
			return;

		m_Root.SetHandler(this);
		m_Preview = ItemPreviewWidget.Cast(m_Root.FindAnyWidget("ItemPreview"));
		m_TxtClass = TextWidget.Cast(m_Root.FindAnyWidget("TxtClassName"));
		m_TxtQtyLabel = TextWidget.Cast(m_Root.FindAnyWidget("TxtQtyLabel"));
		m_EditQty = EditBoxWidget.Cast(m_Root.FindAnyWidget("EditQty"));
		m_BtnSetQty = ButtonWidget.Cast(m_Root.FindAnyWidget("BtnSetQty"));
		m_BtnMaxQty = ButtonWidget.Cast(m_Root.FindAnyWidget("BtnMaxQty"));
		m_BtnMinQty = ButtonWidget.Cast(m_Root.FindAnyWidget("BtnMinQty"));
		m_TxtHealth = TextWidget.Cast(m_Root.FindAnyWidget("TxtHealth"));
		m_BtnLowerHealth = ButtonWidget.Cast(m_Root.FindAnyWidget("BtnLowerHealth"));
		m_BtnRaiseHealth = ButtonWidget.Cast(m_Root.FindAnyWidget("BtnRaiseHealth"));
		m_BtnDelete = ButtonWidget.Cast(m_Root.FindAnyWidget("BtnDelete"));
		m_BtnTake = ButtonWidget.Cast(m_Root.FindAnyWidget("BtnTake"));
		ApplyInfo();
	}

	void ~VPPInvItemRow()
	{
		ClearPreview();
		if (m_Root)
			m_Root.Unlink();
	}

	void ApplyInfo()
	{
		if (!m_Info)
			return;

		if (m_TxtClass)
			m_TxtClass.SetText(m_Info.m_ClassName);

		if (m_EditQty)
			m_EditQty.SetText(m_Info.m_Quantity.ToString());

		bool stackable = m_Info.m_Stackable;
		if (m_TxtQtyLabel)
			m_TxtQtyLabel.Show(stackable);

		if (m_EditQty)
			m_EditQty.Show(stackable);

		if (m_BtnSetQty)
			m_BtnSetQty.Show(stackable);

		if (m_BtnMaxQty)
			m_BtnMaxQty.Show(stackable);

		if (m_BtnMinQty)
			m_BtnMinQty.Show(stackable);

		ApplyHealth();
		ShowPreview();
	}

	void ApplyHealth()
	{
		if (!m_TxtHealth || !m_Info)
			return;

		int level = m_Info.m_HealthLevel;
		string label = "P";
		int color = ARGB(255, 34, 139, 34);
		if (level == 1)
		{
			label = "W";
			color = ARGB(255, 154, 205, 50);
		}
		else if (level == 2)
		{
			label = "D";
			color = ARGB(255, 255, 255, 0);
		}
		else if (level == 3)
		{
			label = "BD";
			color = ARGB(255, 255, 69, 0);
		}
		else if (level == 4)
		{
			label = "R";
			color = ARGB(255, 139, 0, 0);
		}

		m_TxtHealth.SetText(label);
		m_TxtHealth.SetColor(color);
	}

	void ShowPreview()
	{
		ClearPreview();
		if (!m_Info || m_Info.m_ClassName == string.Empty)
			return;

		if (!VPPAttCatalog.Get().CanSpawnLocalPreview(m_Info.m_ClassName))
			return;

		m_PreviewObject = EntityAI.Cast(g_Game.CreateObjectEx(m_Info.m_ClassName, vector.Zero, ECE_LOCAL));
		if (!m_PreviewObject)
			m_PreviewObject = EntityAI.Cast(g_Game.CreateObject(m_Info.m_ClassName, vector.Zero, true, false, false));

		if (!m_Preview || !m_PreviewObject)
			return;

		m_PreviewObject.DisableSimulation(true);

		m_Preview.SetItem(m_PreviewObject);
		m_Preview.SetModelPosition(Vector(0, 0, 0.5));
		m_Preview.SetModelOrientation(Vector(0, 0, 0));
		if (m_Preview.GetItem())
			m_Preview.SetView(m_Preview.GetItem().GetViewIndex());
	}

	void ClearPreview()
	{
		if (m_Preview)
			m_Preview.SetItem(NULL);

		if (m_PreviewObject)
		{
			g_Game.ObjectDelete(m_PreviewObject);
			m_PreviewObject = NULL;
		}
	}

	VPPInvItemInfo GetInfo()
	{
		return m_Info;
	}

	Widget GetRoot()
	{
		return m_Root;
	}

	int ReadQtyInput()
	{
		if (!m_EditQty)
			return 1;

		return m_EditQty.GetText().ToInt();
	}

	override bool OnClick(Widget w, int x, int y, int button)
	{
		if (!g_Game || !m_MenuHost)
			return false;

		if (w == m_BtnSetQty)
		{
			g_Game.GameScript.CallFunction(m_MenuHost, "OnInvSetQty", NULL, this);
			return true;
		}

		if (w == m_BtnMaxQty)
		{
			g_Game.GameScript.CallFunction(m_MenuHost, "OnInvMaxQty", NULL, this);
			return true;
		}

		if (w == m_BtnMinQty)
		{
			g_Game.GameScript.CallFunction(m_MenuHost, "OnInvMinQty", NULL, this);
			return true;
		}

		if (w == m_BtnLowerHealth)
		{
			g_Game.GameScript.CallFunction(m_MenuHost, "OnInvLowerHealth", NULL, this);
			return true;
		}

		if (w == m_BtnRaiseHealth)
		{
			g_Game.GameScript.CallFunction(m_MenuHost, "OnInvRaiseHealth", NULL, this);
			return true;
		}

		if (w == m_BtnDelete)
		{
			g_Game.GameScript.CallFunction(m_MenuHost, "OnInvDeleteItem", NULL, this);
			return true;
		}

		if (w == m_BtnTake)
		{
			g_Game.GameScript.CallFunction(m_MenuHost, "OnInvTakeItem", NULL, this);
			return true;
		}

		return false;
	}
};
