class VPPAttAttach
{
	static EntityAI AttachTo(EntityAI host, string typeName, bool localPreview)
	{
		if (!host || typeName == string.Empty)
			return NULL;

		string hostType = host.GetType();
		if (FillMag(host, typeName, localPreview))
		{
			VPPAttCatalog.Log("fill-mag host=" + hostType + " ammo=" + typeName);
			return host;
		}

		if (localPreview && VPPAttCatalog.Get().IsUnsafeLocalAttach(typeName))
		{
			VPPAttCatalog.Log("skip-preview-attach host=" + hostType + " part=" + typeName);
			return NULL;
		}

		Weapon_Base weapon = Weapon_Base.Cast(host);
		if (weapon && VPPAttCatalog.Get().IsWeaponMagazine(typeName))
			return AttachWeaponMagazine(weapon, typeName, localPreview);

		if (FirstFreeSlot(host, typeName) == InventorySlots.INVALID)
			return NULL;

		EntityAI created = AttachCreated(host, typeName, localPreview);
		if (created)
		{
			VPPAttCatalog.Log("attach-slot host=" + hostType + " part=" + typeName);
			EmptyMag(created, localPreview);
			return created;
		}

		VPPAttCatalog.Log("attach-fail host=" + hostType + " part=" + typeName + " slots=" + SlotList(typeName));
		return NULL;
	}

	static string SlotList(string typeName)
	{
		TStringArray slots = new TStringArray;
		VPPAttCatalog.Get().CollectInventorySlots(typeName, slots);
		if (!slots || slots.Count() == 0)
			return "-";

		string text = slots.Get(0);
		int i;
		for (i = 1; i < slots.Count(); i++)
		{
			text = text + "," + slots.Get(i);
		}

		return text;
	}

	static EntityAI AttachWeaponMagazine(Weapon_Base weapon, string typeName, bool localPreview)
	{
		if (!weapon)
			return NULL;

		if (localPreview)
		{
			VPPAttCatalog.Log("skip-preview-mag weapon=" + weapon.GetType() + " mag=" + typeName);
			return NULL;
		}

		if (VPPAttCatalog.Get().IsWeaponBroken(weapon.GetType()))
		{
			VPPAttCatalog.Log("skip-broken-mag weapon=" + weapon.GetType() + " mag=" + typeName);
			return NULL;
		}

		if (weapon.GetMagazine(0))
		{
			VPPAttCatalog.Log("skip-extra-mag weapon=" + weapon.GetType() + " mag=" + typeName);
			return NULL;
		}

		EntityAI created = AttachCreated(weapon, typeName, false);
		Magazine mag = Magazine.Cast(created);
		if (!mag)
			return NULL;

		EmptyMag(mag, false);
		weapon.RandomizeFSMState();
		weapon.Synchronize();
		VPPAttCatalog.Log("attach-mag weapon=" + weapon.GetType() + " mag=" + typeName);
		return mag;
	}

	static EntityAI CreatePart(string typeName, EntityAI host, bool localPreview)
	{
		if (typeName == string.Empty)
			return NULL;

		if (localPreview)
		{
			if (VPPAttCatalog.Get().IsUnsafeLocalPreview(typeName))
				return NULL;

			EntityAI localItem = EntityAI.Cast(g_Game.CreateObjectEx(typeName, vector.Zero, ECE_LOCAL));
			if (localItem)
				return localItem;

			return EntityAI.Cast(g_Game.CreateObject(typeName, vector.Zero, true, false, false));
		}

		if (!host)
			return NULL;

		return EntityAI.Cast(g_Game.CreateObjectEx(typeName, host.GetPosition(), ECE_SETUP | ECE_KEEPHEIGHT | ECE_PLACE_ON_SURFACE));
	}

