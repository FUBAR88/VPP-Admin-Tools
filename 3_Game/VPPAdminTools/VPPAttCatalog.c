class VPPAttGroup
{
	string m_SlotName;
	ref array<string> m_Types;

	void VPPAttGroup(string slotName)
	{
		m_SlotName = slotName;
		m_Types = new array<string>;
	}
};

class VPPAttSpawnParams
{
	int m_Category;
	ref array<string> m_Types;
	int m_PlacementType;
	vector m_Position;
	ref array<string> m_Targets;

	void VPPAttSpawnParams()
	{
		m_Category = VPPAttCatalog.ATT_CAT_WEAPONS;
		m_Types = new array<string>;
		m_PlacementType = 1;
		m_Position = vector.Zero;
		m_Targets = new array<string>;
	}
};

class VPPAttCatalog
{
	const static int ATT_CAT_WEAPONS = 0;
	const static int ATT_CAT_VEHICLES = 1;
	const static int ATT_CAT_CLOTHES = 2;
	const static bool ATT_DEBUG = false; // set to true to enable debug logging

	protected static ref VPPAttCatalog s_Instance;
	protected ref map<string, ref array<string>> m_SlotToTypes;
	protected ref map<string, ref array<ref VPPAttGroup>> m_Resolved;
	protected ref map<string, bool> m_WeaponBroken;
	protected bool m_SlotCacheReady;

	void VPPAttCatalog()
	{
		m_SlotToTypes = new map<string, ref array<string>>;
		m_Resolved = new map<string, ref array<ref VPPAttGroup>>;
		m_WeaponBroken = new map<string, bool>;
		m_SlotCacheReady = false;
	}

	static VPPAttCatalog Get()
	{
		if (!s_Instance)
			s_Instance = new VPPAttCatalog();

		return s_Instance;
	}

	static void Log(string message)
	{
		if (!ATT_DEBUG)
			return;

		Print("[VPP-ATT] " + message);
	}

	void CollectParents(int category, string searchLower, array<string> classNames, array<string> displayNames)
	{
		string kind = KindForCategory(category);
		string cfgPath = CfgPathForCategory(category);
		int nClasses = g_Game.ConfigGetChildrenCount(cfgPath);
		int i;
		string className;
		string lowerName;
		string displayName;
		int scope;

		for (i = 0; i < nClasses; i++)
		{
			g_Game.ConfigGetChildName(cfgPath, i, className);
			if (className == string.Empty)
				continue;

			scope = g_Game.ConfigGetInt(cfgPath + " " + className + " scope");
			if (scope == 0 || scope == 1)
				continue;

			lowerName = className;
			lowerName.ToLower();
			if (!g_Game.IsKindOf(lowerName, kind))
				continue;

			displayName = ReadDisplayName(cfgPath, className);
			if (!PassesSearch(className, displayName, searchLower))
				continue;

			classNames.Insert(className);
			displayNames.Insert(displayName);
		}
	}

	array<ref VPPAttGroup> Resolve(string className)
	{
		if (m_Resolved.Contains(className))
			return m_Resolved.Get(className);

		array<ref VPPAttGroup> groups = new array<ref VPPAttGroup>;
		string cfgPath = FindConfigPath(className);
		if (cfgPath == string.Empty)
		{
			m_Resolved.Insert(className, groups);
			return groups;
		}

		AddDirectGroup(groups, "magazine", cfgPath + " " + className + " magazines");
		AddDirectGroup(groups, "ammo", cfgPath + " " + className + " chamberableFrom");
		AddDirectGroup(groups, "ammo", cfgPath + " " + className + " ammo");
		AddAttachmentSlots(groups, className, cfgPath);
		AddGuiPropSlots(groups, className);

		VPPAttCatalog.Log("Resolve " + className + " groups=" + groups.Count().ToString());
		int g;
		VPPAttGroup dump;
		for (g = 0; g < groups.Count(); g++)
		{
			dump = groups.Get(g);
			if (!dump)
				continue;

			VPPAttCatalog.Log("  slot=" + dump.m_SlotName + " types=" + dump.m_Types.Count().ToString() + " sample=" + SampleTypes(dump.m_Types));
		}

		m_Resolved.Insert(className, groups);
		return groups;
	}

	string ReadDisplayName(string cfgPath, string className)
	{
		string displayName = g_Game.ConfigGetTextOut(cfgPath + " " + className + " displayName");
		if (g_Game.FormatRawConfigStringKeys(displayName))
			displayName = Widget.TranslateString(displayName);

		if (displayName == string.Empty)
			return className;

		return displayName;
	}

