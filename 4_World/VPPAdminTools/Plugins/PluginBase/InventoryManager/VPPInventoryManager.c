class VPPInventoryManager extends PluginBase
{
	protected ref array<string> m_UnfinishedItems;
	protected ref array<string> m_RestrictedClassNames;

	void VPPInventoryManager()
	{
		m_UnfinishedItems = new array<string>;
		m_UnfinishedItems.Insert("quickiebow");
		m_UnfinishedItems.Insert("recurvebow");
		m_UnfinishedItems.Insert("gp25base");
		m_UnfinishedItems.Insert("gp25");
		m_UnfinishedItems.Insert("gp25_standalone");
		m_UnfinishedItems.Insert("m203base");
		m_UnfinishedItems.Insert("m203");
		m_UnfinishedItems.Insert("m203_standalone");
		m_UnfinishedItems.Insert("red9");
		m_UnfinishedItems.Insert("pvcbow");
		m_UnfinishedItems.Insert("crossbow");
		m_UnfinishedItems.Insert("m249");
		m_UnfinishedItems.Insert("undersluggrenadem4");
		m_UnfinishedItems.Insert("groza");
		m_UnfinishedItems.Insert("pm73rak");
		m_UnfinishedItems.Insert("trumpet");
		m_UnfinishedItems.Insert("lawbase");
		m_UnfinishedItems.Insert("law");
		m_UnfinishedItems.Insert("rpg7base");
		m_UnfinishedItems.Insert("rpg7");
		m_UnfinishedItems.Insert("dartgun");
		m_UnfinishedItems.Insert("shockpistol");
		m_UnfinishedItems.Insert("shockpistol_black");
		m_UnfinishedItems.Insert("fnx45_arrow");
		m_UnfinishedItems.Insert("makarovpb");
		m_UnfinishedItems.Insert("mp133shotgun_pistolgrip");
		m_UnfinishedItems.Insert("largetentbackpack");
		m_UnfinishedItems.Insert("splint_applied");
		m_UnfinishedItems.Insert("leatherbelt_natural");
		m_UnfinishedItems.Insert("leatherbelt_beige");
		m_UnfinishedItems.Insert("leatherbelt_brown");
		m_UnfinishedItems.Insert("leatherbelt_black");
		m_UnfinishedItems.Insert("leatherknifesheath");

		m_RestrictedClassNames = new array<string>;
		m_RestrictedClassNames.Insert("placing");
		m_RestrictedClassNames.Insert("debug");
		m_RestrictedClassNames.Insert("bldr_");
		m_RestrictedClassNames.Insert("land_");
		m_RestrictedClassNames.Insert("staticobj_");

		GetRPCManager().AddRPC("RPC_VPPInventoryManager", "RequestPlayers", this, SingleplayerExecutionType.Server);
		GetRPCManager().AddRPC("RPC_VPPInventoryManager", "RequestInventory", this, SingleplayerExecutionType.Server);
		GetRPCManager().AddRPC("RPC_VPPInventoryManager", "DeleteItem", this, SingleplayerExecutionType.Server);
		GetRPCManager().AddRPC("RPC_VPPInventoryManager", "TakeItem", this, SingleplayerExecutionType.Server);
		GetRPCManager().AddRPC("RPC_VPPInventoryManager", "SetQuantity", this, SingleplayerExecutionType.Server);
		GetRPCManager().AddRPC("RPC_VPPInventoryManager", "SetHealth", this, SingleplayerExecutionType.Server);
		GetRPCManager().AddRPC("RPC_VPPInventoryManager", "ClearInventory", this, SingleplayerExecutionType.Server);
		GetRPCManager().AddRPC("RPC_VPPInventoryManager", "RepairAll", this, SingleplayerExecutionType.Server);
		GetRPCManager().AddRPC("RPC_VPPInventoryManager", "TeleportTo", this, SingleplayerExecutionType.Server);
		GetRPCManager().AddRPC("RPC_VPPInventoryManager", "SpawnItem", this, SingleplayerExecutionType.Server);
		RegisterPerms();
	}

	override void OnInit()
	{
		super.OnInit();
		RegisterPerms();
	}

	void RegisterPerms()
	{
		if (!GetPermissionManager())
			return;

		GetPermissionManager().AddPermissionType({"MenuInventoryManager", "MenuInventoryManager:Delete", "MenuInventoryManager:Take", "MenuInventoryManager:Edit", "MenuInventoryManager:Clear", "MenuInventoryManager:RepairAll", "MenuInventoryManager:Teleport", "MenuInventoryManager:Spawn"});
	}

	protected bool HasMenuPerm(PlayerIdentity sender)
	{
		if (!sender)
			return false;

		return GetPermissionManager().VerifyPermission(sender.GetPlainId(), "MenuInventoryManager");
	}

	protected bool HasActionPerm(PlayerIdentity sender, string perm)
	{
		if (!HasMenuPerm(sender))
			return false;

		return GetPermissionManager().VerifyPermission(sender.GetPlainId(), perm);
	}

	protected PlayerBase FindPlayer(string steamId)
	{
		if (steamId == string.Empty)
			return NULL;

		return GetPermissionManager().GetPlayerBaseByID(steamId);
	}

	protected ItemBase FindItem(PlayerBase player, int netLow, int netHigh)
	{
		if (!player || !player.GetInventory())
			return NULL;

		array<EntityAI> items = new array<EntityAI>;
		player.GetInventory().EnumerateInventory(InventoryTraversalType.INORDER, items);

		int i;
		int low;
		int high;
		EntityAI entity;
		ItemBase item;
		for (i = 0; i < items.Count(); i++)
		{
			entity = items.Get(i);
			if (!entity)
				continue;

			entity.GetNetworkID(low, high);
			if (low != netLow || high != netHigh)
				continue;

			item = ItemBase.Cast(entity);
			if (item)
				return item;
		}

		return NULL;
	}

	protected bool IsItemStackable(ItemBase item)
	{
		if (!item)
			return false;

		if (Magazine.Cast(item))
			return true;

		return item.CanBeSplit() || item.HasQuantity();
	}

	protected int ReadQuantity(ItemBase item)
	{
		Magazine mag;
		if (Class.CastTo(mag, item))
			return mag.GetAmmoCount();

		if (item && item.HasQuantity())
			return item.GetQuantity();

		return 1;
	}

	protected int ReadMaxQuantity(ItemBase item)
	{
		Magazine mag;
		if (Class.CastTo(mag, item))
			return mag.GetAmmoMax();

		if (item)
			return item.GetQuantityMax();

		return 1;
	}

	protected void ApplyQuantity(ItemBase item, int quantity)
	{
		Magazine mag;
		if (Class.CastTo(mag, item))
		{
			mag.ServerSetAmmoCount(quantity);
			return;
		}

		item.SetQuantity(quantity);
	}

	protected VPPInvItemInfo BuildItemInfo(ItemBase item)
	{
		VPPInvItemInfo info = new VPPInvItemInfo();
		if (!item)
			return info;

		item.GetNetworkID(info.m_NetLow, info.m_NetHigh);
		info.m_ClassName = item.GetType();
		info.m_DisplayName = item.GetDisplayName();
		info.m_Quantity = ReadQuantity(item);
		info.m_MaxQuantity = ReadMaxQuantity(item);
		info.m_HealthLevel = item.GetHealthLevel();
		info.m_Stackable = IsItemStackable(item);
		return info;
	}

	protected void SendInventory(PlayerIdentity sender, PlayerBase player)
	{
		array<ref VPPInvItemInfo> list = new array<ref VPPInvItemInfo>;
		string steamId = "";
		vector pos = vector.Zero;

		if (player && player.GetIdentity())
		{
			steamId = player.GetIdentity().GetPlainId();
			pos = player.GetPosition();
			if (player.GetInventory())
			{
				array<EntityAI> items = new array<EntityAI>;
				player.GetInventory().EnumerateInventory(InventoryTraversalType.INORDER, items);
				int i;
				ItemBase item;
				for (i = 0; i < items.Count(); i++)
				{
					item = ItemBase.Cast(items.Get(i));
					if (!item)
						continue;

					list.Insert(BuildItemInfo(item));
				}
			}
		}

		GetRPCManager().VSendRPC("RPC_MenuInventoryManager", "ReceiveInventory", new Param3<string, vector, ref array<ref VPPInvItemInfo>>(steamId, pos, list), true, sender);
	}

	void RequestPlayers(CallType type, ParamsReadContext ctx, PlayerIdentity sender, Object target)
	{
		if (type != CallType.Server)
			return;

		if (!HasMenuPerm(sender))
			return;

		array<ref VPPInvPlayerInfo> list = new array<ref VPPInvPlayerInfo>;
		array<Man> players = new array<Man>;
		g_Game.GetPlayers(players);

		int i;
		PlayerBase player;
		for (i = 0; i < players.Count(); i++)
		{
			player = PlayerBase.Cast(players.Get(i));
			if (!player || !player.GetIdentity())
				continue;

			list.Insert(new VPPInvPlayerInfo(player.GetIdentity().GetPlainId(), player.GetIdentity().GetName()));
		}

		GetRPCManager().VSendRPC("RPC_MenuInventoryManager", "ReceivePlayers", new Param1<ref array<ref VPPInvPlayerInfo>>(list), true, sender);
	}

	void RequestInventory(CallType type, ParamsReadContext ctx, PlayerIdentity sender, Object target)
	{
		if (type != CallType.Server)
			return;

		if (!HasMenuPerm(sender))
			return;

		Param1<string> data;
		if (!ctx.Read(data))
			return;

		PlayerBase player = FindPlayer(data.param1);
		if (!player)
		{
			GetPermissionManager().NotifyPlayer(sender.GetPlainId(), Widget.TranslateString("#VSTR_NOTIFY_INV_NOTFOUND"), NotifyTypes.NOTIFY);
			SendInventory(sender, NULL);
			return;
		}

		SendInventory(sender, player);
	}

	void DeleteItem(CallType type, ParamsReadContext ctx, PlayerIdentity sender, Object target)
	{
		if (type != CallType.Server)
			return;

		if (!HasActionPerm(sender, "MenuInventoryManager:Delete"))
			return;

		Param3<string, int, int> data;
		if (!ctx.Read(data))
			return;

		PlayerBase player = FindPlayer(data.param1);
		ItemBase item = FindItem(player, data.param2, data.param3);
		if (!player || !item)
		{
			GetPermissionManager().NotifyPlayer(sender.GetPlainId(), Widget.TranslateString("#VSTR_NOTIFY_INV_NOITEM"), NotifyTypes.NOTIFY);
			return;
		}

		string itemName = item.GetDisplayName();
		item.Delete();
		GetPermissionManager().NotifyPlayer(sender.GetPlainId(), Widget.TranslateString("#VSTR_NOTIFY_INV_DELETED") + " " + itemName, NotifyTypes.NOTIFY);
		if (GetSimpleLogger())
			GetSimpleLogger().Log(string.Format("\"%1\" (steamid=%2) deleted %3 from %4", sender.GetName(), sender.GetPlainId(), itemName, player.GetIdentity().GetName()));

		SendInventory(sender, player);
	}

	void TakeItem(CallType type, ParamsReadContext ctx, PlayerIdentity sender, Object target)
	{
		if (type != CallType.Server)
			return;

		if (!HasActionPerm(sender, "MenuInventoryManager:Take"))
			return;

		Param3<string, int, int> data;
		if (!ctx.Read(data))
			return;

		PlayerBase targetPlayer = FindPlayer(data.param1);
		PlayerBase admin = FindPlayer(sender.GetPlainId());
		ItemBase item = FindItem(targetPlayer, data.param2, data.param3);
		if (!targetPlayer || !admin || !item)
		{
			GetPermissionManager().NotifyPlayer(sender.GetPlainId(), Widget.TranslateString("#VSTR_NOTIFY_INV_TAKE_FAIL"), NotifyTypes.NOTIFY);
			return;
		}

		if (admin == targetPlayer)
		{
			GetPermissionManager().NotifyPlayer(sender.GetPlainId(), Widget.TranslateString("#VSTR_NOTIFY_INV_TAKE_SELF"), NotifyTypes.NOTIFY);
			return;
		}

		GameInventory adminInv = admin.GetInventory();
		GameInventory targetInv = targetPlayer.GetInventory();
		if (!adminInv || !targetInv || !item.GetInventory())
		{
			GetPermissionManager().NotifyPlayer(sender.GetPlainId(), Widget.TranslateString("#VSTR_NOTIFY_INV_TAKE_FAIL"), NotifyTypes.NOTIFY);
			return;
		}

		InventoryLocation src = new InventoryLocation();
		item.GetInventory().GetCurrentInventoryLocation(src);

		InventoryLocation dst = new InventoryLocation();
		if (!adminInv.FindFreeLocationFor(item, FindInventoryLocationType.CARGO, dst))
		{
			GetPermissionManager().NotifyPlayer(sender.GetPlainId(), Widget.TranslateString("#VSTR_NOTIFY_INV_TAKE_SPACE"), NotifyTypes.NOTIFY);
			return;
		}

		if (!targetInv.LocationCanRemoveEntity(src))
		{
			GetPermissionManager().NotifyPlayer(sender.GetPlainId(), Widget.TranslateString("#VSTR_NOTIFY_INV_TAKE_FAIL"), NotifyTypes.NOTIFY);
			return;
		}

		g_Game.ClearJuncture(admin, item);
		adminInv.ClearInventoryReservation(item, dst);
		bool moved = targetInv.LocationSyncMoveEntity(src, dst);
		if (moved && dst.IsValid())
			InventoryInputUserData.SendServerMove(null, InventoryCommandType.SYNC_MOVE, src, dst);

		if (!moved)
		{
			GetPermissionManager().NotifyPlayer(sender.GetPlainId(), Widget.TranslateString("#VSTR_NOTIFY_INV_TAKE_FAIL"), NotifyTypes.NOTIFY);
			return;
		}

		GetPermissionManager().NotifyPlayer(sender.GetPlainId(), Widget.TranslateString("#VSTR_NOTIFY_INV_TOOK") + " " + item.GetDisplayName(), NotifyTypes.NOTIFY);
		if (GetSimpleLogger())
			GetSimpleLogger().Log(string.Format("\"%1\" (steamid=%2) took %3 from %4", sender.GetName(), sender.GetPlainId(), item.GetDisplayName(), targetPlayer.GetIdentity().GetName()));

		SendInventory(sender, targetPlayer);
	}

	void SetQuantity(CallType type, ParamsReadContext ctx, PlayerIdentity sender, Object target)
	{
		if (type != CallType.Server)
			return;

		if (!HasActionPerm(sender, "MenuInventoryManager:Edit"))
			return;

		Param4<string, int, int, int> data;
		if (!ctx.Read(data))
			return;

		PlayerBase player = FindPlayer(data.param1);
		ItemBase item = FindItem(player, data.param2, data.param3);
		if (!player || !item || !IsItemStackable(item))
		{
			GetPermissionManager().NotifyPlayer(sender.GetPlainId(), Widget.TranslateString("#VSTR_NOTIFY_INV_QTY_FAIL"), NotifyTypes.NOTIFY);
			return;
		}

		int quantity = data.param4;
		int maxQty = ReadMaxQuantity(item);
		if (quantity < 1 || quantity > maxQty)
		{
			GetPermissionManager().NotifyPlayer(sender.GetPlainId(), Widget.TranslateString("#VSTR_NOTIFY_INV_QTY_FAIL"), NotifyTypes.NOTIFY);
			return;
		}

		ApplyQuantity(item, quantity);
		GetPermissionManager().NotifyPlayer(sender.GetPlainId(), Widget.TranslateString("#VSTR_NOTIFY_INV_QTY") + " " + quantity.ToString(), NotifyTypes.NOTIFY);
		if (GetSimpleLogger())
			GetSimpleLogger().Log(string.Format("\"%1\" (steamid=%2) set quantity of %3 to %4 on %5", sender.GetName(), sender.GetPlainId(), item.GetDisplayName(), quantity, player.GetIdentity().GetName()));

		SendInventory(sender, player);
	}

	void SetHealth(CallType type, ParamsReadContext ctx, PlayerIdentity sender, Object target)
	{
		if (type != CallType.Server)
			return;

		if (!HasActionPerm(sender, "MenuInventoryManager:Edit"))
			return;

		Param4<string, int, int, int> data;
		if (!ctx.Read(data))
			return;

		PlayerBase player = FindPlayer(data.param1);
		ItemBase item = FindItem(player, data.param2, data.param3);
		if (!player || !item)
		{
			GetPermissionManager().NotifyPlayer(sender.GetPlainId(), Widget.TranslateString("#VSTR_NOTIFY_INV_NOITEM"), NotifyTypes.NOTIFY);
			return;
		}

		int health = data.param4;
		if (health < 0)
			health = 0;

		if (health > 4)
			health = 4;

		item.SetHealthLevel(health);
		if (GetSimpleLogger())
			GetSimpleLogger().Log(string.Format("\"%1\" (steamid=%2) set health of %3 to %4 on %5", sender.GetName(), sender.GetPlainId(), item.GetDisplayName(), health, player.GetIdentity().GetName()));

		SendInventory(sender, player);
	}

	void ClearInventory(CallType type, ParamsReadContext ctx, PlayerIdentity sender, Object target)
	{
		if (type != CallType.Server)
			return;

		if (!HasActionPerm(sender, "MenuInventoryManager:Clear"))
			return;

		Param1<string> data;
		if (!ctx.Read(data))
			return;

		PlayerBase player = FindPlayer(data.param1);
		if (!player)
		{
			GetPermissionManager().NotifyPlayer(sender.GetPlainId(), Widget.TranslateString("#VSTR_NOTIFY_INV_NOTFOUND"), NotifyTypes.NOTIFY);
			return;
		}

		player.RemoveAllItems();
		GetPermissionManager().NotifyPlayer(sender.GetPlainId(), Widget.TranslateString("#VSTR_NOTIFY_INV_CLEARED") + " " + player.GetIdentity().GetName(), NotifyTypes.NOTIFY);
		if (GetSimpleLogger())
			GetSimpleLogger().Log(string.Format("\"%1\" (steamid=%2) cleared inventory of %3", sender.GetName(), sender.GetPlainId(), player.GetIdentity().GetName()));

		SendInventory(sender, player);
	}

	void RepairAll(CallType type, ParamsReadContext ctx, PlayerIdentity sender, Object target)
	{
		if (type != CallType.Server)
			return;

		if (!HasActionPerm(sender, "MenuInventoryManager:RepairAll"))
			return;

		Param1<string> data;
		if (!ctx.Read(data))
			return;

		PlayerBase player = FindPlayer(data.param1);
		if (!player || !player.GetInventory())
		{
			GetPermissionManager().NotifyPlayer(sender.GetPlainId(), Widget.TranslateString("#VSTR_NOTIFY_INV_NOTFOUND"), NotifyTypes.NOTIFY);
			return;
		}

		array<EntityAI> items = new array<EntityAI>;
		player.GetInventory().EnumerateInventory(InventoryTraversalType.INORDER, items);
		int i;
		ItemBase item;
		for (i = 0; i < items.Count(); i++)
		{
			item = ItemBase.Cast(items.Get(i));
			if (item)
				item.SetHealthLevel(0);
		}

		GetPermissionManager().NotifyPlayer(sender.GetPlainId(), Widget.TranslateString("#VSTR_NOTIFY_INV_REPAIRED") + " " + player.GetIdentity().GetName(), NotifyTypes.NOTIFY);
		if (GetSimpleLogger())
			GetSimpleLogger().Log(string.Format("\"%1\" (steamid=%2) repaired inventory of %3", sender.GetName(), sender.GetPlainId(), player.GetIdentity().GetName()));

		SendInventory(sender, player);
	}

	void TeleportTo(CallType type, ParamsReadContext ctx, PlayerIdentity sender, Object target)
	{
		if (type != CallType.Server)
			return;

		if (!HasActionPerm(sender, "MenuInventoryManager:Teleport"))
			return;

		Param1<string> data;
		if (!ctx.Read(data))
			return;

		PlayerBase targetPlayer = FindPlayer(data.param1);
		PlayerBase admin = FindPlayer(sender.GetPlainId());
		if (!targetPlayer || !admin)
		{
			GetPermissionManager().NotifyPlayer(sender.GetPlainId(), Widget.TranslateString("#VSTR_NOTIFY_INV_NOTFOUND"), NotifyTypes.NOTIFY);
			return;
		}

		admin.SetPosition(targetPlayer.GetPosition());
		GetPermissionManager().NotifyPlayer(sender.GetPlainId(), Widget.TranslateString("#VSTR_NOTIFY_INV_TP") + " " + targetPlayer.GetIdentity().GetName(), NotifyTypes.NOTIFY);
		if (GetSimpleLogger())
			GetSimpleLogger().Log(string.Format("\"%1\" (steamid=%2) teleported to %3", sender.GetName(), sender.GetPlainId(), targetPlayer.GetIdentity().GetName()));
	}

	void SpawnItem(CallType type, ParamsReadContext ctx, PlayerIdentity sender, Object target)
	{
		if (type != CallType.Server)
			return;

		if (!HasActionPerm(sender, "MenuInventoryManager:Spawn"))
			return;

		Param6<string, string, int, int, bool, bool> data;
		if (!ctx.Read(data))
			return;

		string steamId = data.param1;
		string className = data.param2;
		int quantity = data.param3;
		int health = data.param4;
		bool atFeet = data.param5;
		bool attachments = data.param6;

		if (className == string.Empty)
		{
			GetPermissionManager().NotifyPlayer(sender.GetPlainId(), Widget.TranslateString("#VSTR_NOTIFY_INV_SPAWN_FAIL"), NotifyTypes.NOTIFY);
			return;
		}

		if (health < 0 || health > 4)
		{
			GetPermissionManager().NotifyPlayer(sender.GetPlainId(), Widget.TranslateString("#VSTR_NOTIFY_INV_SPAWN_FAIL"), NotifyTypes.NOTIFY);
			return;
		}

		PlayerBase player = FindPlayer(steamId);
		if (!player)
		{
			GetPermissionManager().NotifyPlayer(sender.GetPlainId(), Widget.TranslateString("#VSTR_NOTIFY_INV_NOTFOUND"), NotifyTypes.NOTIFY);
			return;
		}

		ItemBase item;
		if (!atFeet && player.GetInventory())
			item = ItemBase.Cast(player.GetInventory().CreateInInventory(className));

		if (!item)
			item = ItemBase.Cast(g_Game.CreateObject(className, player.GetPosition(), false, true));

		if (!item)
		{
			GetPermissionManager().NotifyPlayer(sender.GetPlainId(), Widget.TranslateString("#VSTR_NOTIFY_INV_SPAWN_FAIL"), NotifyTypes.NOTIFY);
			return;
		}

		ApplySpawnParams(item, quantity, health, attachments);
		if (atFeet)
			GetPermissionManager().NotifyPlayer(sender.GetPlainId(), Widget.TranslateString("#VSTR_NOTIFY_INV_SPAWN_FEET") + " " + className, NotifyTypes.NOTIFY);
		else
			GetPermissionManager().NotifyPlayer(sender.GetPlainId(), Widget.TranslateString("#VSTR_NOTIFY_INV_SPAWN_INV") + " " + className + " " + player.GetIdentity().GetName(), NotifyTypes.NOTIFY);

		if (GetSimpleLogger())
			GetSimpleLogger().Log(string.Format("\"%1\" (steamid=%2) spawned %3 for %4", sender.GetName(), sender.GetPlainId(), className, player.GetIdentity().GetName()));

		SendInventory(sender, player);
	}

	protected void ApplySpawnParams(ItemBase item, int quantity, int health, bool attachments)
	{
		if (!item)
			return;

		if (IsItemStackable(item))
		{
			int maxQty = ReadMaxQuantity(item);
			int qty = quantity;
			if (qty < 1)
				qty = 1;

			if (qty > maxQty)
				qty = maxQty;

			ApplyQuantity(item, qty);
		}

		item.SetHealthLevel(health);
		if (attachments)
			SpawnCompatibleAttachments(item, 3);
	}

	protected bool IsExcludedClassName(string className)
	{
		string lower = className;
		lower.ToLower();

		int i;
		string restricted;
		for (i = 0; i < m_UnfinishedItems.Count(); i++)
		{
			if (lower.Contains(m_UnfinishedItems.Get(i)))
				return true;
		}

		for (i = 0; i < m_RestrictedClassNames.Count(); i++)
		{
			restricted = m_RestrictedClassNames.Get(i);
			if (lower.Contains(restricted))
				return true;
		}

		return false;
	}

	protected void SpawnChildAttachments(EntityAI entity, int depth)
	{
		if (!entity || !entity.GetInventory())
			return;

		if (!entity.GetInventory().GetAttachmentSlotsCount())
			return;

		if (entity.GetInventory().AttachmentCount())
			return;

		if (entity.IsInherited(DayZCreature) || entity.IsInherited(TentBase) || entity.IsInherited(Weapon_Base))
			return;

		SpawnCompatibleAttachments(entity, depth);
	}

	protected void SpawnCompatibleAttachments(EntityAI entity, int depth)
	{
		if (!entity)
			return;

		TStringArray atts = new TStringArray;
		entity.ConfigGetTextArray("attachments", atts);

		TIntArray slotIds = new TIntArray;
		int i;
		int slotId;
		string att;
		for (i = 0; i < atts.Count(); i++)
		{
			att = atts.Get(i);
			slotId = InventorySlots.GetSlotIdFromString(att);
			if (slotId != InventorySlots.INVALID && InventorySlots.GetShowForSlotId(slotId))
				slotIds.Insert(slotId);
		}

		TStringArray paths = new TStringArray;
		paths.Insert(CFG_VEHICLESPATH);
		paths.Insert(CFG_WEAPONSPATH);

		int p;
		int n;
		int children;
		int scope;
		int idx;
		string path;
		string childName;
		string model;
		string invSlot;
		TStringArray invSlots;
		EntityAI child;
		for (p = 0; p < paths.Count(); p++)
		{
			path = paths.Get(p);
			children = g_Game.ConfigGetChildrenCount(path);
			for (n = 0; n < children; n++)
			{
				g_Game.ConfigGetChildName(path, n, childName);
				scope = g_Game.ConfigGetInt(path + " " + childName + " scope");
				if (scope != 2)
					continue;

				if (!g_Game.ConfigGetText(path + " " + childName + " model", model) || model == string.Empty)
					continue;

				invSlots = new TStringArray;
				g_Game.ConfigGetTextArray(path + " " + childName + " inventorySlot", invSlots);
				int s;
				for (s = 0; s < invSlots.Count(); s++)
				{
					invSlot = invSlots.Get(s);
					slotId = InventorySlots.GetSlotIdFromString(invSlot);
					if (slotId == InventorySlots.INVALID)
						continue;

					idx = slotIds.Find(slotId);
					if (idx < 0)
						continue;

					if (IsExcludedClassName(childName))
						break;

					child = entity.GetInventory().CreateAttachmentEx(childName, slotId);
					if (child)
					{
						slotIds.Remove(idx);
						if (depth > 0)
							SpawnChildAttachments(child, depth - 1);

						if (slotIds.Count() == 0)
							return;
					}
				}
			}
		}
	}
};
