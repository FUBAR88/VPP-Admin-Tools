modded class Groza_Barrel_Grip
{
	override bool CanPutAsAttachment(EntityAI parent)
	{
		if (!parent)
			return false;

		EntityAI stock = parent.FindAttachmentBySlotName("weaponButtstockAK");
		if (stock && stock.IsKindOf("GrozaGL_LowerReceiver"))
			return false;

		return true;
	}
};

modded class Groza_Barrel_Suppressor
{
	override bool CanPutAsAttachment(EntityAI parent)
	{
		if (!parent)
			return false;

		EntityAI stock = parent.FindAttachmentBySlotName("weaponButtstockAK");
		if (stock && stock.IsKindOf("GrozaGL_LowerReceiver"))
			return false;

		return true;
	}
};