	string FindConfigPath(string className)
	{
		if (g_Game.ConfigIsExisting("CfgVehicles " + className))
			return "CfgVehicles";

		if (g_Game.ConfigIsExisting("CfgWeapons " + className))
			return "CfgWeapons";

		if (g_Game.ConfigIsExisting("CfgMagazines " + className))
			return "CfgMagazines";

		return "";
	}

	protected string KindForCategory(int category)
	{
		if (category == VPPAttCatalog.ATT_CAT_VEHICLES)
			return "transport";

		if (category == VPPAttCatalog.ATT_CAT_CLOTHES)
			return "clothing_base";

		return "weapon_base";
	}

	protected string CfgPathForCategory(int category)
	{
		if (category == VPPAttCatalog.ATT_CAT_WEAPONS)
			return "CfgWeapons";

		return "CfgVehicles";
	}

	protected bool PassesSearch(string className, string displayName, string searchLower)
	{
		if (searchLower == string.Empty)
			return true;

		string lowerClass = className;
		string lowerDisplay = displayName;
		lowerClass.ToLower();
		lowerDisplay.ToLower();
		if (lowerClass.Contains(searchLower))
			return true;

		if (lowerDisplay.Contains(searchLower))
			return true;

		return false;
	}

	protected void AddDirectGroup(array<ref VPPAttGroup> groups, string slotName, string cfgKey)
	{
		TStringArray types = new TStringArray;
		g_Game.ConfigGetTextArray(cfgKey, types);
		if (!types)
			return;

		if (types.Count() == 0)
			return;

		VPPAttGroup group = FindOrCreateGroup(groups, slotName);
		int i;
		string typeName;
		for (i = 0; i < types.Count(); i++)
		{
			typeName = types.Get(i);
			if (typeName == string.Empty)
				continue;

			if (group.m_Types.Find(typeName) == -1)
				group.m_Types.Insert(typeName);
		}
	}

	protected void AddAttachmentSlots(array<ref VPPAttGroup> groups, string className, string cfgPath)
	{
		TStringArray slots = new TStringArray;
		g_Game.ConfigGetTextArray(cfgPath + " " + className + " attachments", slots);
		AddSlotNames(groups, slots);
	}

	protected void AddGuiPropSlots(array<ref VPPAttGroup> groups, string className)
	{
		string guiPath = "CfgVehicles " + className + " GUIInventoryAttachmentsProps";
		int catCount = g_Game.ConfigGetChildrenCount(guiPath);
		int i;
		string categoryName;
		TStringArray slots;

		for (i = 0; i < catCount; i++)
		{
			g_Game.ConfigGetChildName(guiPath, i, categoryName);
			if (categoryName == string.Empty)
				continue;

			slots = new TStringArray;
			g_Game.ConfigGetTextArray(guiPath + " " + categoryName + " attachmentSlots", slots);
			AddSlotNames(groups, slots);
		}
	}

	protected void AddSlotNames(array<ref VPPAttGroup> groups, TStringArray slots)
	{
		if (!slots)
			return;

		int i;
		string slotName;
		array<string> types;
		VPPAttGroup group;
		int t;
		string typeName;

		for (i = 0; i < slots.Count(); i++)
		{
			slotName = slots.Get(i);
			if (slotName == string.Empty)
				continue;

			if (slotName == "Back" || slotName == "Shoulder")
			{
				VPPAttCatalog.Log("skip player slot " + slotName + " on host");
				continue;
			}

			types = GetTypesForSlot(slotName);
			if (!types || types.Count() == 0)
			{
				VPPAttCatalog.Log("empty types for slot " + slotName);
				continue;
			}

			group = FindOrCreateGroup(groups, slotName);
			for (t = 0; t < types.Count(); t++)
			{
				typeName = types.Get(t);
				if (group.m_Types.Find(typeName) == -1)
					group.m_Types.Insert(typeName);
			}
		}
	}

	protected VPPAttGroup FindOrCreateGroup(array<ref VPPAttGroup> groups, string slotName)
	{
		int i;
		VPPAttGroup group;
		for (i = 0; i < groups.Count(); i++)
		{
			group = groups.Get(i);
			if (group.m_SlotName == slotName)
				return group;
		}

		group = new VPPAttGroup(slotName);
		groups.Insert(group);
		return group;
	}

	protected array<string> GetTypesForSlot(string slotName)
	{
		EnsureSlotCache();
		string key = slotName;
		key.ToLower();
		array<string> types;
		if (m_SlotToTypes.Contains(key))
			return m_SlotToTypes.Get(key);

		types = new array<string>;
		return types;
	}

