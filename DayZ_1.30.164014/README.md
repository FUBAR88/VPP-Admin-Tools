# VPP Admin Tools — Changelog

Use this file as the source for release notes. Newest changes first.

**Target:** DayZ Experimental **1.30.164014** (`DAYZ_1_30`)

---

## Unreleased — DayZ Experimental 1.30.164014 compatibility

### Fixed
- **World module compile failure** caused by removed `CombinationLock.UnlockServer` API.
  - Replaced with `CombinationLock.UnlockOnServer(NULL, fence)` in fence unlock / clear-combo flow.
  - File: `4_World/VPPAdminTools/Misc/basebuildinghelperfuncs.c`

### Changed (DayZ 1.30.164014 API updates)
- Attachment take APIs updated to 1.30 replacements:
  - `LocalTakeEntityAsAttachmentEx` → `LocalTakeEntityToTargetAttachmentEx`
  - `ServerTakeEntityAsAttachmentEx` → `ServerTakeEntityToTargetAttachmentEx`
  - File: `4_World/VPPAdminTools/Plugins/PluginBase/AttachmentsBuilder/Classes/VPPAttAttach.c`
- Construction collision check updated:
  - `Construction.IsColliding(partName)` → `Construction.IsCollidingEx(CollisionCheckData)`
  - File: `4_World/VPPAdminTools/Classes/Actions/ActionAdminBaseBuilder.c`
- Removed obsolete `Construction.DestroyCollisionTrigger()` call (1.30: no replacement).
  - File: `4_World/VPPAdminTools/Classes/Actions/ActionAdminBaseBuilder.c`

### Notes
- Script log symptom before fix: `Can't compile "World" script module!` with `Undefined function 'CombinationLock.UnlockServer'`.
- Many follow-on `Bad type 'Param*' / 'JsonFileLoader' / 'map'` errors in the same log were cascade failures after World failed to compile, not separate root causes.
- Verified against DayZ Experimental **1.30.164014**.

### Release-note draft
```
DayZ Experimental 1.30.164014 compatibility:
- Fixed World compile crash from CombinationLock.UnlockServer removal (now UnlockOnServer)
- Updated attachment and construction collision APIs to 1.30 replacements
- Removed obsolete DestroyCollisionTrigger usage
```
