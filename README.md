# 🌾 Farming Tracker

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![GitHub Release](https://img.shields.io/github/v/release/xLiviax-Dev/FarmingTracker)](https://github.com/xLiviax-Dev/FarmingTracker/releases)
[![GW2](https://img.shields.io/badge/Guild%20Wars%202-Addon-blue)](https://www.guildwars2.com/)
[![Nexus](https://img.shields.io/badge/Nexus-Addon-green)](https://raidcore.gg/Nexus)

> A powerful Guild Wars 2 Nexus addon that tracks items and currencies collected during farming sessions in real-time, with detailed profit calculations, advanced filtering, and customizable display options.

---

## ✨ Features

- **🔴 Real-time Tracking** — Live item and currency tracking via DRF integration
- **💰 Profit Calculation** — Vendor prices, Trading Post values, and custom profit overrides
- **🔧 Advanced Filtering** — Filter by sell method, item type, currency, API status, and more
- **📊 Session History** — Save, review, and compare past farming sessions
- **⭐ Favorites System** — Quick access to your most important items and currencies
- **🚫 Ignore System** — Hide items you don't want to track
- **🎨 Custom UI** — List & Grid views, customizable colors, mini window overlay
- **⌨️ Hotkey Support** — Quick access to main and mini windows
- **🌍 Multi-Language** — Available in 18 languages
- **💾 Backup & Restore** — Export and import your settings and data
- **🔔 Notifications** — Custom alerts for profit goals and session milestones
- **🔄 Auto-Reset** — Automatic session reset based on schedule

---

## 🌍 Supported Languages

🇨🇿 Czech | 🇩🇪 German | 🇬🇧 English | 🇪🇸 Spanish | 🇫🇷 French | 🇮🇹 Italian | 🇵🇱 Polish | 🇵🇹 Portuguese | 🇷🇺 Russian | 🇨🇳 Chinese | 🇩🇰 Danish | 🇬🇷 Greek | 🇫🇮 Finnish | 🇭🇺 Hungarian | 🇳🇱 Dutch | 🇳🇴 Norwegian | 🇷🇴 Romanian | 🇸🇪 Swedish

---

## 📋 Requirements

| Requirement | Description |
|------------|-------------|
| **[Nexus Addon Manager](https://raidcore.gg/Nexus)** | Required to load the addon |
| **[DRF (drf.rs)](https://drf.rs/)** | Required for real-time item and currency data |
| **GW2 API Key** | Required for item details and Trading Post prices |

### GW2 API Key Permissions

Create your API key at [account.guildwars2.com/applications](https://account.guildwars2.com/applications) with these permissions:

```
account • characters • inventories • wallet • tradingpost
```

---

## 🚀 Installation

1. **Download** the latest release from [GitHub](https://github.com/xLiviax-Dev/FarmingTracker/releases)
2. **Place** the `.dll` file in your addons folder:
   ```
   Guild Wars 2\addons\FarmingTracker\
   ```
3. **Load** the addon through Nexus Addon Manager
4. **Configure** your DRF Token and GW2 API Key in the settings

---

## ⚙️ Configuration

### 🔑 DRF Token

1. Open DRF and copy your personal token from its settings
2. Paste it into Farming Tracker settings under **Account Management**
3. Click **Save**

### 🔑 GW2 API Key

1. Visit [account.guildwars2.com/applications](https://account.guildwars2.com/applications)
2. Create a new API key with the required permissions
3. Paste it into Farming Tracker settings under **Account Management**
4. Click **Save**

> **Note:** A valid API key must consist of exactly 9 blocks separated by hyphens (e.g., `XXXX-XXXX-XXXX-XXXX-XXXX-XXXX-XXXX-XXXX-XXXX`)

### ⌨️ Hotkeys

Customizable in **Nexus → Keybinds**:

| Action | Default Hotkey |
|--------|---------------|
| Main Window | `CTRL+F` |
| Mini Window | `CTRL+SHIFT+M` |

---

## 📑 Tabs Overview

| Tab | Description |
|-----|-------------|
| **Summary** | Session overview with DRF status, total profit, item count, and quick actions |
| **Profit** | Detailed breakdown of vendor value, Trading Post value, and custom profit |
| **Timeline** | Chronological view of all drops and currency changes |
| **Items** | All tracked items with List/Grid views, right-click for actions |
| **Currencies** | All tracked currencies including Coin, Karma, Laurels, map currencies |
| **Favorites** | Quick access to your favorite items and currencies |
| **Ignored** | Items and currencies you've chosen to hide |
| **Session History** | Browse and compare past farming sessions |
| **Custom Profit** | Manage custom profit values for specific items |
| **Filter** | Advanced filtering options for items and currencies |
| **Debug** | DRF/API logs, profit calculations, memory usage, and diagnostics |

---

## 🎯 Filter Options

Filter items and currencies by:

- **Sell Method** — Vendor, Trading Post, Custom Profit
- **API Knowledge** — Known / Unknown by GW2 API
- **Item Type** — Armor, Weapon, Trinket, Bag, Container, Crafting Material, Consumable, Trophy, and more
- **Currencies** — Main currencies, WvW/PvP currencies, Map currencies
- **Additional** — Account Bound, No-sell, Favorite, Ignored, price range, count range

---

## ⏰ Auto-Reset Options

Choose when to automatically reset your session data:

- Never (manual only)
- On addon load
- Daily reset (00:00 UTC)
- Weekly reset (Monday 07:30 UTC)
- Weekly NA WvW reset (Saturday 02:00 UTC)
- Weekly EU WvW reset (Friday 18:00 UTC)
- Map bonus reset (Thursday 20:00 UTC)
- Minutes after game shutdown

---

## 🔧 Troubleshooting

### DRF not connected

- ✅ Make sure DRF is running
- ✅ Verify your DRF Token is entered correctly in settings
- ✅ Check that DRF is transmitting data

### Items show "Loading..."

- ✅ Ensure your GW2 API Key is entered with correct permissions
- ✅ Check your internet connection
- ✅ Verify the API key format (9 blocks with hyphens)

### No data appearing

- ✅ Wait a few seconds after loading into a map for DRF to transmit data
- ✅ Verify DRF connection status in the Summary tab
- ✅ Check that you're in a map where items can drop

---

## 🙏 Credits

This project is built with amazing open-source tools and services:

- **[Nexus Addon API](https://raidcore.gg/Nexus)** — Addon framework
- **[Dear ImGui](https://github.com/ocornut/imgui)** — UI rendering
- **[GW2 API](https://wiki.guildwars2.com/wiki/API:Main)** — Item and price data
- **[DRF (drf.rs)](https://drf.rs/)** — Real-time game data (Special thanks to DRF for their dedication!)
- **[nlohmann/json](https://github.com/nlohmann/json)** — JSON parsing

---

## 📄 License

This project is licensed under the **MIT License** — see the [LICENSE](LICENSE) file for details.

---

## 💬 Support

For issues, feature requests, or questions, please visit:

- **[GitHub Repository](https://github.com/xLiviax-Dev/FarmingTracker)** — Report issues and request features
- **[Discord](https://discord.gg/)** — Join the community (add link if available)

---

<div align="center">

**Made with ❤️ for the Guild Wars 2 community**

[⬆ Back to top](#-farming-tracker)

</div>