	protected void EnsureSlotCache()
	{
		if (m_SlotCacheReady)
			return;

		AddCfgToSlotCache("CfgVehicles");
		AddCfgToSlotCache("CfgWeapons");
		AddCfgToSlotCache("CfgMagazines");
		m_SlotCacheReady = true;
	}

	protected void AddCfgToSlotCache(string cfgPath)
	{
		int nClasses = g_Game.ConfigGetChildrenCount(cfgPath);
		int i;
		string className;
		int scope;
		int cfgType;
		string slotPath;
		TStringArray slots;
		string singleSlot;

		for (i = 0; i < nClasses; i++)
		{
			g_Game.ConfigGetChildName(cfgPath, i, className);
			if (className == string.Empty)
				continue;

			scope = g_Game.ConfigGetInt(cfgPath + " " + className + " scope");
			if (scope != 2)
				continue;

			if (IsAbstractClass(className))
				continue;

			slotPath = cfgPath + " " + className + " inventorySlot";
			cfgType = g_Game.ConfigGetType(slotPath);
			if (cfgType == CT_ARRAY)
			{
				slots = new TStringArray;
				g_Game.ConfigGetTextArray(slotPath, slots);
				AddClassToSlots(className, slots);
			}
			else if (cfgType == CT_STRING)
			{
				singleSlot = string.Empty;
				if (g_Game.ConfigGetText(slotPath, singleSlot) && singleSlot != string.Empty)
				{
					slots = new TStringArray;
					slots.Insert(singleSlot);
					AddClassToSlots(className, slots);
				}
			}
		}
	}

	protected void AddClassToSlots(string className, TStringArray slots)
	{
		if (!slots)
			return;

		int i;
		string slotName;
		array<string> types;
		for (i = 0; i < slots.Count(); i++)
		{
			slotName = slots.Get(i);
			if (slotName == string.Empty)
				continue;

			slotName.ToLower();
			if (m_SlotToTypes.Contains(slotName))
			{
				types = m_SlotToTypes.Get(slotName);
			}
			else
			{
				types = new array<string>;
				m_SlotToTypes.Insert(slotName, types);
			}

			if (types.Find(className) == -1)
				types.Insert(className);
		}
	}

	bool IsWeaponMagazine(string className)
	{
		if (className == string.Empty)
			return false;

		if (!g_Game.IsKindOf(className, "Magazine_Base"))
			return false;

		if (g_Game.IsKindOf(className, "Ammunition_Base"))
			return false;

		return true;
	}

	bool IsUnsafeLocalPreview(string className)
	{
		if (className == string.Empty)
			return false;

		if (!g_Game)
			return false;

		if (g_Game.IsKindOf(className, "GP25Base"))
			return true;

		if (g_Game.IsKindOf(className, "M203Base"))
			return true;

		if (g_Game.IsKindOf(className, "Launcher_Base"))
			return true;

		if (g_Game.IsKindOf(className, "DZ_LightAI"))
			return true;

		if (g_Game.IsKindOf(className, "DayZCreature"))
			return true;

		if (className == "Groza")
			return true;

		if (className == "PM73Rak")
			return true;

		if (className == "Trumpet")
			return true;

		if (className == "Red9")
			return true;

		if (className == "QuickieBow")
			return true;

		if (className == "AKM_TESTBED")
			return true;

		if (IsWeaponBroken(className))
			return true;

		return false;
	}

	bool IsWeaponBroken(string className)
	{
		if (className == string.Empty)
			return false;

		if (!g_Game)
			return false;

		if (IsWeaponMagazine(className) || IsAmmoPile(className))
			return false;

		if (!g_Game.IsKindOf(className, "Weapon_Base"))
			return false;

		if (m_WeaponBroken.Contains(className))
			return m_WeaponBroken.Get(className);

		string cfgPath = FindConfigPath(className);
		if (cfgPath == string.Empty)
		{
			m_WeaponBroken.Insert(className, false);
			return false;
		}

		bool broken = false;
		if (ConfigListHasMissingMag(cfgPath + " " + className + " magazines"))
			broken = true;
		else if (ConfigListHasMissingMag(cfgPath + " " + className + " chamberableFrom"))
			broken = true;

		m_WeaponBroken.Insert(className, broken);
		return broken;
	}