	static int FirstFreeSlot(EntityAI host, string typeName)
	{
		if (!host || !host.GetInventory())
			return InventorySlots.INVALID;

		TStringArray slots = new TStringArray;
		VPPAttCatalog.Get().CollectInventorySlots(typeName, slots);
		int i;
		int slotId;
		string slotName;
		for (i = 0; i < slots.Count(); i++)
		{
			slotName = slots.Get(i);
			slotId = InventorySlots.GetSlotIdFromString(slotName);
			if (slotId == InventorySlots.INVALID)
				continue;

			if (!host.GetInventory().HasAttachmentSlot(slotId))
				continue;

			if (host.GetInventory().FindAttachment(slotId))
				continue;

			if (host.FindAttachmentBySlotName(slotName))
				continue;

			return slotId;
		}

		return InventorySlots.INVALID;
	}

	static EntityAI AttachCreated(EntityAI host, string typeName, bool localPreview)
	{
		if (!host || typeName == string.Empty)
			return NULL;

		if (!host.GetInventory())
			return NULL;

		int slotId = FirstFreeSlot(host, typeName);
		if (slotId == InventorySlots.INVALID)
			return NULL;

		EntityAI created = host.GetInventory().CreateAttachmentEx(typeName, slotId);
		if (created)
			return created;

		EntityAI item = CreatePart(typeName, host, localPreview);
		if (!item)
			return NULL;

		bool taken;
		if (localPreview)
			taken = host.LocalTakeEntityToTargetAttachmentEx(host, item, slotId);
		else
			taken = host.ServerTakeEntityToTargetAttachmentEx(host, item, slotId);

		if (taken)
			return item;

		g_Game.ObjectDelete(item);
		return NULL;
	}

	static EntityAI TakeAsAttachment(EntityAI host, string typeName, bool localPreview)
	{
		if (!host)
			return NULL;

		if (localPreview && VPPAttCatalog.Get().IsUnsafeLocalAttach(typeName))
			return NULL;

		return AttachCreated(host, typeName, localPreview);
	}

	static void EmptyMag(EntityAI item, bool localPreview)
	{
		if (!item)
			return;

		if (!VPPAttCatalog.Get().IsWeaponMagazine(item.GetType()))
			return;

		Magazine mag = Magazine.Cast(item);
		if (!mag)
			return;

		if (localPreview)
			mag.LocalSetAmmoCount(0);
		else
			mag.ServerSetAmmoCount(0);
	}

	static bool FillMag(EntityAI host, string ammoType, bool localPreview)
	{
		if (!host || ammoType == string.Empty)
			return false;

		if (!VPPAttCatalog.Get().IsWeaponMagazine(host.GetType()))
			return false;

		if (!VPPAttCatalog.Get().IsAmmoPile(ammoType))
		{
			if (!VPPAttCatalog.Get().MagazineAcceptsAmmo(host.GetType(), ammoType))
				return false;
		}

		Magazine mag = Magazine.Cast(host);
		if (!mag)
			return false;

		string cart = VPPAttCatalog.Get().CartridgeType(ammoType);
		int max = mag.GetAmmoMax();
		int i;
		int stored;
		stored = 0;
		if (localPreview)
		{
			mag.LocalSetAmmoCount(0);
			if (cart != string.Empty)
			{
				for (i = 0; i < max; i++)
				{
					if (!mag.LocalStoreCartridge(0.0, cart))
						break;

					stored = stored + 1;
				}
			}

			if (stored == 0)
				mag.LocalSetAmmoMax();

			return mag.GetAmmoCount() > 0;
		}

		mag.ServerSetAmmoCount(0);
		if (cart != string.Empty)
		{
			for (i = 0; i < max; i++)
			{
				if (!mag.ServerStoreCartridge(0.0, cart))
					break;

				stored = stored + 1;
			}
		}

		if (stored == 0)
			mag.ServerSetAmmoMax();

		mag.SetSynchDirty();
		return mag.GetAmmoCount() > 0;
	}
};
