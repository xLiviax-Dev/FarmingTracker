# Farming Tracker Plugin

A Guild Wars 2 addon that tracks items and currencies collected during farming sessions, providing detailed profit calculations and customizable display options.

## Features

- **Item Tracking**: Automatically tracks items collected from DRF (Dungeon Raid Fractal) data
- **Currency Tracking**: Monitors all currencies including Coin, Karma, Laurels, and more
- **Profit Calculation**: Calculates profit based on vendor sell prices and Trading Post values
- **Custom Profit System**: Set custom profit values for specific items
- **Advanced Filtering**: Filter items by rarity, type, sellability, and more
- **Mini Window**: Overlay widget showing key statistics at a glance
- **Debug Mode**: Comprehensive debug information for troubleshooting
- **Favorites System**: Mark items as favorites for quick access
- **Ignored Items**: Exclude specific items from tracking
- **Auto-Reset**: Automatic data reset based on various schedules
- **Hotkey Support**: Toggle windows with customizable hotkeys

## Requirements

- **Nexus Addon Manager**: Required to load the addon
- **DRF (Dungeon Raid Fractal)**: Required for real-time data transmission
  - Install via Nexus Addon Manager or [https://drf.rs/](https://drf.rs/)
- **GW2 API Key**: Required for item/currency details and Trading Post prices
  - Get your API key at [https://account.guildwars2.com/applications](https://account.guildwars2.com/applications)
  - Required permissions: `account`, `characters`, `inventories`, `wallet`, `commerce`

## Installation

1. Download the latest release from GitHub
2. Extract the plugin folder to your Guild Wars 2 addons directory:
   - `%USERPROFILE%\Documents\Guild Wars 2\addons\`
3. Load the addon through Nexus Addon Manager
4. Configure your DRF Token and GW2 API Key in the Nexus Mods Options (Farming Tracker settings)

## Configuration

### DRF Token

1. Open DRF and get your token from the DRF settings
2. Enter the token in the Farming Tracker settings in Nexus Mods Options
3. Click "Save" to apply

### GW2 API Key

1. Visit [https://account.guildwars2.com/applications](https://account.guildwars2.com/applications)
2. Create a new application with the required permissions
3. Copy the API key and enter it in the Farming Tracker settings in Nexus Mods Options
4. Click "Save" to apply

### Hotkeys

- **Main Window Toggle**: Default `CTRL+SHIFT+F` (customizable in Nexus Mods Keybinds)
- **Mini Window Toggle**: Default `CTRL+SHIFT+M` (customizable in Nexus Mods Keybinds)

## Usage

### Main Window

- **Summary Tab**: Overview of session statistics, DRF status, and quick actions
- **Items Tab**: Detailed list of collected items with profit calculations
- **Currencies Tab**: Overview of collected currencies
- **Profit Tab**: Profit breakdown and statistics
- **Debug Tab**: Debug information and logs (enable in Nexus Mods Options)

### Mini Window

The Mini Window is an overlay widget that shows key statistics:
- Toggle with hotkey (`CTRL+SHIFT+M` by default)
- Customize displayed information in Nexus Mods Options
- Drag to reposition (if enabled in Nexus Mods Options)
- Click-through mode available for gameplay

### Auto-Reset

Configure automatic data reset based on:
- Never (manual reset only)
- On addon load
- Daily reset (00:00 UTC)
- Weekly reset (Monday 07:30 UTC)
- Weekly NA WvW reset (Saturday 02:00 UTC)
- Weekly EU WvW reset (Friday 18:00 UTC)
- Map bonus reset (Thursday 20:00 UTC)
- Minutes after shutdown

## Advanced Features

### Custom Profit System

Enable custom profit for specific items:
1. Enable "Custom Profit" in Nexus Mods Options
2. Set custom profit values for individual items
3. Items with custom profit override API prices

### Filtering

Filter items by:
- Count (positive/negative)
- Sellability (vendor/Trading Post)
- Rarity (Basic to Legendary)
- Item type (Armor, Weapon, Trinket, etc.)
- Custom profit items
- Known/unknown by API

### Salvage Kits

Configure salvage kit settings:
- Enable/disable specific salvage kits
- Choose between Gold or Karma for salvage costs

## Debug Mode

Enable Debug Mode in Nexus Mods Options to access:
- DRF connection logs
- GW2 API request logs
- Detailed profit calculations
- Item/currency API data details
- Filter status
- Current settings
- GW2 Process Memory usage
- API Request Count
- DRF Reconnect Count
- Custom Profit List
- Ignored Items List

## Troubleshooting

### "DRF not connected"
- Ensure DRF is running
- Verify your DRF Token is correct in Nexus Mods Options
- Check that DRF is properly installed via Nexus Addon Manager

### "No data loaded"
- Wait for DRF to transmit data (may take a few seconds)
- Verify DRF connection status
- Check if Fake DRF Server is enabled in Nexus Mods Options

### Items showing "Loading..."
- Verify GW2 API Key is set in Nexus Mods Options
- Ensure API key has required permissions
- Check internet connection for API requests

## Credits

- Built with Nexus Addon API
- Uses ImGui for UI rendering
- GW2 API for item/currency data
- DRF for real-time data transmission

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Support

For issues, feature requests, or questions, please visit the [GitHub repository](https://github.com/yourusername/farming-tracker-plugin).