	bool ConfigListHasMissingMag(string cfgKey)
	{
		TStringArray names = new TStringArray;
		g_Game.ConfigGetTextArray(cfgKey, names);
		if (!names)
			return false;

		int i;
		string name;
		for (i = 0; i < names.Count(); i++)
		{
			name = names.Get(i);
			if (name == string.Empty)
				continue;

			if (g_Game.ConfigIsExisting("CfgMagazines " + name))
				continue;

			if (g_Game.ConfigIsExisting("CfgAmmo " + name))
				continue;

			return true;
		}

		return false;
	}

	bool CanSpawnLocalPreview(string className)
	{
		if (className == string.Empty)
			return false;

		if (IsWeaponMagazine(className) || IsAmmoPile(className))
			return true;

		if (IsUnsafeLocalPreview(className))
			return false;

		return true;
	}

	bool IsUnsafeLocalAttach(string className)
	{
		if (className == string.Empty)
			return true;

		if (IsWeaponMagazine(className))
			return true;

		if (IsUnsafeLocalPreview(className))
			return true;

		return false;
	}

	string CartridgeType(string ammoType)
	{
		if (ammoType == string.Empty)
			return "";

		TStringArray pileAmmo = new TStringArray;
		g_Game.ConfigGetTextArray("CfgMagazines " + ammoType + " ammo", pileAmmo);
		if (pileAmmo && pileAmmo.Count() > 0 && pileAmmo.Get(0) != string.Empty)
			return pileAmmo.Get(0);

		string single = string.Empty;
		if (g_Game.ConfigGetText("CfgMagazines " + ammoType + " ammo", single) && single != string.Empty)
			return single;

		return ammoType;
	}

	bool MagazineAcceptsAmmo(string magType, string ammoType)
	{
		if (magType == string.Empty || ammoType == string.Empty)
			return false;

		if (!IsWeaponMagazine(magType))
			return false;

		if (ListContains("CfgMagazines " + magType + " ammoItems", ammoType))
			return true;

		string cart = CartridgeType(ammoType);
		if (ListContains("CfgMagazines " + magType + " ammo", ammoType))
			return true;

		if (ListContains("CfgMagazines " + magType + " ammo", cart))
			return true;

		if (ListContains("CfgMagazines " + magType + " chamberableFrom", ammoType))
			return true;

		if (ListContains("CfgMagazines " + magType + " chamberableFrom", cart))
			return true;

		if (g_Game.IsKindOf(ammoType, "Ammunition_Base"))
			return true;

		return false;
	}

	bool IsAmmoPile(string className)
	{
		if (className == string.Empty)
			return false;

		return g_Game.IsKindOf(className, "Ammunition_Base");
	}

	void CollectInventorySlots(string className, TStringArray slots)
	{
		if (!slots)
			return;

		string cfgPath = FindConfigPath(className);
		if (cfgPath == string.Empty)
			return;

		string slotPath = cfgPath + " " + className + " inventorySlot";
		TStringArray cfgSlots = new TStringArray;
		g_Game.ConfigGetTextArray(slotPath, cfgSlots);
		int i;
		string slotName;
		if (cfgSlots)
		{
			for (i = 0; i < cfgSlots.Count(); i++)
			{
				slotName = cfgSlots.Get(i);
				if (slotName != string.Empty && slots.Find(slotName) == -1)
					slots.Insert(slotName);
			}
		}

		if (slots.Count() > 0)
			return;

		slotName = string.Empty;
		if (g_Game.ConfigGetText(slotPath, slotName) && slotName != string.Empty)
			slots.Insert(slotName);
	}

	protected bool ListContains(string cfgKey, string value)
	{
		if (value == string.Empty)
			return false;

		TStringArray values = new TStringArray;
		g_Game.ConfigGetTextArray(cfgKey, values);
		if (values && values.Find(value) > -1)
			return true;

		string single = string.Empty;
		if (g_Game.ConfigGetText(cfgKey, single) && single == value)
			return true;

		return false;
	}

	protected bool IsAbstractClass(string className)
	{
		int len = className.Length();
		string tail;
		if (len > 5)
		{
			tail = className.Substring(len - 5, 5);
			tail.ToLower();
			if (tail == "_base")
				return true;
		}

		if (len > 10)
		{
			tail = className.Substring(len - 10, 10);
			tail.ToLower();
			if (tail == "_colorbase")
				return true;
		}

		return false;
	}

	protected string SampleTypes(array<string> types)
	{
		if (!types || types.Count() == 0)
			return "-";

		string sample = types.Get(0);
		int i;
		int limit = types.Count();
		if (limit > 6)
			limit = 6;

		for (i = 1; i < limit; i++)
		{
			sample = sample + "," + types.Get(i);
		}

		return sample;
	}
};
