class VPPInvPlayerInfo
{
	string m_SteamId;
	string m_Name;

	void VPPInvPlayerInfo(string steamId, string name)
	{
		m_SteamId = steamId;
		m_Name = name;
	}
};

class VPPInvItemInfo
{
	int m_NetLow;
	int m_NetHigh;
	string m_ClassName;
	string m_DisplayName;
	int m_Quantity;
	int m_MaxQuantity;
	int m_HealthLevel;
	bool m_Stackable;

	void VPPInvItemInfo()
	{
		m_NetLow = 0;
		m_NetHigh = 0;
		m_ClassName = "";
		m_DisplayName = "";
		m_Quantity = 1;
		m_MaxQuantity = 1;
		m_HealthLevel = 0;
		m_Stackable = false;
	}
};
