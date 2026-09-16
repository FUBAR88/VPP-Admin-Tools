class VPPAttachmentsSpawner extends PluginBase
{
	void VPPAttachmentsSpawner()
	{
		GetRPCManager().AddRPC("RPC_VPPAttachmentsSpawner", "SpawnAttachments", this, SingleplayerExecutionType.Server);
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

		GetPermissionManager().AddPermissionType({"MenuAttachmentsBuilder", "MenuAttachmentsBuilder:SpawnWeapons", "MenuAttachmentsBuilder:SpawnVehicles", "MenuAttachmentsBuilder:SpawnClothes"});
	}

	void SpawnAttachments(CallType type, ParamsReadContext ctx, PlayerIdentity sender, Object target)
	{
		if (type != CallType.Server)
			return;

		if (!sender)
			return;

		Param1<ref VPPAttSpawnParams> data;
		if (!ctx.Read(data))
			return;

		VPPAttSpawnParams params = data.param1;
		if (!params)
			return;

		if (!GetPermissionManager().VerifyPermission(sender.GetPlainId(), "MenuAttachmentsBuilder"))
			return;

		if (!VerifySpawnPerm(sender.GetPlainId(), params.m_Category))
			return;

		if (!params.m_Types || params.m_Types.Count() == 0)
			return;

		array<string> targetIds = params.m_Targets;
		if (!targetIds)
			targetIds = new array<string>;

		if (targetIds.Count() == 0)
			targetIds.Insert(sender.GetPlainId());

		int i;
		string targetId;
		PlayerBase player;
		bool useCursor;
		for (i = 0; i < targetIds.Count(); i++)
		{
			targetId = targetIds.Get(i);
			player = GetPermissionManager().GetPlayerBaseByID(targetId);
			if (!player)
				continue;

			useCursor = false;
			if (targetId == sender.GetPlainId() && params.m_PlacementType == PlacementTypes.AT_CROSSHAIR)
				useCursor = true;

			SpawnForPlayer(params, player, useCursor);
		}

		GetPermissionManager().NotifyPlayer(sender.GetPlainId(), Widget.TranslateString("#VSTR_NOTIFY_ATT_SPAWN"), NotifyTypes.NOTIFY);
		if (GetSimpleLogger())
			GetSimpleLogger().Log(string.Format("\"%1\" (steamid=%2) spawned attachments (%3 types)", sender.GetName(), sender.GetPlainId(), params.m_Types.Count()));
	}

	protected bool VerifySpawnPerm(string steamId, int category)
	{
		if (category == VPPAttCatalog.ATT_CAT_VEHICLES)
			return GetPermissionManager().VerifyPermission(steamId, "MenuAttachmentsBuilder:SpawnVehicles");

		if (category == VPPAttCatalog.ATT_CAT_CLOTHES)
			return GetPermissionManager().VerifyPermission(steamId, "MenuAttachmentsBuilder:SpawnClothes");

		return GetPermissionManager().VerifyPermission(steamId, "MenuAttachmentsBuilder:SpawnWeapons");
	}

	protected void SpawnForPlayer(VPPAttSpawnParams params, PlayerBase player, bool useCursor)
	{
		array<EntityAI> hosts = new array<EntityAI>;
		int i;
		string typeName;
		EntityAI created;
		bool fillVehicle;

		fillVehicle = false;
		if (params.m_Category == VPPAttCatalog.ATT_CAT_VEHICLES)
			fillVehicle = true;

		VPPAttCatalog.Log("spawn cat=" + params.m_Category.ToString() + " count=" + params.m_Types.Count().ToString() + " place=" + params.m_PlacementType.ToString());
		for (i = 0; i < params.m_Types.Count(); i++)
		{
			typeName = params.m_Types.Get(i);
			if (typeName == string.Empty)
				continue;

			created = AttachToHosts(hosts, typeName, fillVehicle);
			if (created)
			{
				VPPAttCatalog.Log("spawn-attached " + typeName + " -> " + created.GetType());
				continue;
			}

			created = CreateLoose(typeName, params, player, useCursor);
			if (created)
			{
				VPPAttCatalog.Log("spawn-loose " + typeName);
				VPPAttAttach.EmptyMag(created, false);
				hosts.Insert(created);
			}
			else
			{
				VPPAttCatalog.Log("spawn-fail " + typeName);
			}
		}
	}

	protected EntityAI AttachToHosts(array<EntityAI> hosts, string typeName, bool fillVehicle)
	{
		int i;
		EntityAI host;
		EntityAI created;
		EntityAI last;

		for (i = hosts.Count() - 1; i >= 0; i--)
		{
			host = hosts.Get(i);
			if (!host)
				continue;

			created = VPPAttAttach.AttachTo(host, typeName, false);
			if (!created)
				continue;

			if (created == host)
				return host;

			hosts.Insert(created);
			if (!fillVehicle)
				return created;

			if (!Transport.Cast(host))
				return created;

			last = created;
			created = VPPAttAttach.AttachTo(host, typeName, false);
			while (created && created != host)
			{
				hosts.Insert(created);
				last = created;
				created = VPPAttAttach.AttachTo(host, typeName, false);
			}

			return last;
		}

		return NULL;
	}

	protected EntityAI CreateLoose(string typeName, VPPAttSpawnParams params, PlayerBase player, bool useCursor)
	{
		int placement = params.m_PlacementType;
		if (g_Game.IsKindOf(typeName, "transport") && placement == PlacementTypes.IN_INVENTORY)
			placement = PlacementTypes.ON_GROUND;

		if (placement == PlacementTypes.IN_INVENTORY)
		{
			EntityAI inInv = NULL;
			if (player.GetInventory())
				inInv = player.GetInventory().CreateInInventory(typeName);

			if (inInv)
				return inInv;

			return CreateWorld(typeName, player.GetPosition());
		}

		if (useCursor)
			return CreateWorld(typeName, params.m_Position);

		return CreateWorld(typeName, player.GetPosition());
	}

	protected EntityAI CreateWorld(string typeName, vector position)
	{
		int flags = ECE_SETUP | ECE_KEEPHEIGHT | ECE_PLACE_ON_SURFACE;
		EntityAI item = EntityAI.Cast(g_Game.CreateObjectEx(typeName, position, flags));
		if (item && Transport.Cast(item))
			dBodyApplyImpulse(item, vector.Up);

		return item;
	}
};

VPPAttachmentsSpawner GetVPPAttachmentsSpawner()
{
	PluginManager mgr = GetPluginManager();
	if (!mgr)
		return NULL;

	return VPPAttachmentsSpawner.Cast(mgr.GetPluginByType(VPPAttachmentsSpawner));
};
