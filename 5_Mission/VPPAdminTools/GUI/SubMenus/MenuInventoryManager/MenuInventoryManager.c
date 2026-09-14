class MenuInventoryManager extends AdminHudSubMenu
{
	protected bool m_Loaded;
	protected TextListboxWidget m_PlayerList;
	protected TextWidget m_TxtPlayerCount;
	protected PlayerPreviewWidget m_PlayerPreview;
	protected ButtonWidget m_BtnRefresh;
	protected ButtonWidget m_BtnClearInventory;
	protected ButtonWidget m_BtnInventoryView;
	protected ButtonWidget m_BtnAddItem;
	protected TextWidget m_LblInventoryView;
	protected TextWidget m_LblAddItem;
	protected ButtonWidget m_BtnRepairAll;
	protected ButtonWidget m_BtnTeleport;
	protected EditBoxWidget m_SearchInputBox;
	protected TextWidget m_TxtItemCount;
	protected ScrollWidget m_InvScroll;
	protected GridSpacerWidget m_ParentGrid;
	protected Widget m_PanelSpawn;
	protected TextListboxWidget m_ObjectsList;
	protected ButtonWidget m_BtnCatAll;
	protected ButtonWidget m_BtnCatItems;
	protected ButtonWidget m_BtnCatEdible;
	protected ButtonWidget m_BtnCatWeapon;
	protected ButtonWidget m_BtnCatClothing;
	protected ItemPreviewWidget m_SpawnPreview;
	protected TextWidget m_TxtSelectedItem;
	protected ButtonWidget m_BtnConditionLower;
	protected ButtonWidget m_BtnConditionRaise;
	protected TextWidget m_TxtCondition;
	protected TextWidget m_TxtSpawnQty;
	protected SliderWidget m_SpawnQtySlider;
	protected CheckBoxWidget m_ChkSpawnAttachments;
	protected ButtonWidget m_BtnBuildAttachments;
	protected ButtonWidget m_BtnSpawnOnPlayer;

	protected ref array<ref VPPInvPlayerInfo> m_Players;
	protected ref array<ref VPPInvItemInfo> m_AllItems;
	protected ref array<ref VPPInvItemRow> m_Rows;
	protected ref array<Widget> m_CardGrids;
	protected GridSpacerWidget m_LastCardGrid;
	protected int m_LastCardCount;
	protected ref array<string> m_Catalog;
	protected string m_SelectedSteamId;
	protected string m_SearchBoxStr;
	protected float m_FilterTick;
	protected bool m_SpawnMode;
	protected bool m_HideBroken;
	protected string m_SpawnCategory;
	protected string m_SpawnClass;
	protected int m_SpawnHealth;
	protected EntityAI m_SpawnPreviewObject;
	protected ref VPPInvItemInfo m_PendingDelete;

	void MenuInventoryManager()
	{
		m_Players = new array<ref VPPInvPlayerInfo>;
		m_AllItems = new array<ref VPPInvItemInfo>;
		m_Rows = new array<ref VPPInvItemRow>;
		m_CardGrids = new array<Widget>;
		m_LastCardCount = 0;
		m_Catalog = new array<string>;
		m_SelectedSteamId = "";
		m_SearchBoxStr = "";
		m_FilterTick = 0;
		m_SpawnMode = false;
		m_HideBroken = false;
		m_SpawnCategory = "All";
		m_SpawnClass = "";
		m_SpawnHealth = 0;
		GetRPCManager().AddRPC("RPC_MenuInventoryManager", "ReceivePlayers", this, SingleplayerExecutionType.Client);
		GetRPCManager().AddRPC("RPC_MenuInventoryManager", "ReceiveInventory", this, SingleplayerExecutionType.Client);
	}

	void ~MenuInventoryManager()
	{
		ClearRows();
		ClearSpawnPreview();
	}

	override void OnCreate(Widget RootW)
	{
		super.OnCreate(RootW);
		M_SUB_WIDGET = CreateWidgets(VPPATUIConstants.MenuInventoryManager);
		M_SUB_WIDGET.SetHandler(this);
		m_TitlePanel = Widget.Cast(M_SUB_WIDGET.FindAnyWidget("Header"));
		m_closeButton = ButtonWidget.Cast(M_SUB_WIDGET.FindAnyWidget("BtnClose"));

		m_PlayerList = TextListboxWidget.Cast(M_SUB_WIDGET.FindAnyWidget("PlayerList"));
		m_TxtPlayerCount = TextWidget.Cast(M_SUB_WIDGET.FindAnyWidget("TxtPlayerCount"));
		m_PlayerPreview = PlayerPreviewWidget.Cast(M_SUB_WIDGET.FindAnyWidget("PlayerPreview"));
		m_BtnRefresh = ButtonWidget.Cast(M_SUB_WIDGET.FindAnyWidget("BtnRefresh"));
		m_BtnClearInventory = ButtonWidget.Cast(M_SUB_WIDGET.FindAnyWidget("BtnClearInventory"));
		m_BtnInventoryView = ButtonWidget.Cast(M_SUB_WIDGET.FindAnyWidget("BtnInventoryView"));
		m_BtnAddItem = ButtonWidget.Cast(M_SUB_WIDGET.FindAnyWidget("BtnAddItem"));
		m_LblInventoryView = TextWidget.Cast(M_SUB_WIDGET.FindAnyWidget("LblInventoryView"));
		m_LblAddItem = TextWidget.Cast(M_SUB_WIDGET.FindAnyWidget("LblAddItem"));
		m_BtnRepairAll = ButtonWidget.Cast(M_SUB_WIDGET.FindAnyWidget("BtnRepairAll"));
		m_BtnTeleport = ButtonWidget.Cast(M_SUB_WIDGET.FindAnyWidget("BtnTeleport"));
		m_SearchInputBox = EditBoxWidget.Cast(M_SUB_WIDGET.FindAnyWidget("SearchInputBox"));
		m_TxtItemCount = TextWidget.Cast(M_SUB_WIDGET.FindAnyWidget("TxtItemCount"));
		m_InvScroll = ScrollWidget.Cast(M_SUB_WIDGET.FindAnyWidget("InvScroll"));
		m_ParentGrid = GridSpacerWidget.Cast(M_SUB_WIDGET.FindAnyWidget("ParentGrid"));
		m_PanelSpawn = M_SUB_WIDGET.FindAnyWidget("PanelSpawn");
		m_ObjectsList = TextListboxWidget.Cast(M_SUB_WIDGET.FindAnyWidget("ObjectsList"));
		m_BtnCatAll = ButtonWidget.Cast(M_SUB_WIDGET.FindAnyWidget("CategoryAll"));
		m_BtnCatItems = ButtonWidget.Cast(M_SUB_WIDGET.FindAnyWidget("CategoryItems"));
		m_BtnCatEdible = ButtonWidget.Cast(M_SUB_WIDGET.FindAnyWidget("CategoryEdible"));
		m_BtnCatWeapon = ButtonWidget.Cast(M_SUB_WIDGET.FindAnyWidget("CategoryWeapon"));
		m_BtnCatClothing = ButtonWidget.Cast(M_SUB_WIDGET.FindAnyWidget("CategoryClothing"));
		m_SpawnPreview = ItemPreviewWidget.Cast(M_SUB_WIDGET.FindAnyWidget("SpawnPreview"));
		m_TxtSelectedItem = TextWidget.Cast(M_SUB_WIDGET.FindAnyWidget("TxtSelectedItem"));
		m_BtnConditionLower = ButtonWidget.Cast(M_SUB_WIDGET.FindAnyWidget("BtnConditionLower"));
		m_BtnConditionRaise = ButtonWidget.Cast(M_SUB_WIDGET.FindAnyWidget("BtnConditionRaise"));
		m_TxtCondition = TextWidget.Cast(M_SUB_WIDGET.FindAnyWidget("TxtCondition"));
		m_TxtSpawnQty = TextWidget.Cast(M_SUB_WIDGET.FindAnyWidget("TxtSpawnQty"));
		m_SpawnQtySlider = SliderWidget.Cast(M_SUB_WIDGET.FindAnyWidget("SpawnQtySlider"));
		m_ChkSpawnAttachments = CheckBoxWidget.Cast(M_SUB_WIDGET.FindAnyWidget("ChkSpawnAttachments"));
		m_BtnBuildAttachments = ButtonWidget.Cast(M_SUB_WIDGET.FindAnyWidget("BtnBuildAttachments"));
		m_BtnSpawnOnPlayer = ButtonWidget.Cast(M_SUB_WIDGET.FindAnyWidget("BtnSpawnOnPlayer"));

		if (m_BtnClearInventory)
			GetVPPUIManager().HookConfirmationDialog(m_BtnClearInventory, M_SUB_WIDGET, this, "ConfirmClearInventory", DIAGTYPE.DIAG_YESNO, "#VSTR_INV_CLEAR_TITLE", "#VSTR_INV_CLEAR_BODY");

		if (m_BtnRepairAll)
			GetVPPUIManager().HookConfirmationDialog(m_BtnRepairAll, M_SUB_WIDGET, this, "ConfirmRepairAll", DIAGTYPE.DIAG_YESNO, "#VSTR_INV_REPAIR_TITLE", "#VSTR_INV_REPAIR_BODY");

		SetSpawnMode(false);
		ApplySpawnCondition();
		RequestPlayers();
		m_Loaded = true;
	}

	override void OnMenuShow()
	{
		super.OnMenuShow();
		RequestPlayers();
		if (m_SelectedSteamId != string.Empty)
			RequestInventory();
	}

	override void HideBrokenWidgets(bool state)
	{
		m_HideBroken = state;

		if (m_InvScroll)
		{
			if (!state && !m_SpawnMode)
				m_InvScroll.Show(true);
			else
				m_InvScroll.Show(false);
		}

		if (m_PlayerList)
			m_PlayerList.Show(!state);

		if (m_ObjectsList)
		{
			if (!state && m_SpawnMode)
				m_ObjectsList.Show(true);
			else
				m_ObjectsList.Show(false);
		}

		if (m_PlayerPreview)
		{
			if (state)
				m_PlayerPreview.Show(false);
			else
				UpdatePlayerPreview();
		}

		if (m_SpawnPreview)
		{
			if (state)
			{
				m_SpawnPreview.SetItem(NULL);
				m_SpawnPreview.Show(false);
			}
			else
			{
				if (m_SpawnMode && m_SpawnPreviewObject)
				{
					m_SpawnPreview.Show(true);
					m_SpawnPreview.SetItem(m_SpawnPreviewObject);
					m_SpawnPreview.SetModelPosition(Vector(0, 0, 0.5));
					m_SpawnPreview.SetModelOrientation(Vector(0, 0, 0));
					if (m_SpawnPreview.GetItem())
						m_SpawnPreview.SetView(m_SpawnPreview.GetItem().GetViewIndex());
				}
				else
				{
					m_SpawnPreview.Show(false);
				}
			}
		}
	}

	override void OnUpdate(float timeslice)
	{
		super.OnUpdate(timeslice);
		if (!IsSubMenuVisible() && !m_Loaded)
			return;

		if (!m_SearchInputBox)
			return;

		m_FilterTick = m_FilterTick + timeslice;
		if (m_FilterTick < 0.4)
			return;

		m_FilterTick = 0;
		string newSearch = ReadSearchText();
		if (newSearch != m_SearchBoxStr)
		{
			m_SearchBoxStr = newSearch;
			if (m_SpawnMode)
				RebuildSpawnList();
			else
				RebuildItemRows();
		}
	}

	override bool OnClick(Widget w, int x, int y, int button)
	{
		super.OnClick(w, x, y, button);
		if (w == m_closeButton)
			return true;

		if (w == m_BtnRefresh)
		{
			RequestPlayers();
			RequestInventory();
			return true;
		}

		if (w == m_BtnInventoryView)
		{
			SetSpawnMode(false);
			return true;
		}

		if (w == m_BtnAddItem)
		{
			SetSpawnMode(true);
			return true;
		}

		if (w == m_BtnTeleport)
		{
			RequestTeleport();
			return true;
		}

		if (w == m_PlayerList)
		{
			SelectPlayerFromList();
			return true;
		}

		if (w == m_ObjectsList)
		{
			SelectSpawnFromList();
			return true;
		}

		if (w == m_BtnCatAll)
		{
			m_SpawnCategory = "All";
			RebuildSpawnList();
			return true;
		}

		if (w == m_BtnCatItems)
		{
			m_SpawnCategory = "inventory_base";
			RebuildSpawnList();
			return true;
		}

		if (w == m_BtnCatEdible)
		{
			m_SpawnCategory = "edible_base";
			RebuildSpawnList();
			return true;
		}

		if (w == m_BtnCatWeapon)
		{
			m_SpawnCategory = "weapon_base";
			RebuildSpawnList();
			return true;
		}

		if (w == m_BtnCatClothing)
		{
			m_SpawnCategory = "clothing_base";
			RebuildSpawnList();
			return true;
		}

		if (w == m_BtnConditionLower)
		{
			if (m_SpawnHealth < 4)
				m_SpawnHealth = m_SpawnHealth + 1;

			ApplySpawnCondition();
			return true;
		}

		if (w == m_BtnConditionRaise)
		{
			if (m_SpawnHealth > 0)
				m_SpawnHealth = m_SpawnHealth - 1;

			ApplySpawnCondition();
			return true;
		}

		if (w == m_BtnBuildAttachments)
		{
			OpenAttachmentsBuilder();
			return true;
		}

		if (w == m_BtnSpawnOnPlayer)
		{
			RequestSpawn();
			return true;
		}

		return true;
	}

	override bool OnChange(Widget w, int x, int y, bool finished)
	{
		super.OnChange(w, x, y, finished);
		if (w == m_PlayerList)
			SelectPlayerFromList();

		if (w == m_ObjectsList)
			SelectSpawnFromList();

		return false;
	}

	string ReadSearchText()
	{
		if (!m_SearchInputBox)
			return "";

		string text = m_SearchInputBox.GetText();
		if (text == Widget.TranslateString("#VSTR_INV_SEARCH"))
			return "";

		return text;
	}

	void RequestPlayers()
	{
		GetRPCManager().VSendRPC("RPC_VPPInventoryManager", "RequestPlayers", new Param1<int>(1), true);
	}

	void RequestInventory()
	{
		if (m_SelectedSteamId == string.Empty)
			return;

		GetRPCManager().VSendRPC("RPC_VPPInventoryManager", "RequestInventory", new Param1<string>(m_SelectedSteamId), true);
	}

	void ReceivePlayers(CallType type, ParamsReadContext ctx, PlayerIdentity sender, Object target)
	{
		if (type != CallType.Client)
			return;

		Param1<ref array<ref VPPInvPlayerInfo>> data;
		if (!ctx.Read(data))
			return;

		if (!data.param1)
			return;

		m_Players = new array<ref VPPInvPlayerInfo>;
		int i;
		for (i = 0; i < data.param1.Count(); i++)
		{
			if (data.param1.Get(i))
				m_Players.Insert(data.param1.Get(i));
		}

		RebuildPlayerList();
	}

	void ReceiveInventory(CallType type, ParamsReadContext ctx, PlayerIdentity sender, Object target)
	{
		if (type != CallType.Client)
			return;

		Param3<string, vector, ref array<ref VPPInvItemInfo>> data;
		if (!ctx.Read(data))
			return;

		if (data.param1 != string.Empty)
			m_SelectedSteamId = data.param1;

		m_AllItems = new array<ref VPPInvItemInfo>;
		if (data.param3)
		{
			int i;
			for (i = 0; i < data.param3.Count(); i++)
			{
				if (data.param3.Get(i))
					m_AllItems.Insert(data.param3.Get(i));
			}
		}

		UpdatePlayerPreview();
		if (!m_SpawnMode)
			RebuildItemRows();
	}

	void RebuildPlayerList()
	{
		if (!m_PlayerList)
			return;

		int selected = -1;
		m_PlayerList.ClearItems();
		int i;
		VPPInvPlayerInfo info;
		for (i = 0; i < m_Players.Count(); i++)
		{
			info = m_Players.Get(i);
			if (!info)
				continue;

			m_PlayerList.AddItem(info.m_Name, new Param1<string>(info.m_SteamId), 0);
			if (info.m_SteamId == m_SelectedSteamId)
				selected = i;
		}

		if (m_TxtPlayerCount)
		{
			if (m_Players.Count() == 0)
				m_TxtPlayerCount.SetText(Widget.TranslateString("#VSTR_INV_COUNT_EMPTY"));
			else
				m_TxtPlayerCount.SetText(m_Players.Count().ToString() + " " + Widget.TranslateString("#VSTR_INV_PLAYERS"));
		}

		if (selected > -1)
			m_PlayerList.SelectRow(selected);
	}

	void SelectPlayerFromList()
	{
		if (!m_PlayerList)
			return;

		int row = m_PlayerList.GetSelectedRow();
		if (row < 0)
			return;

		Param1<string> data;
		m_PlayerList.GetItemData(row, 0, data);
		if (!data)
			return;

		if (data.param1 == m_SelectedSteamId)
			return;

		m_SelectedSteamId = data.param1;
		RequestInventory();
	}

	void UpdatePlayerPreview()
	{
		if (!m_PlayerPreview)
			return;

		if (m_HideBroken)
		{
			m_PlayerPreview.Show(false);
			return;
		}

		PlayerBase player = FindLocalPlayer(m_SelectedSteamId);
		if (!player)
		{
			m_PlayerPreview.Show(false);
			return;
		}

		m_PlayerPreview.Show(true);
		m_PlayerPreview.SetPlayer(player);
		if (player.GetHumanInventory())
			m_PlayerPreview.UpdateItemInHands(player.GetHumanInventory().GetEntityInHands());
	}

	PlayerBase FindLocalPlayer(string steamId)
	{
		if (steamId == string.Empty || !g_Game)
			return NULL;

		array<Man> players = new array<Man>;
		g_Game.GetPlayers(players);
		int i;
		PlayerBase player;
		for (i = 0; i < players.Count(); i++)
		{
			player = PlayerBase.Cast(players.Get(i));
			if (!player || !player.GetIdentity())
				continue;

			if (player.GetIdentity().GetPlainId() == steamId)
				return player;
		}

		return NULL;
	}

	void RebuildItemRows()
	{
		ClearRows();
		if (!m_ParentGrid)
			return;

		m_CardGrids = new array<Widget>;
		m_LastCardGrid = NULL;
		m_LastCardCount = 0;

		string search = m_SearchBoxStr;
		search.ToLower();
		int shown;
		int i;
		VPPInvItemInfo info;
		string className;
		string displayName;
		for (i = 0; i < m_AllItems.Count(); i++)
		{
			info = m_AllItems.Get(i);
			if (!info)
				continue;

			className = info.m_ClassName;
			className.ToLower();
			displayName = info.m_DisplayName;
			displayName.ToLower();
			if (search != string.Empty && !className.Contains(search) && !displayName.Contains(search))
				continue;

			AddItemRow(info);
			shown = shown + 1;
		}

		if (m_LastCardGrid)
			m_LastCardGrid.Update();

		if (m_ParentGrid)
			m_ParentGrid.Update();

		if (m_InvScroll)
			m_InvScroll.Update();

		if (m_TxtItemCount)
		{
			if (shown == 0)
				m_TxtItemCount.SetText(Widget.TranslateString("#VSTR_INV_ITEMS_EMPTY"));
			else
				m_TxtItemCount.SetText(shown.ToString() + " " + Widget.TranslateString("#VSTR_INV_ITEMS"));
		}
	}

	void AddItemRow(VPPInvItemInfo info)
	{
		if (!m_ParentGrid)
			return;

		if (!m_LastCardGrid || m_LastCardCount >= 99)
		{
			m_LastCardGrid = GridSpacerWidget.Cast(g_Game.GetWorkspace().CreateWidgets(VPPATUIConstants.InvCardGrid, m_ParentGrid));
			if (m_LastCardGrid)
				m_CardGrids.Insert(m_LastCardGrid);

			m_LastCardCount = 0;
		}

		if (!m_LastCardGrid)
			return;

		VPPInvItemRow row = new VPPInvItemRow(m_LastCardGrid, this, info);
		m_LastCardCount = m_LastCardCount + 1;
		m_Rows.Insert(row);
		m_LastCardGrid.Update();
	}

	void ClearRows()
	{
		int i;
		VPPInvItemRow row;
		for (i = 0; i < m_Rows.Count(); i++)
		{
			row = m_Rows.Get(i);
			if (row)
				delete row;
		}

		m_Rows = new array<ref VPPInvItemRow>;
		m_CardGrids = new array<Widget>;
		m_LastCardGrid = NULL;
		m_LastCardCount = 0;
		if (!m_ParentGrid)
			return;

		Widget child;
		Widget next;
		child = m_ParentGrid.GetChildren();
		while (child)
		{
			next = child.GetSibling();
			child.Unlink();
			child = next;
		}
	}

	void SetSpawnMode(bool state)
	{
		m_SpawnMode = state;
		if (m_InvScroll)
			m_InvScroll.Show(!state);

		if (m_PanelSpawn)
			m_PanelSpawn.Show(state);

		if (m_ObjectsList)
			m_ObjectsList.Show(state);

		if (state)
		{
			EnsureCatalog();
			RebuildSpawnList();
		}
		else
		{
			RebuildItemRows();
			RequestInventory();
		}

		ApplyViewTabs();
	}

	void ApplyViewTabs()
	{
		int onColor = ARGB(255, 232, 163, 61);
		int offColor = ARGB(255, 225, 228, 231);
		if (m_LblInventoryView)
		{
			if (m_SpawnMode)
				m_LblInventoryView.SetColor(offColor);
			else
				m_LblInventoryView.SetColor(onColor);
		}

		if (m_LblAddItem)
		{
			if (m_SpawnMode)
				m_LblAddItem.SetColor(onColor);
			else
				m_LblAddItem.SetColor(offColor);
		}
	}

	void EnsureCatalog()
	{
		if (m_Catalog.Count() > 0)
			return;

		TStringArray paths = new TStringArray;
		paths.Insert("CfgVehicles");
		paths.Insert("CfgWeapons");
		paths.Insert("CfgMagazines");
		int p;
		int n;
		int count;
		int scope;
		string path;
		string name;
		for (p = 0; p < paths.Count(); p++)
		{
			path = paths.Get(p);
			count = g_Game.ConfigGetChildrenCount(path);
			for (n = 0; n < count; n++)
			{
				g_Game.ConfigGetChildName(path, n, name);
				scope = g_Game.ConfigGetInt(path + " " + name + " scope");
				if (scope != 2)
					continue;

				m_Catalog.Insert(name);
			}
		}
	}

	void RebuildSpawnList()
	{
		if (!m_ObjectsList)
			return;

		m_ObjectsList.ClearItems();
		string search = m_SearchBoxStr;
		search.ToLower();
		int i;
		string name;
		string lower;
		for (i = 0; i < m_Catalog.Count(); i++)
		{
			name = m_Catalog.Get(i);
			lower = name;
			lower.ToLower();
			if (m_SpawnCategory != "All" && !g_Game.IsKindOf(name, m_SpawnCategory))
				continue;

			if (search != string.Empty && !lower.Contains(search))
				continue;

			m_ObjectsList.AddItem(name, NULL, 0);
		}
	}

	void SelectSpawnFromList()
	{
		if (!m_ObjectsList)
			return;

		int row = m_ObjectsList.GetSelectedRow();
		if (row < 0)
			return;

		string name;
		m_ObjectsList.GetItemText(row, 0, name);
		if (name == string.Empty || name == m_SpawnClass)
			return;

		m_SpawnClass = name;
		if (m_TxtSelectedItem)
			m_TxtSelectedItem.SetText(name);

		ShowSpawnPreview(name);
	}

	void ShowSpawnPreview(string className)
	{
		ClearSpawnPreview();
		if (className == string.Empty)
			return;

		if (!VPPAttCatalog.Get().CanSpawnLocalPreview(className))
			return;

		m_SpawnPreviewObject = EntityAI.Cast(g_Game.CreateObjectEx(className, vector.Zero, ECE_LOCAL));
		if (!m_SpawnPreviewObject)
			m_SpawnPreviewObject = EntityAI.Cast(g_Game.CreateObject(className, vector.Zero, true, false, false));

		if (!m_SpawnPreview || !m_SpawnPreviewObject)
			return;

		m_SpawnPreviewObject.DisableSimulation(true);
		if (m_HideBroken)
			return;

		m_SpawnPreview.SetItem(m_SpawnPreviewObject);
		m_SpawnPreview.SetModelPosition(Vector(0, 0, 0.5));
		m_SpawnPreview.SetModelOrientation(Vector(0, 0, 0));
		if (m_SpawnPreview.GetItem())
			m_SpawnPreview.SetView(m_SpawnPreview.GetItem().GetViewIndex());

		ItemBase item = ItemBase.Cast(m_SpawnPreviewObject);
		bool stackable = false;
		int maxQty = 1;
		if (item)
		{
			Magazine mag = Magazine.Cast(item);
			if (mag)
			{
				stackable = true;
				maxQty = mag.GetAmmoMax();
			}
			else if (item.CanBeSplit() || item.HasQuantity())
			{
				stackable = true;
				maxQty = item.GetQuantityMax();
			}
		}

		if (maxQty < 1)
			maxQty = 1;

		if (m_SpawnQtySlider)
		{
			m_SpawnQtySlider.Show(stackable);
			if (stackable)
			{
				if (maxQty > 100)
					maxQty = 100;

				m_SpawnQtySlider.SetCurrent(maxQty);
			}
		}

		if (m_TxtSpawnQty)
			m_TxtSpawnQty.Show(stackable);

		ApplySpawnCondition();
	}

	void ClearSpawnPreview()
	{
		if (m_SpawnPreview)
			m_SpawnPreview.SetItem(NULL);

		if (m_SpawnPreviewObject)
		{
			g_Game.ObjectDelete(m_SpawnPreviewObject);
			m_SpawnPreviewObject = NULL;
		}
	}

	void ApplySpawnCondition()
	{
		string label = "Pristine";
		int color = ARGB(255, 34, 139, 34);
		if (m_SpawnHealth == 1)
		{
			label = "Worn";
			color = ARGB(255, 154, 205, 50);
		}
		else if (m_SpawnHealth == 2)
		{
			label = "Damaged";
			color = ARGB(255, 255, 255, 0);
		}
		else if (m_SpawnHealth == 3)
		{
			label = "Badly Damaged";
			color = ARGB(255, 255, 69, 0);
		}
		else if (m_SpawnHealth == 4)
		{
			label = "Ruined";
			color = ARGB(255, 139, 0, 0);
		}

		if (m_TxtCondition)
		{
			m_TxtCondition.SetText(label);
			m_TxtCondition.SetColor(color);
		}

		ItemBase item = ItemBase.Cast(m_SpawnPreviewObject);
		if (item)
			item.SetHealthLevel(m_SpawnHealth);
	}

	int ReadSpawnQty()
	{
		if (!m_SpawnQtySlider || !m_SpawnQtySlider.IsVisible())
			return 1;

		return m_SpawnQtySlider.GetCurrent();
	}

	void RequestTeleport()
	{
		if (m_SelectedSteamId == string.Empty)
		{
			if (GetVPPUIManager())
				GetVPPUIManager().DisplayNotification(Widget.TranslateString("#VSTR_NOTIFY_INV_SELECT"));

			return;
		}

		GetRPCManager().VSendRPC("RPC_VPPInventoryManager", "TeleportTo", new Param1<string>(m_SelectedSteamId), true);
	}

	void OpenAttachmentsBuilder()
	{
		if (m_SelectedSteamId == string.Empty)
		{
			if (GetVPPUIManager())
				GetVPPUIManager().DisplayNotification(Widget.TranslateString("#VSTR_NOTIFY_INV_SELECT"));

			return;
		}

		if (m_SpawnClass == string.Empty)
		{
			if (GetVPPUIManager())
				GetVPPUIManager().DisplayNotification(Widget.TranslateString("#VSTR_NOTIFY_INV_SPAWN_FAIL"));

			return;
		}

		VPPAdminHud hud = VPPAdminHud.Cast(GetVPPUIManager().GetMenuByType(VPPAdminHud));
		if (!hud)
			return;

		if (!hud.HasPermission("MenuAttachmentsBuilder"))
		{
			if (GetVPPUIManager())
				GetVPPUIManager().DisplayNotification(Widget.TranslateString("#VSTR_NOTIFY_INV_ATT"));

			return;
		}

		if (!hud.GetSubMenuByType(MenuAttachmentsBuilder))
			hud.CreateSubMenu(MenuAttachmentsBuilder);

		MenuAttachmentsBuilder attMenu = MenuAttachmentsBuilder.Cast(hud.GetSubMenuByType(MenuAttachmentsBuilder));
		if (!attMenu)
			return;

		if (!attMenu.IsSubMenuVisible())
			attMenu.ShowSubMenu();

		attMenu.OpenFromInventory(m_SpawnClass, m_SelectedSteamId, PlacementTypes.IN_INVENTORY);
	}

	void RequestSpawn()
	{
		if (m_SelectedSteamId == string.Empty)
		{
			if (GetVPPUIManager())
				GetVPPUIManager().DisplayNotification(Widget.TranslateString("#VSTR_NOTIFY_INV_SELECT"));

			return;
		}

		if (m_SpawnClass == string.Empty)
		{
			if (GetVPPUIManager())
				GetVPPUIManager().DisplayNotification(Widget.TranslateString("#VSTR_NOTIFY_INV_SPAWN_FAIL"));

			return;
		}

		bool attachments = false;
		if (m_ChkSpawnAttachments)
			attachments = m_ChkSpawnAttachments.IsChecked();

		GetRPCManager().VSendRPC("RPC_VPPInventoryManager", "SpawnItem", new Param6<string, string, int, int, bool, bool>(m_SelectedSteamId, m_SpawnClass, ReadSpawnQty(), m_SpawnHealth, true, attachments), true);
	}

	void SendItemAction(string rpcName, VPPInvItemInfo info, int extra)
	{
		if (!info || m_SelectedSteamId == string.Empty)
			return;

		if (rpcName == "SetQuantity" || rpcName == "SetHealth")
		{
			GetRPCManager().VSendRPC("RPC_VPPInventoryManager", rpcName, new Param4<string, int, int, int>(m_SelectedSteamId, info.m_NetLow, info.m_NetHigh, extra), true);
			return;
		}

		GetRPCManager().VSendRPC("RPC_VPPInventoryManager", rpcName, new Param3<string, int, int>(m_SelectedSteamId, info.m_NetLow, info.m_NetHigh), true);
	}

	void OnInvSetQty(VPPInvItemRow row)
	{
		if (!row || !row.GetInfo())
			return;

		SendItemAction("SetQuantity", row.GetInfo(), row.ReadQtyInput());
	}

	void OnInvMaxQty(VPPInvItemRow row)
	{
		if (!row || !row.GetInfo())
			return;

		SendItemAction("SetQuantity", row.GetInfo(), row.GetInfo().m_MaxQuantity);
	}

	void OnInvMinQty(VPPInvItemRow row)
	{
		if (!row || !row.GetInfo())
			return;

		SendItemAction("SetQuantity", row.GetInfo(), 1);
	}

	void OnInvLowerHealth(VPPInvItemRow row)
	{
		if (!row || !row.GetInfo())
			return;

		int health = row.GetInfo().m_HealthLevel + 1;
		if (health > 4)
			health = 4;

		SendItemAction("SetHealth", row.GetInfo(), health);
	}

	void OnInvRaiseHealth(VPPInvItemRow row)
	{
		if (!row || !row.GetInfo())
			return;

		int health = row.GetInfo().m_HealthLevel - 1;
		if (health < 0)
			health = 0;

		SendItemAction("SetHealth", row.GetInfo(), health);
	}

	void OnInvTakeItem(VPPInvItemRow row)
	{
		if (!row || !row.GetInfo())
			return;

		SendItemAction("TakeItem", row.GetInfo(), 0);
	}

	void OnInvDeleteItem(VPPInvItemRow row)
	{
		if (!row || !row.GetInfo())
			return;

		m_PendingDelete = row.GetInfo();
		VPPDialogBox dialog = GetVPPUIManager().CreateDialogBox(M_SUB_WIDGET);
		if (!dialog)
			return;

		dialog.InitDiagBox(DIAGTYPE.DIAG_YESNO, Widget.TranslateString("#VSTR_INV_DELETE_TITLE"), Widget.TranslateString("#VSTR_INV_DELETE_BODY") + " " + row.GetInfo().m_ClassName, this, "ConfirmDeleteItem");
	}

	void ConfirmDeleteItem(int result)
	{
		if (result != DIAGRESULT.YES)
			return;

		if (!m_PendingDelete)
			return;

		SendItemAction("DeleteItem", m_PendingDelete, 0);
		m_PendingDelete = NULL;
	}

	void ConfirmClearInventory(int result, string input)
	{
		if (result != DIAGRESULT.YES)
			return;

		if (m_SelectedSteamId == string.Empty)
			return;

		GetRPCManager().VSendRPC("RPC_VPPInventoryManager", "ClearInventory", new Param1<string>(m_SelectedSteamId), true);
	}

	void ConfirmRepairAll(int result, string input)
	{
		if (result != DIAGRESULT.YES)
			return;

		if (m_SelectedSteamId == string.Empty)
			return;

		GetRPCManager().VSendRPC("RPC_VPPInventoryManager", "RepairAll", new Param1<string>(m_SelectedSteamId), true);
	}
};
