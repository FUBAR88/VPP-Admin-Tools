class MenuAttachmentsBuilder extends AdminHudSubMenu
{
	protected bool m_Loaded;
	protected EditBoxWidget m_SearchInputBox;
	protected TextListboxWidget m_ItemListBox;
	protected TextWidget m_TxtItemCount;
	protected TextWidget m_TxtPreviewName;
	protected ItemPreviewWidget m_ItemPreview;
	protected CheckBoxWidget m_ChkOnSelectedPlayers;
	protected ButtonWidget m_BtnSpawnItem;
	protected ButtonWidget m_BtnSelectAll;
	protected ButtonWidget m_BtnClear;
	protected ScrollWidget m_PartsScroll;
	protected GridSpacerWidget m_PartsGrid;
	protected ref VPPDropDownMenu m_CategoryDropDown;
	protected ref VPPDropDownMenu m_PlacementDropDown;
	protected Widget m_CategoryDropHost;
	protected Widget m_PlacementDropHost;
	protected ref array<string> m_CategoryLabels;
	protected ref array<string> m_PlacementLabels;
	protected int m_CurrentCategory;
	protected string m_SearchBoxStr;
	protected float m_FilterTick;
	protected int m_PrevRow;
	protected string m_SelectedParent;
	protected EntityAI m_PreviewObject;
	protected ref VPPAttTreeNode m_RootNode;
	protected ref array<ref VPPAttTreeRow> m_TreeRows;
	protected ref array<ref CustomGridSpacer> m_DataGrids;
	protected ref CustomGridSpacer m_LastGrid;
	protected VPPAttTreeRow m_SelectedRow;
	protected bool m_PreviewIsParent;
	protected bool m_PreviewBuildMode;
	protected string m_TrialClass;
	protected bool m_HideBroken;
	protected bool m_PreviewDirty;
	protected float m_PreviewDirtyTick;
	protected ref array<string> m_HandoffTargets;

	void MenuAttachmentsBuilder()
	{
		m_CategoryLabels = new array<string>;
		m_CategoryLabels.Insert(Widget.TranslateString("#VSTR_ATT_CAT_WEAPONS"));
		m_CategoryLabels.Insert(Widget.TranslateString("#VSTR_ATT_CAT_VEHICLES"));
		m_CategoryLabels.Insert(Widget.TranslateString("#VSTR_ATT_CAT_CLOTHES"));
		m_PlacementLabels = new array<string>;
		m_PlacementLabels.Insert("#VSTR_LBL_IN_INENTORY");
		m_PlacementLabels.Insert("#VSTR_LBL_ON_GROUND");
		m_PlacementLabels.Insert("#VSTR_LBL_AT_CROSSHAIRS");
		m_TreeRows = new array<ref VPPAttTreeRow>;
		m_DataGrids = new array<ref CustomGridSpacer>;
		m_CurrentCategory = VPPAttCatalog.ATT_CAT_WEAPONS;
		m_SearchBoxStr = "";
		m_FilterTick = 0;
		m_PrevRow = -2;
		m_SelectedParent = "";
		m_PreviewIsParent = false;
		m_PreviewBuildMode = false;
		m_TrialClass = "";
		m_PreviewDirty = false;
		m_PreviewDirtyTick = 0;
		m_HandoffTargets = new array<string>;
	}

	void ~MenuAttachmentsBuilder()
	{
		ClearPreview();
	}

	override void OnCreate(Widget RootW)
	{
		super.OnCreate(RootW);
		M_SUB_WIDGET = CreateWidgets(VPPATUIConstants.MenuAttachmentsBuilder);
		M_SUB_WIDGET.SetHandler(this);
		m_TitlePanel = Widget.Cast(M_SUB_WIDGET.FindAnyWidget("Header"));
		m_closeButton = ButtonWidget.Cast(M_SUB_WIDGET.FindAnyWidget("BtnClose"));
		m_SearchInputBox = EditBoxWidget.Cast(M_SUB_WIDGET.FindAnyWidget("SearchInputBox"));
		m_ItemListBox = TextListboxWidget.Cast(M_SUB_WIDGET.FindAnyWidget("ItemListBox"));
		m_TxtItemCount = TextWidget.Cast(M_SUB_WIDGET.FindAnyWidget("TxtItemCount"));
		m_TxtPreviewName = TextWidget.Cast(M_SUB_WIDGET.FindAnyWidget("TxtPreviewName"));
		m_ItemPreview = ItemPreviewWidget.Cast(M_SUB_WIDGET.FindAnyWidget("ItemPreview"));
		m_ChkOnSelectedPlayers = CheckBoxWidget.Cast(M_SUB_WIDGET.FindAnyWidget("ChkOnSelectedPlayers"));
		m_BtnSpawnItem = ButtonWidget.Cast(M_SUB_WIDGET.FindAnyWidget("BtnSpawnItem"));
		m_BtnSelectAll = ButtonWidget.Cast(M_SUB_WIDGET.FindAnyWidget("BtnSelectAll"));
		m_BtnClear = ButtonWidget.Cast(M_SUB_WIDGET.FindAnyWidget("BtnClear"));
		m_PartsScroll = ScrollWidget.Cast(M_SUB_WIDGET.FindAnyWidget("PartsScroll"));
		m_PartsGrid = GridSpacerWidget.Cast(M_SUB_WIDGET.FindAnyWidget("PartsGrid"));

		m_CategoryDropHost = M_SUB_WIDGET.FindAnyWidget("CategoryDropDownPanel");
		m_CategoryDropDown = new VPPDropDownMenu(m_CategoryDropHost, m_CategoryLabels.Get(0));
		int i;
		for (i = 0; i < m_CategoryLabels.Count(); i++)
		{
			m_CategoryDropDown.AddElement(m_CategoryLabels.Get(i));
		}

		m_CategoryDropDown.SetIndex(0);
		m_CategoryDropDown.m_OnSelectItem.Insert(OnSelectCategory);
		FitDropDown(m_CategoryDropHost, m_CategoryLabels.Count());

		m_PlacementDropHost = M_SUB_WIDGET.FindAnyWidget("PlacementDropDownPanel");
		m_PlacementDropDown = new VPPDropDownMenu(m_PlacementDropHost, m_PlacementLabels.Get(PlacementTypes.ON_GROUND));
		for (i = 0; i < m_PlacementLabels.Count(); i++)
		{
			m_PlacementDropDown.AddElement(m_PlacementLabels.Get(i));
		}

		m_PlacementDropDown.SetIndex(PlacementTypes.ON_GROUND);
		m_PlacementDropDown.m_OnSelectItem.Insert(OnSelectPlacement);
		FitDropDown(m_PlacementDropHost, m_PlacementLabels.Count());
		FillParents();
		m_Loaded = true;
	}

	void FitDropDown(Widget host, int itemCount)
	{
		if (!host)
			return;

		host.SetSort(100, true);
		Widget scroller = host.FindAnyWidget("dropdown_container");
		if (!scroller)
			return;

		int rows = itemCount;
		if (rows < 1)
			rows = 1;

		float width;
		float height;
		scroller.GetSize(width, height);
		scroller.SetSize(width, rows * 26);
		scroller.SetSort(101, true);
	}

	override void OnMenuShow()
	{
		super.OnMenuShow();
		if (m_ItemListBox && m_ItemListBox.GetNumItems() == 0)
			FillParents();
	}

	override void OnMenuHide()
	{
		super.OnMenuHide();
		ClearPreview();
		if (m_HandoffTargets)
			m_HandoffTargets.Clear();
	}

	void OpenFromInventory(string className, string steamId, int placement)
	{
		if (className == string.Empty)
			return;

		if (!m_HandoffTargets)
			m_HandoffTargets = new array<string>;

		m_HandoffTargets.Clear();
		if (steamId != string.Empty)
			m_HandoffTargets.Insert(steamId);

		int category = CategoryForClass(className);
		OnSelectCategory(category);
		if (m_SearchInputBox)
			m_SearchInputBox.SetText(className);

		m_SearchBoxStr = className;
		FillParents();
		HighlightParentRow(className);
		SelectParent(className);
		if (m_RootNode)
			m_RootNode.m_Checked = true;

		int i;
		VPPAttTreeRow row;
		for (i = 0; i < m_TreeRows.Count(); i++)
		{
			row = m_TreeRows.Get(i);
			if (row)
				row.SyncCheck();
		}

		OnSelectPlacement(placement);
		if (m_ChkOnSelectedPlayers && m_HandoffTargets.Count() > 0)
			m_ChkOnSelectedPlayers.SetChecked(true);
	}

	int CategoryForClass(string className)
	{
		if (g_Game && g_Game.IsKindOf(className, "transport"))
			return VPPAttCatalog.ATT_CAT_VEHICLES;

		if (g_Game && g_Game.IsKindOf(className, "clothing_base"))
			return VPPAttCatalog.ATT_CAT_CLOTHES;

		return VPPAttCatalog.ATT_CAT_WEAPONS;
	}

	void HighlightParentRow(string className)
	{
		if (!m_ItemListBox)
			return;

		int i;
		string name;
		for (i = 0; i < m_ItemListBox.GetNumItems(); i++)
		{
			m_ItemListBox.GetItemText(i, 0, name);
			if (name != className)
				continue;

			m_ItemListBox.SelectRow(i);
			m_PrevRow = i;
			return;
		}
	}

	override void HideBrokenWidgets(bool state)
	{
		m_HideBroken = state;
		if (m_ItemListBox)
			m_ItemListBox.Show(!state);

		if (m_PartsScroll)
			m_PartsScroll.Show(!state);

		if (m_ItemPreview)
			m_ItemPreview.Show(!state);
	}

	override void OnUpdate(float timeslice)
	{
		super.OnUpdate(timeslice);
		if (!IsSubMenuVisible() && !m_Loaded)
			return;

		if (!m_SearchInputBox)
			return;

		m_FilterTick = m_FilterTick + timeslice;
		if (m_FilterTick >= 0.35)
		{
			m_FilterTick = 0;
			string newSearch = m_SearchInputBox.GetText();
			if (newSearch != m_SearchBoxStr)
			{
				m_SearchBoxStr = newSearch;
				FillParents();
			}
		}

		PollParentSelection();
		FlushParentPreview(timeslice);
		if (m_ItemListBox && IsSubMenuVisible() && !m_HideBroken)
		{
			Widget catScroller = NULL;
			if (m_CategoryDropHost)
				catScroller = m_CategoryDropHost.FindAnyWidget("dropdown_container");

			bool catOpen = false;
			if (catScroller && catScroller.IsVisible())
				catOpen = true;

			if (m_ItemListBox.IsVisible() == catOpen)
				m_ItemListBox.Show(!catOpen);
		}
	}

	override bool OnClick(Widget w, int x, int y, int button)
	{
		super.OnClick(w, x, y, button);
		if (w == m_BtnSpawnItem)
		{
			RequestSpawn();
			return true;
		}

		if (w == m_BtnSelectAll)
		{
			SetAllTicks(true);
			return true;
		}

		if (w == m_BtnClear)
		{
			SetAllTicks(false);
			return true;
		}

		return false;
	}

	void OnSelectCategory(int index)
	{
		if (index < 0)
			return;

		if (index >= m_CategoryLabels.Count())
			return;

		m_CategoryDropDown.SetIndex(index);
		m_CategoryDropDown.SetText(m_CategoryLabels.Get(index));
		m_CategoryDropDown.Close();
		m_CurrentCategory = index;
		m_PrevRow = -2;
		m_SelectedParent = "";
		ClearTree();
		ClearPreview();
		FillParents();
	}

	void OnSelectPlacement(int index)
	{
		if (index < 0)
			return;

		if (index >= m_PlacementLabels.Count())
			return;

		m_PlacementDropDown.SetIndex(index);
		m_PlacementDropDown.SetText(m_PlacementLabels.Get(index));
		m_PlacementDropDown.Close();
	}

	void FillParents()
	{
		if (!m_ItemListBox)
			return;

		m_ItemListBox.ClearItems();
		m_PrevRow = -2;
		string search = m_SearchBoxStr;
		string placeholder = Widget.TranslateString("#VSTR_ATT_SEARCH");
		search.ToLower();
		placeholder.ToLower();
		if (search == "#vstr_plus_att_search" || search == placeholder)
			search = "";

		array<string> classNames = new array<string>;
		array<string> displayNames = new array<string>;
		VPPAttCatalog.Get().CollectParents(m_CurrentCategory, search, classNames, displayNames);
		int i;
		int row;
		for (i = 0; i < classNames.Count(); i++)
		{
			row = m_ItemListBox.AddItem(classNames.Get(i), NULL, 0);
			m_ItemListBox.SetItem(row, displayNames.Get(i), NULL, 1);
		}

		if (m_TxtItemCount)
		{
			if (classNames.Count() == 0)
				m_TxtItemCount.SetText(Widget.TranslateString("#VSTR_ATT_COUNT_EMPTY"));
			else
				m_TxtItemCount.SetText(string.Format(Widget.TranslateString("#VSTR_ATT_COUNT"), classNames.Count()));
		}
	}

	void PollParentSelection()
	{
		if (!m_ItemListBox)
			return;

		if (!(GetMouseState(MouseState.LEFT) & MB_PRESSED_MASK))
			return;

		int row = m_ItemListBox.GetSelectedRow();
		if (row < 0)
			return;

		string className;
		m_ItemListBox.GetItemText(row, 0, className);
		if (className == string.Empty)
			return;

		if (className == m_SelectedParent)
			return;

		m_PrevRow = row;
		SelectParent(className);
	}

	void SelectParent(string className)
	{
		m_SelectedParent = className;
		ClearTree();
		VPPAttCatalog catalog = VPPAttCatalog.Get();
		string cfgPath = catalog.FindConfigPath(className);
		string displayName = catalog.ReadDisplayName(cfgPath, className);
		m_RootNode = new VPPAttTreeNode(className, displayName, false, 0);
		VPPAttCatalog.Log("select-parent " + className);
		RevealSubAttachments(m_RootNode);
		RebuildTree();
		SelectTreeNode(m_RootNode);
		ShowClassPreview(m_RootNode);
	}

	void OnAttTreeRowClick(VPPAttTreeRow row)
	{
		if (!row)
			return;

		VPPAttTreeNode node = row.GetNode();
		if (!node)
			return;

		if (node.m_IsHeader)
		{
			node.m_Expanded = !node.m_Expanded;
			RebuildTree();
			SelectTreeNode(node);
			return;
		}

		RevealAndRebuild(node);
		m_PreviewBuildMode = false;
		SelectTreeNode(node);
		ShowClassPreview(node);
	}

	void OnAttTreeRowView(VPPAttTreeRow row)
	{
		if (!row)
			return;

		VPPAttTreeNode node = row.GetNode();
		if (!node)
			return;

		if (node.m_IsHeader)
			return;

		RevealAndRebuild(node);

		m_TrialClass = "";
		if (!node.m_Checked && m_RootNode && node != m_RootNode)
			m_TrialClass = node.m_ClassName;

		m_PreviewBuildMode = true;
		SelectTreeNode(node);
		ShowBuildPreview();
	}

	void OnAttTreeTick(VPPAttTreeRow row)
	{
		if (!row)
			return;

		VPPAttTreeNode node = row.GetNode();
		if (!node)
			return;

		SelectTreeNode(node);
		if (m_PreviewBuildMode)
		{
			if (node.m_Checked && m_TrialClass == node.m_ClassName)
				m_TrialClass = "";

			ShowBuildPreview();
		}
		else
		{
			ShowClassPreview(node);
		}
	}

	void FlushParentPreview(float timeslice)
	{
		if (!m_PreviewDirty)
			return;

		m_PreviewDirtyTick = m_PreviewDirtyTick + timeslice;
		if (m_PreviewDirtyTick < 0.2)
			return;

		m_PreviewDirtyTick = 0;
		m_PreviewDirty = false;
		ShowBuildPreview();
	}

	void SelectTreeNode(VPPAttTreeNode target)
	{
		int i;
		VPPAttTreeRow row;
		m_SelectedRow = NULL;
		for (i = 0; i < m_TreeRows.Count(); i++)
		{
			row = m_TreeRows.Get(i);
			if (!row)
				continue;

			if (row.GetNode() == target)
			{
				m_SelectedRow = row;
				row.SetSelected(true);
				row.SetViewActive(m_PreviewBuildMode);
			}
			else
			{
				row.SetSelected(false);
				row.SetViewActive(false);
			}
		}
	}

	void RevealAndRebuild(VPPAttTreeNode node)
	{
		if (!node)
			return;

		bool needRebuild = !node.m_Expanded;
		RevealSubAttachments(node);
		if (needRebuild)
			RebuildTree();
	}

	void RevealSubAttachments(VPPAttTreeNode node)
	{
		if (!node)
			return;

		if (node.m_IsHeader)
			return;

		ExpandNode(node);
		ExpandChildHeaders(node);
	}

	void ExpandChildHeaders(VPPAttTreeNode node)
	{
		if (!node)
			return;

		int i;
		VPPAttTreeNode child;
		for (i = 0; i < node.m_Children.Count(); i++)
		{
			child = node.m_Children.Get(i);
			if (!child)
				continue;

			if (child.m_IsHeader)
				child.m_Expanded = true;
		}
	}

	void ExpandNode(VPPAttTreeNode node)
	{
		if (!node)
			return;

		if (node.m_IsHeader)
			return;

		if (node.m_Expanded)
			return;

		node.m_Expanded = true;
		array<ref VPPAttGroup> groups = VPPAttCatalog.Get().Resolve(node.m_ClassName);
		if (!groups)
			return;

		int i;
		int t;
		int usable;
		VPPAttGroup group;
		VPPAttTreeNode header;
		VPPAttTreeNode child;
		string typeName;
		string cfgPath;
		string displayName;
		VPPAttCatalog catalog = VPPAttCatalog.Get();
		int childDepth = node.m_Depth + 1;
		usable = 0;
		for (i = 0; i < groups.Count(); i++)
		{
			group = groups.Get(i);
			if (group && group.m_Types.Count() > 0)
				usable = usable + 1;
		}

		for (i = 0; i < groups.Count(); i++)
		{
			group = groups.Get(i);
			if (!group || group.m_Types.Count() == 0)
				continue;

			if (usable == 1)
			{
				for (t = 0; t < group.m_Types.Count(); t++)
				{
					typeName = group.m_Types.Get(t);
					cfgPath = catalog.FindConfigPath(typeName);
					displayName = catalog.ReadDisplayName(cfgPath, typeName);
					child = new VPPAttTreeNode(typeName, displayName, false, childDepth);
					node.m_Children.Insert(child);
				}

				continue;
			}

			header = new VPPAttTreeNode("", group.m_SlotName, true, childDepth);
			node.m_Children.Insert(header);
			for (t = 0; t < group.m_Types.Count(); t++)
			{
				typeName = group.m_Types.Get(t);
				cfgPath = catalog.FindConfigPath(typeName);
				displayName = catalog.ReadDisplayName(cfgPath, typeName);
				child = new VPPAttTreeNode(typeName, displayName, false, childDepth + 1);
				header.m_Children.Insert(child);
			}
		}
	}

	void RebuildTree()
	{
		ClearTreeRows();
		if (!m_PartsGrid)
			return;

		m_DataGrids = new array<ref CustomGridSpacer>;
		m_DataGrids.Insert(new CustomGridSpacer(m_PartsGrid));
		m_LastGrid = m_DataGrids.Get(0);
		if (m_RootNode)
			AddNodeRows(m_RootNode);

		if (m_LastGrid && m_LastGrid.GetGrid())
			m_LastGrid.GetGrid().Update();

		if (m_PartsGrid)
			m_PartsGrid.Update();

		if (m_PartsScroll)
			m_PartsScroll.Update();
	}

	void AddNodeRows(VPPAttTreeNode node)
	{
		if (!node)
			return;

		AddTreeRow(node);
		if (node.m_IsHeader && !node.m_Expanded)
			return;

		int i;
		for (i = 0; i < node.m_Children.Count(); i++)
		{
			AddNodeRows(node.m_Children.Get(i));
		}
	}

	void AddTreeRow(VPPAttTreeNode node)
	{
		if (!m_LastGrid)
			return;

		if (m_LastGrid.GetContentCount() == 100)
		{
			m_LastGrid = new CustomGridSpacer(m_PartsGrid);
			m_DataGrids.Insert(m_LastGrid);
		}

		VPPAttTreeRow row = new VPPAttTreeRow(m_LastGrid.GetGrid(), this, node);
		m_LastGrid.AddWidget(row.GetRoot());
		m_TreeRows.Insert(row);
	}

	void ClearTree()
	{
		m_RootNode = NULL;
		m_SelectedRow = NULL;
		ClearTreeRows();
	}

	void ClearTreeRows()
	{
		int i;
		VPPAttTreeRow row;
		for (i = 0; i < m_TreeRows.Count(); i++)
		{
			row = m_TreeRows.Get(i);
			if (row)
				delete row;
		}

		m_TreeRows = new array<ref VPPAttTreeRow>;
		m_SelectedRow = NULL;
		m_DataGrids = new array<ref CustomGridSpacer>;
		m_LastGrid = NULL;
		if (m_PartsGrid)
		{
			Widget child;
			Widget next;
			child = m_PartsGrid.GetChildren();
			while (child)
			{
				next = child.GetSibling();
				child.Unlink();
				child = next;
			}

			m_PartsGrid.Update();
		}
	}

	void SetAllTicks(bool state)
	{
		if (!m_RootNode)
			return;

		ApplyTick(m_RootNode, state);
		int i;
		VPPAttTreeRow row;
		for (i = 0; i < m_TreeRows.Count(); i++)
		{
			row = m_TreeRows.Get(i);
			if (row)
				row.SyncCheck();
		}

		if (m_PreviewBuildMode)
			ShowBuildPreview();
	}

	void ApplyTick(VPPAttTreeNode node, bool state)
	{
		if (!node)
			return;

		if (!node.m_IsHeader)
			node.m_Checked = state;

		int i;
		for (i = 0; i < node.m_Children.Count(); i++)
		{
			ApplyTick(node.m_Children.Get(i), state);
		}
	}

	void CollectTicked(VPPAttTreeNode node, array<string> types)
	{
		if (!node)
			return;

		if (!node.m_IsHeader && node.m_Checked && node.m_ClassName != string.Empty)
			types.Insert(node.m_ClassName);

		int i;
		for (i = 0; i < node.m_Children.Count(); i++)
		{
			CollectTicked(node.m_Children.Get(i), types);
		}
	}

	void ShowClassPreview(VPPAttTreeNode node)
	{
		if (!node)
			return;

		m_PreviewBuildMode = false;
		m_TrialClass = "";
		ShowPreview(node.m_ClassName, node.m_ClassName);
		if (m_SelectedRow)
			m_SelectedRow.SetViewActive(false);
	}

	void ShowBuildPreview()
	{
		if (!m_RootNode)
			return;

		m_PreviewBuildMode = true;
		m_PreviewIsParent = true;
		ShowPreview(m_RootNode.m_ClassName, m_RootNode.m_Label);
		m_PreviewIsParent = true;
		AttachTickedToPreview();
		if (m_TrialClass != string.Empty)
			AttachTrialToPreview(m_TrialClass);

		RefreshPreviewWidget();
		if (m_SelectedRow)
			m_SelectedRow.SetViewActive(true);
	}

	void ShowParentPreview()
	{
		m_TrialClass = "";
		ShowBuildPreview();
	}

	void ShowPreview(string className, string displayName)
	{
		ClearPreview();
		m_PreviewIsParent = false;
		if (m_TxtPreviewName)
			m_TxtPreviewName.SetText(displayName);

		if (className == string.Empty)
			return;

		if (!VPPAttCatalog.Get().CanSpawnLocalPreview(className))
		{
			VPPAttCatalog.Log("skip-preview " + className);
			return;
		}

		m_PreviewObject = EntityAI.Cast(g_Game.CreateObjectEx(className, vector.Zero, ECE_LOCAL));
		if (!m_PreviewObject)
			m_PreviewObject = EntityAI.Cast(g_Game.CreateObject(className, vector.Zero, true, false, false));

		if (m_PreviewObject)
			m_PreviewObject.DisableSimulation(true);

		RefreshPreviewWidget();
	}

	void RefreshPreviewWidget()
	{
		if (!m_ItemPreview)
			return;

		m_ItemPreview.SetItem(NULL);
		if (!m_PreviewObject)
			return;

		m_ItemPreview.SetItem(m_PreviewObject);
		m_ItemPreview.SetModelPosition(Vector(0, 0, 0.5));
		m_ItemPreview.SetModelOrientation(Vector(0, 0, 0));
		if (m_ItemPreview.GetItem())
			m_ItemPreview.SetView(m_ItemPreview.GetItem().GetViewIndex());

		m_ItemPreview.Show(true);
	}

	void AttachTickedToPreview()
	{
		if (!m_PreviewObject || !m_RootNode)
			return;

		array<string> types = new array<string>;
		CollectTicked(m_RootNode, types);
		array<EntityAI> hosts = new array<EntityAI>;
		hosts.Insert(m_PreviewObject);
		int i;
		string typeName;
		EntityAI created;
		for (i = 0; i < types.Count(); i++)
		{
			typeName = types.Get(i);
			if (typeName == string.Empty || typeName == m_RootNode.m_ClassName)
				continue;

			created = AttachPreviewPart(hosts, typeName);
			if (created)
				hosts.Insert(created);
		}
	}

	void AttachTrialToPreview(string typeName)
	{
		if (!m_PreviewObject || typeName == string.Empty)
			return;

		array<string> ticked = new array<string>;
		CollectTicked(m_RootNode, ticked);
		if (ticked.Find(typeName) > -1)
			return;

		array<EntityAI> hosts = new array<EntityAI>;
		CollectPreviewHosts(m_PreviewObject, hosts);
		AttachPreviewPart(hosts, typeName);
	}

	void CollectPreviewHosts(EntityAI entity, array<EntityAI> hosts)
	{
		if (!entity)
			return;

		hosts.Insert(entity);
		if (!entity.GetInventory())
			return;

		int i;
		EntityAI child;
		for (i = 0; i < entity.GetInventory().AttachmentCount(); i++)
		{
			child = entity.GetInventory().GetAttachmentFromIndex(i);
			if (child)
				CollectPreviewHosts(child, hosts);
		}
	}

	EntityAI AttachPreviewPart(array<EntityAI> hosts, string typeName)
	{
		int i;
		EntityAI host;
		EntityAI created;
		for (i = hosts.Count() - 1; i >= 0; i--)
		{
			host = hosts.Get(i);
			created = VPPAttAttach.AttachTo(host, typeName, true);
			if (created)
				return created;
		}

		return NULL;
	}

	void ClearPreview()
	{
		if (m_ItemPreview)
			m_ItemPreview.SetItem(NULL);

		if (m_PreviewObject)
		{
			g_Game.ObjectDelete(m_PreviewObject);
			m_PreviewObject = NULL;
		}
	}

	void RequestSpawn()
	{
		array<string> types = new array<string>;
		CollectTicked(m_RootNode, types);
		VPPAttCatalog.Log("request-spawn n=" + types.Count().ToString() + " cat=" + m_CurrentCategory.ToString());
		int t;
		for (t = 0; t < types.Count(); t++)
		{
			VPPAttCatalog.Log("  tick " + types.Get(t));
		}

		if (types.Count() == 0)
		{
			GetVPPUIManager().DisplayError("#VSTR_NOTIFY_ATT_NONE");
			return;
		}

		VPPAttSpawnParams params = new VPPAttSpawnParams();
		params.m_Category = m_CurrentCategory;
		params.m_Types = types;
		params.m_PlacementType = m_PlacementDropDown.GetIndex();
		params.m_Position = vector.Zero;
		if (params.m_PlacementType == PlacementTypes.ON_GROUND)
			params.m_Position = g_Game.GetPlayer().GetPosition();
		else if (params.m_PlacementType == PlacementTypes.AT_CROSSHAIR)
			params.m_Position = g_Game.GetCursorPos();

		if (m_ChkOnSelectedPlayers && m_ChkOnSelectedPlayers.IsChecked() && m_HandoffTargets && m_HandoffTargets.Count() > 0)
		{
			params.m_Targets = m_HandoffTargets;
		}
		else if (m_ChkOnSelectedPlayers && m_ChkOnSelectedPlayers.IsChecked())
		{
			MenuPlayerManager pManager = MenuPlayerManager.Cast(VPPAdminHud.Cast(GetVPPUIManager().GetMenuByType(VPPAdminHud)).GetSubMenuByType(MenuPlayerManager));
			if (!pManager)
			{
				GetVPPUIManager().DisplayError("#VSTR_NOTIFY_ERR_SPAWN_PRESET");
				return;
			}

			params.m_Targets = pManager.GetSelectedPlayersIDs();
			if (!params.m_Targets || params.m_Targets.Count() == 0)
			{
				GetVPPUIManager().DisplayError("#VSTR_NOTIFY_ERR_SPAWN_PRESET_NOPLAYER");
				return;
			}
		}

		GetRPCManager().VSendRPC("RPC_VPPAttachmentsSpawner", "SpawnAttachments", new Param1<ref VPPAttSpawnParams>(params), true, NULL);
	}
};
