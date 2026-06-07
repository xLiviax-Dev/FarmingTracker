# Farming Tracker

A Guild Wars 2 Nexus addon that tracks items and currencies collected during farming sessions in real-time, with detailed profit calculations, filtering, and customizable display options.

## Requirements

- **[Nexus Addon Manager](https://raidcore.gg/Nexus)** — required to load the addon
- **[DRF (drf.rs)](https://drf.rs/)** — required for real-time item and currency data
  - Install via Nexus Addon Manager or directly from [https://drf.rs/](https://drf.rs/)
- **GW2 API Key** — required for item details and Trading Post prices
  - Create one at [https://account.guildwars2.com/applications](https://account.guildwars2.com/applications)
  - Required permissions: `account`, `characters`, `inventories`, `wallet`, `tradingpost`

## Installation

1. Download the latest release from [GitHub](https://github.com/xLiviax-Dev/FarmingTracker)
2. Place the `.dll` file in your addons folder:
   ```
   Guild Wars 2\addons\FarmingTracker\
   ```
3. Load the addon through Nexus Addon Manager
4. Enter your **DRF Token** and **GW2 API Key** in the Farming Tracker settings (Nexus Options Panel)

## Configuration

### DRF Token
1. Open DRF and copy your personal token from its settings
2. Paste it into the Farming Tracker settings under **Account Management**
3. Click **Save**

### GW2 API Key
1. Visit [https://account.guildwars2.com/applications](https://account.guildwars2.com/applications)
2. Create a new API key with the required permissions listed above
3. Paste it into the Farming Tracker settings under **Account Management** (Note: A valid key must consist of exactly 9 blocks separated by hyphens)
4. Click **Save**

### Hotkeys
Customizable in **Nexus → Keybinds**:
- **Main Window**: `CTRL+F` (default)
- **Mini Window**: `CTRL+SHIFT+M` (default)

## Tabs

### Summary
Overview of the current session — DRF connection status, total profit, item count, and quick action buttons (Reset, Refresh).

### Profit
Detailed profit breakdown including vendor value, Trading Post value, and custom profit totals.

### Timeline
Chronological view of all drops and currency changes in the current session. Enable in settings.

### Items
All tracked items from the current session. Supports **List** and **Grid** view. Each item shows icon, name, count change, and profit. Right-click an item to ignore, favorite, or set a custom profit value.

### Currencies
All tracked currencies including Coin, Karma, Laurels, map currencies, WvW currencies, and more.

### Favorites
Items and currencies marked as favorites for quick access. Enable in settings.

### Ignored
Items and currencies you have chosen to hide from the main lists. Enable in settings.

### Session History
Browse and compare past farming sessions. Enable in settings. Configurable number of sessions to keep.

### Custom Profit
Manage custom profit values for specific items. Enable in settings.

### Filter
Filter items and currencies displayed in the tracker by:
- **Sell method** — Vendor, Trading Post, Custom Profit
- **API knowledge** — Known / Unknown by API
- **Item type** — Armor, Weapon, Trinket, Bag, Container, Crafting Material, Consumable, Trophy, and more
- **Currencies** — Main currencies, WvW/PvP currencies, Map currencies
- **Additional** — Account Bound, No-sell, Favorite, Ignored, price range, count range

### Debug
DRF and API logs, profit calculations, filter status, memory usage, request counts, and more. Enable in settings.

## Features

- **Real-time tracking** via DRF integration
- **Profit calculation** using vendor prices, Trading Post sell/buy prices, and custom values
- **Salvage Kit tracking** — Copper-Fed, Silver-Fed, and Runecrafter's
- **List & Grid view** for Items and Currencies tabs
- **Favorites system** — mark items for quick access
- **Ignore system** — hide items you don't care about
- **Session History** — save and review past sessions
- **Custom Profit** — override API prices with your own values per item
- **Advanced filters** — show exactly what you want
- **Mini Window** — compact overlay with key stats, click-through mode, fully repositionable
- **Auto-Reset** — reset session data automatically based on a schedule
- **Hotkey support** for main and mini window
- **Multi-language support** — Czech, German, English, Spanish, French, Italian, Polish, Portuguese, Russian, Chinese
- **Backup & Restore** — export and import your settings and data
- **Search** — quickly find items and currencies

## Auto-Reset Options

- Never (manual only)
- On addon load
- Daily reset (00:00 UTC)
- Weekly reset (Monday 07:30 UTC)
- Weekly NA WvW reset (Saturday 02:00 UTC)
- Weekly EU WvW reset (Friday 18:00 UTC)
- Map bonus reset (Thursday 20:00 UTC)
- Minutes after game shutdown

## Troubleshooting

**DRF not connected**
- Make sure DRF is running and your DRF Token is entered correctly in settings

**Items show "Loading..."**
- Make sure your GW2 API Key is entered and has the correct permissions
- Check your internet connection

**No data appearing**
- Wait a few seconds after loading into a map for DRF to transmit data
- Verify DRF connection status in the Summary tab

## Credits

- Built with [Nexus Addon API](https://raidcore.gg/Nexus)
- UI rendered with [Dear ImGui](https://github.com/ocornut/imgui)
- Item and price data from the [GW2 API](https://wiki.guildwars2.com/wiki/API:Main)
- Real-time data from [DRF (drf.rs)](https://drf.rs/) Special thanks to DRF for the dedication and for providing such an amazing resource!
- JSON parsing with [nlohmann/json](https://github.com/nlohmann/json)

## License

This project is licensed under the MIT License — see the [LICENSE] file for details.

## Support

For issues, feature requests, or questions, please visit the [GitHub repository](https://github.com/xLiviax-Dev/FarmingTracker).
